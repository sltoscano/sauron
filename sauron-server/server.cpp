
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

#define PACKETSIZE (20*2*4)

// How many pending connections the queue will hold.
#define BACKLOG 10

void sigchld_handler(int s)
{
  while (waitpid(-1, 0, WNOHANG) > 0);
}

int main()
{
  printf("Starting server...\n");
  
  int sock;
  int connected;
  int bytes_recieved;
  int result = 1;

  char recv_data[PACKETSIZE];

  sockaddr_in server_addr;
  sockaddr_in client_addr;
  socklen_t sin_size;

  struct sigaction sa;

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
    perror("Unable to bind");
    exit(1);
  }

  if (listen(sock, BACKLOG) == -1)
  {
    perror("Listen");
    exit(1);
  }

  // Reap all dead processes.
  sa.sa_handler = sigchld_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = SA_RESTART;
  if (sigaction(SIGCHLD, &sa, 0) == -1)
  {
    perror("Sigaction");
    exit(1);
  }

  printf("Sauron waiting for client connections on port 6969\n");
  fflush(stdout);

  while(1)
  {
    sin_size = sizeof(sockaddr_in);

    connected = accept(sock, (struct sockaddr *)&client_addr, &sin_size);
    if (connected == -1)
    {
      perror("Accept");
      continue;
    }

    printf("Got a connection from: (%s, %d)i\n",
      inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    fflush(stdout);

    // Spawn a child process for this connection.
    if (!fork())
    {
      // Child doesn't need the listener.
      //close(sock);

      while(1)
      {
        char* recv_buf = (char*) &recv_data;

        bytes_recieved = recv(connected, recv_data, PACKETSIZE, 0);

        // Check if the client disconnected.
        if (bytes_recieved <= 0)
        {
          printf("Got a disconnect from: (%s, %d)\n",
            inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
          fflush(stdout);
          close(connected);
          break;
        }

        while (bytes_recieved < PACKETSIZE)
        {
          recv_buf += bytes_recieved;
          bytes_recieved += recv(connected, recv_buf, PACKETSIZE, 0);
        }

        for (int i=0; i<PACKETSIZE; ++i)
        {
          printf("%c ", recv_data[i]);
        }
        printf("\n");

        fflush(stdout);
      }
    }

    // Parent doesn't need this.
    close(connected);
  }

  close(sock);
  return 0;
}
