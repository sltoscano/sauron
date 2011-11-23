
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <errno.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <_types.h>
#include <unistd.h>

#include "../../sauron-protocol/protocol.h"
#include "shmem.h"
#include "sender.h"

// How many pending connections the queue will hold.
#define BACKLOG 10


// Print a client header 
static void printh(const sockaddr_in& client_addr)
{
  printf("\tviewer_client(%s, %d) ",
         inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
}

static void sigchld_handler(int s)
{
  while (waitpid(-1, 0, WNOHANG) > 0);
}

void sender(pid_t shmem_key)
{
  printf("Starting sender on port %d...\n", shmem_key);

  int sock = 0;
  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("Socket");
    exit(1);
  }

  int result = 1;
  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &result, sizeof(int)) == -1)
  {
    perror("Setsockopt");
    exit(1);
  }

  sockaddr_in server_addr;
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(shmem_key);
  server_addr.sin_addr.s_addr = INADDR_ANY;
  bzero(&server_addr.sin_zero, 8);

  if (bind(sock, (sockaddr *)&server_addr, sizeof(sockaddr)) == -1)
  {
    perror("Unable to bind");
    exit(1);
  }

  if (listen(sock, BACKLOG) == -1)
  {
    perror("Listen");
    exit(1);
  }

  // Reap all dead processes.
  struct sigaction sa;
  sa.sa_handler = sigchld_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGCHLD, &sa, 0) == -1)
  {
    perror("Sigaction");
    exit(1);
  }

  printf("Sauron(sender:%d) waiting for client connections\n", shmem_key);
  fflush(stdout);

  sockaddr_in client_addr;

  while(1)
  {
    socklen_t sin_size = sizeof(sockaddr_in);

    int connected = accept(sock, (struct sockaddr *)&client_addr, &sin_size);
    if (connected == -1)
    {
      perror("Accept");
      continue;
    }

    printf("Sauron(sender:%d) got a connection from: (%s, %d)\n",
           shmem_key, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
    
    fflush(stdout);

    // Spawn child process to send packets for this connection.
    if (!fork())
    {
      send_loop(shmem_key, connected, client_addr);
    }
    close(connected);
  }

  close(sock);
  exit(0);
}

bool do_send(int connected, int send_size, unsigned char* send_buffer,
             const char* name, const sockaddr_in& client_addr)
{
  memset(send_buffer, 0, send_size);
  int packet_size = send_size;
  unsigned char* packet_buffer = send_buffer;

  int bytes_send = 0;
  int send_result = -1;
  int total_packet_size = send_size;

  do
  {
    send_result = send(connected, packet_buffer, packet_size, 0);
    bytes_send += send_result;
    packet_buffer += send_result;
    packet_size -= send_result;
  }
  while (send_result > 0 && bytes_send < total_packet_size);

  // Check if the client disconnected.
  if (send_result == 0)
  {
    printh(client_addr);
    printf("Client disconnected during %s data send, disconnecting.\n", name);
    fflush(stdout);
    return false;
  }
  else if (send_result == -1)
  {
    printh(client_addr);
    printf("Client send failed during %s data recv, disconnecting.\n", name);
    fflush(stdout);
    return false;
  }

  return true;
}

void send_loop(pid_t shmem_key, int connected, sockaddr_in client_addr)
{
  bool disconnect_client = false;

  int header_packet_size = sizeof(SauronFrameHeader);
  __int64_t shmem_size = 0;

  char vid_shmem_name[20] = "";
  sprintf(vid_shmem_name, "SRN_%d_%d", kVideoFrame, shmem_key);
  char depth_shmem_name[20] = "";
  sprintf(depth_shmem_name, "SRN_%d_%d", kDepthData, shmem_key);
  char skele_shmem_name[20] = "";
  sprintf(skele_shmem_name, "SRN_%d_%d", kSkeletonData, shmem_key);

  // Allocate local and shared memory for the video traffic
  int video_packet_size = sizeof(VideoData);
  unsigned char* video_payload = new unsigned char[video_packet_size+header_packet_size];
  char* video_shaddr = 0;
  int video_shfd = -1;
  shmem_size = video_packet_size+header_packet_size;
  if(!open_memory_map(vid_shmem_name, shmem_size, (void**)&video_shaddr, &video_shfd) ||
     shmem_size != video_packet_size+header_packet_size)
  {
    printh(client_addr);
    printf("Error opening shared memory object: '%s'", vid_shmem_name);
    fflush(stdout);
    disconnect_client = true;
  }
  // Allocate local and shared memory for the depth traffic
  int depth_packet_size = sizeof(DepthData);
  unsigned char* depth_payload = new unsigned char[depth_packet_size+header_packet_size];
  char* depth_shaddr = 0;
  int depth_shfd = -1;
  shmem_size = depth_packet_size+header_packet_size;
  if(!open_memory_map(depth_shmem_name, shmem_size, (void**)&depth_shaddr, &depth_shfd) ||
     shmem_size != depth_packet_size+header_packet_size)
  {
    printh(client_addr);
    printf("Error opening shared memory object: '%s'", depth_shmem_name);
    fflush(stdout);
    disconnect_client = true;
  }
  // Allocate local and shared memory for the skeleton traffic
  int skeleton_packet_size = sizeof(SkeletalData);
  unsigned char* skele_payload = new unsigned char[skeleton_packet_size+header_packet_size];
  char* skele_shaddr = 0;
  int skele_shfd = -1;
  shmem_size = skeleton_packet_size+header_packet_size;
  if(!open_memory_map(skele_shmem_name, shmem_size, (void**)&skele_shaddr, &skele_shfd) ||
     shmem_size != skeleton_packet_size+header_packet_size)
  {
    printh(client_addr);
    printf("Error opening shared memory object: '%s'", skele_shmem_name);
    fflush(stdout);
    disconnect_client = true;
  }

  unsigned int video_prev_timestamp = 0;
  unsigned int depth_prev_timestamp = 0;
  unsigned int skele_prev_timestamp = 0;

  while(!disconnect_client)
  {
    // Build a header for the streaming viewer
    SauronFrameHeader video_header;
    memcpy(&video_header, video_shaddr, header_packet_size);

    // Check for data validity and duplicate frames
    if (video_header.m_signature == 'SRN1' &&
        video_header.m_clientTimestamp != video_prev_timestamp)
    {
      video_header.DeSerialize(video_payload);
      VideoData video_data;
      memcpy(&video_data, video_shaddr+header_packet_size, sizeof(video_data));
      video_data.DeSerialize(video_payload+header_packet_size);

      if (!do_send(connected, video_packet_size+header_packet_size,
                   video_payload, "video", client_addr))
      {
        break;
      }
    }

    // Build a header for the streaming viewer
    SauronFrameHeader depth_header;
    memcpy(&depth_header, depth_shaddr, header_packet_size);

    // Check for data validity and duplicate frames
    if (depth_header.m_signature == 'SRN1' &&
        depth_header.m_clientTimestamp != depth_prev_timestamp)
    {
      depth_header.DeSerialize(video_payload);
      DepthData depth_data;
      memcpy(&depth_data, depth_shaddr+header_packet_size, sizeof(depth_data));
      depth_data.DeSerialize(depth_payload+header_packet_size);

      if (!do_send(connected, depth_packet_size+header_packet_size,
                   depth_payload, "depth", client_addr))
      {
        break;
      }
    }

    // Build a header for the streaming viewer
    SauronFrameHeader skele_header;
    memcpy(&skele_header, skele_shaddr, header_packet_size);

    // Check for data validity and duplicate frames
    if (skele_header.m_signature == 'SRN1' &&
        skele_header.m_clientTimestamp != skele_prev_timestamp)
    {
      skele_header.DeSerialize(skele_payload);
      SkeletalData skele_data;
      memcpy(&skele_data, skele_shaddr+header_packet_size, sizeof(skele_data));
      skele_data.DeSerialize(skele_payload+header_packet_size);

      if (!do_send(connected, skeleton_packet_size+header_packet_size,
                   skele_payload, "skeleton", client_addr))
      {
        break;
      }
    }

  } // while(!disconnect_client)

  // Cleanup shared and local memory.
  // Do not shutdown a server when clients are connected or shmem objects will leak!
  // TODO: Add a CTRL-C handler for last resort cleanup.
  close_memory_map(video_packet_size+header_packet_size, video_shaddr, video_shfd);
  close_memory_map(depth_packet_size+header_packet_size, depth_shaddr, depth_shfd);
  close_memory_map(skeleton_packet_size+header_packet_size, skele_shaddr, skele_shfd);
  delete[] video_payload;
  delete[] depth_payload;
  delete[] skele_payload;
  close(connected);
  printh(client_addr);
  printf("Disconnected client and cleaned up local resources.\n");
  fflush(stdout);
  exit(0);
}
