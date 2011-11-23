
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <signal.h>

#include "../../sauron-protocol/protocol.h"
#include "receiver.h"
#include "shmem.h"


// Print a client header 
static void printh(const sockaddr_in& client_addr)
{
  printf("\tcamera_client(%s, %d) ",
         inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
}

bool do_recv(int connected, int recv_size, unsigned char* recv_buffer,
             const char* name, const sockaddr_in& client_addr)
{
  memset(recv_buffer, 0, recv_size);
  int packet_size = recv_size;
  unsigned char* packet_buffer = recv_buffer;

  int bytes_recv = 0;
  int recv_result = -1;
  int total_packet_size = recv_size;

  do
  {
    recv_result = recv(connected, packet_buffer, packet_size, 0);
    bytes_recv += recv_result;
    packet_buffer += recv_result;
    packet_size -= recv_result;
  }
  while (recv_result > 0 && bytes_recv < total_packet_size);

  // Check if the client disconnected.
  if (recv_result == 0)
  {
    printh(client_addr);
    printf("Client disconnected during %s data recv, disconnecting.\n", name);
    fflush(stdout);
    return false;
  }
  else if (recv_result == -1)
  {
    printh(client_addr);
    printf("Client recv failed during %s data recv, disconnecting.\n", name);
    fflush(stdout);
    return false;
  }

  return true;
}

void recv_loop(int connected, sockaddr_in client_addr)
{
  pid_t shmem_key = getpid();
  bool disconnect_client = false;

  int header_packet_size = sizeof(SauronFrameHeader);
  unsigned char* header_packet = new unsigned char[header_packet_size];

  char vid_shmem_name[20] = "";
  sprintf(vid_shmem_name, "SRN_%d_%d", kVideoFrame, shmem_key);
  char depth_shmem_name[20] = "";
  sprintf(depth_shmem_name, "SRN_%d_%d", kDepthData, shmem_key);
  char skele_shmem_name[20] = "";
  sprintf(skele_shmem_name, "SRN_%d_%d", kSkeletonData, shmem_key);

  // Allocate local and shared memory for the video traffic
  int video_packet_size = sizeof(VideoData);
  unsigned char* video_payload = new unsigned char[video_packet_size];
  unsigned char* video_shaddr = 0;
  int video_shfd = -1;
  if(!create_memory_map(vid_shmem_name, video_packet_size+header_packet_size, (void**)&video_shaddr, &video_shfd))
  {
    printh(client_addr);
    printf("Error creating shared memory object: '%s'\n", vid_shmem_name);
    fflush(stdout);
    disconnect_client = true;
  }
  // Allocate local and shared memory for the depth traffic
  int depth_packet_size = sizeof(DepthData);
  unsigned char* depth_payload = new unsigned char[depth_packet_size];
  unsigned char* depth_shaddr = 0;
  int depth_shfd = -1;
  if(!create_memory_map(depth_shmem_name, depth_packet_size+header_packet_size, (void**)&depth_shaddr, &depth_shfd))
  {
    printh(client_addr);
    printf("Error creating shared memory object: '%s'\n", depth_shmem_name);
    fflush(stdout);
    disconnect_client = true;
  }
  // Allocate local and shared memory for the skeleton traffic
  int skeleton_packet_size = sizeof(SkeletalData);
  unsigned char* skeleton_payload = new unsigned char[skeleton_packet_size];
  unsigned char* skele_shaddr = 0;
  int skele_shfd = -1;
  if(!create_memory_map(skele_shmem_name, skeleton_packet_size+header_packet_size, (void**)&skele_shaddr, &skele_shfd))
  {
    printh(client_addr);
    printf("Error creating shared memory object: '%s'\n", skele_shmem_name);
    fflush(stdout);
    disconnect_client = true;
  }

  while(!disconnect_client)
  {
    // Retrieve the header from the client.
    if (!do_recv(connected, header_packet_size,
                 header_packet, "header", client_addr))
    {
      break;
    }
    SauronFrameHeader frame_header;
    frame_header.DeSerialize(header_packet);

    // Check validity of the data received.
    if (frame_header.m_signature != 'SRN1')
    {
      printh(client_addr);
      printf("Got malformed packet with bad signature: '%d', disconnecting.\n",
             frame_header.m_signature);
      fflush(stdout);
      break;
    }

    switch (frame_header.m_payloadKind)
    {
      case kVideoFrame:
      {
        // Retrieve the video frame data from the client.
        if (!do_recv(connected, video_packet_size,
                     video_payload, "video", client_addr))
        {
          disconnect_client = true;
          break;
        }
        VideoData video_data;
        video_data.DeSerialize(video_payload);
        memcpy(video_shaddr, &frame_header, header_packet_size);
        memcpy(video_shaddr+header_packet_size, &video_data, sizeof(video_data));
        break;
      }
      case kDepthData:
      {
        // Retrieve the depth data from the client.
        if (!do_recv(connected, depth_packet_size,
                     depth_payload, "depth", client_addr))
        {
          disconnect_client = true;
          break;
        }
        DepthData depth_data;
        depth_data.DeSerialize(depth_payload);
        memcpy(depth_shaddr, &frame_header, header_packet_size);
        memcpy(depth_shaddr+header_packet_size, &depth_data, sizeof(depth_data));
        break;
      }
      case kSkeletonData:
      {
        // Retrieve the skeleton data from the client.
        if (!do_recv(connected, skeleton_packet_size,
                     skeleton_payload, "skeleton", client_addr))
        {
          disconnect_client = true;
          break;
        }
        SkeletalData skeleton_data;
        skeleton_data.DeSerialize(skeleton_payload);
        memcpy(skele_shaddr, &frame_header, header_packet_size);
        memcpy(skele_shaddr+header_packet_size, &skeleton_data, sizeof(skeleton_data));
        break;
      }
      default:
      {
        printh(client_addr);
        printf("Got bad payload kind: %d, disconnecting.\n",
               frame_header.m_payloadKind);
        fflush(stdout);
        disconnect_client = true;
        break;
      }
    }
  } // while(!disconnect_client)

  // Cleanup shared and local memory.
  // Do not shutdown a server when clients are connected or shmem objects will leak!
  // TODO: Add a CTRL-C handler for last resort cleanup.
  destroy_memory_map(vid_shmem_name, video_packet_size+header_packet_size, video_shaddr, video_shfd);
  destroy_memory_map(depth_shmem_name, depth_packet_size+header_packet_size, depth_shaddr, depth_shfd);
  destroy_memory_map(skele_shmem_name, skeleton_packet_size+header_packet_size, skele_shaddr, skele_shfd);
  delete[] header_packet;
  delete[] video_payload;
  delete[] depth_payload;
  delete[] skeleton_payload;
  close(connected);
  printh(client_addr);
  printf("Disconnected client and cleaned up local resources.\n");
  fflush(stdout);
  exit(0);
}
