
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

#define PACKETSIZE (20*2*4)

int main()
{
  printf("starting server...\n");
  
  int sock;
  int connected;
  int bytes_recieved;
  int result = 1;

  char recv_data[PACKETSIZE];

  sockaddr_in server_addr;
  sockaddr_in client_addr;
  socklen_t sin_size;

  if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
  {
    perror("Socket");
    exit(1);
  }

  if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &result, sizeof(int)) == -1)
  {
    perror("Setsockopt");
    exit(1);
  }

  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(6969);
  server_addr.sin_addr.s_addr = INADDR_ANY;
  bzero(&server_addr.sin_zero, 8);

  if (bind(sock, (sockaddr *)&server_addr, sizeof(sockaddr)) == -1)
  {
    perror("unable to bind");
    exit(1);
  }

  if (listen(sock, 5) == -1)
  {
    perror("Listen");
    exit(1);
  }

  printf("\nTCPServer waiting for client on port 6969");
  fflush(stdout);

  while(1)
  {
    sin_size = sizeof(sockaddr_in);

    connected = accept(sock, (struct sockaddr *)&client_addr, &sin_size);
    
    printf("\nGot a connection from: (%s, %d)",
      inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    fflush(stdout);

    while (1)
    {
      char* recv_pos = (char*) &recv_data;

      bytes_recieved = recv(connected, recv_data, PACKETSIZE, 0);

      while (bytes_recieved < PACKETSIZE)
      {
        recv_pos += bytes_recieved;
        bytes_recieved += recv(connected, recv_pos, PACKETSIZE, 0);
      }

      for (int i=0; i<PACKETSIZE; ++i)
      {
        printf("%x ", recv_data[i]);
      }
      printf("\n");

      fflush(stdout);
    }
  }

  close(connected);
  close(sock);
  return 0;
}
