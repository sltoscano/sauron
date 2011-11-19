
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>

int main(int argc, char* argv[])
{
	int sock;
	int bytes_recieved;
	
	char send_data[1024];
	char recv_data[1024];

	hostent* host;
	sockaddr_in server_addr;
	
	if (argc < 2)
	{
		printf("usage: %s <ip address>", argv[0]);
		return 1;
	}

	host = gethostbyname(argv[1]);

	if ((sock = socket(AF_INET, SOCK_STREAM, 0)) == -1)
	{
		perror("Socket");
		exit(1);
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(5000);
	server_addr.sin_addr = *((in_addr *)host->h_addr);
	bzero(&server_addr.sin_zero, 8);

	if (connect(sock, (sockaddr *)&server_addr, sizeof(sockaddr)) == -1)
	{
		perror("Connect");
		exit(1);
	}

	while(1)
	{
		bytes_recieved = recv(sock, recv_data, 1024, 0);
		recv_data[bytes_recieved] = '\0';

		if (strcmp(recv_data, "q") == 0 || strcmp(recv_data, "Q") == 0)
		{
			close(sock);
			break;
		}
		else
		{
			printf("\nRecieved data = %s", recv_data);
		}

		printf("\nSEND (q or Q to quit): ");
		gets(send_data);

		if (strcmp(send_data, "q") == 0 || strcmp(send_data, "Q") == 0)
		{
			send(sock, send_data, strlen(send_data), 0);
			close(sock);
			break;
		}
		else
		{
			send(sock, send_data, strlen(send_data), 0);
		}
	}

	return 0;
}
