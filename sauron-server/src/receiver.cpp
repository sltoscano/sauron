
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

// Print a client header 
void printh(const sockaddr_in& client_addr)
{
  printf("\t(%s, %d) ",
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
  int header_packet_size = sizeof(SauronFrameHeader);
  unsigned char* header_packet = new unsigned char[header_packet_size];
  int video_packet_size = sizeof(VideoData);
  unsigned char* video_payload = new unsigned char[video_packet_size];
  int depth_packet_size = sizeof(DepthData);
  unsigned char* depth_payload = new unsigned char[depth_packet_size];
  int skeleton_packet_size = sizeof(SkeletalData);
  unsigned char* skeleton_payload = new unsigned char[skeleton_packet_size];

  while(1)
  {
    bool disconnect_client = false;

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

    if (disconnect_client)
    {
      break;
    }
  } // while(1)
  
  delete[] header_packet;
  delete[] video_payload;
  delete[] depth_payload;
  delete[] skeleton_payload;
  close(connected);
  exit(0);
}
