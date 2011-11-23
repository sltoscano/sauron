
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
#include <unistd.h>

#include "receiver.h"
#include "sender.h"

// How many pending connections the queue will hold.
#define BACKLOG 10

// The port that this server will listen on
#define OUTSIDEPORT 6969


static void sigchld_handler(int s)
{
  while (waitpid(-1, 0, WNOHANG) > 0);
}

int main()
{
  printf("Starting server on port %d...\n", OUTSIDEPORT);

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
  server_addr.sin_port = htons(OUTSIDEPORT);
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

  printf("Sauron(server:%d) waiting for client connections\n", OUTSIDEPORT);
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

    printf("Sauron(server:%d) got a connection from: (%s, %d)\n",
      OUTSIDEPORT, inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    fflush(stdout);

    // Spawn a child process and receive packets for this connection.
    pid_t recv_child = fork();

    // The pid will be used to lookup shared memory for each payload kind.
    if (!recv_child)
    {
      recv_loop(connected, client_addr);
    }

    // Spawn another child process to send packets for this connection.
    if (!fork())
    {
      close(sock);
      close(connected);
      sender(recv_child);
    }
/*
    // Spawn another child process to archive packets for this connection.
    if (!fork())
    {
      close(sock);
      close(connected);
      arch_loop(recv_child);
    }
*/
    // Parent doesn't need this.
    close(connected);
  }

  close(sock);
  return 0;
}
