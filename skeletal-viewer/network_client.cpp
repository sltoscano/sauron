
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <winsock2.h>
#include <Ws2tcpip.h>

#include "network_client.h"


bool NetworkClient::NetworkStartup(const char* hostname, int port)
{
    //----------------------
    // Declare and initialize variables.
    WSADATA wsaData;
    int iResult;

    //----------------------
    // Initialize Winsock
    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != NO_ERROR) {
        printf("WSAStartup failed: %d\n", iResult);
        return false;
    }

    struct hostent *host;
    struct sockaddr_in server_addr;  

    host = gethostbyname(hostname);
    if ((m_sock = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket");
        return false;
    }

    server_addr.sin_family = AF_INET;     
    server_addr.sin_port = htons(port);   
    server_addr.sin_addr = *((struct in_addr *)host->h_addr);
    ZeroMemory(&(server_addr.sin_zero),8);

    if (connect(m_sock, (struct sockaddr *)&server_addr,
        sizeof(struct sockaddr)) == -1) 
    {
        perror("Connect");
        return false;
    }

    return true;
}

void NetworkClient::NetworkShutdown()
{
    if (m_sock)
    {
        closesocket(m_sock);
    }
    WSACleanup();
}

extern void packi32(unsigned char *buf, unsigned long int i);

bool NetworkClient::NetworkBlockingSend(SkeletalData& data)
{
    if (m_sock == 0)
    {
        return false;
    }

    int bytes_sent;

    int countof_joints = sizeof(data.m_joints)/sizeof(data.m_joints[0]);

    int packet_length = countof_joints*2*4;
    unsigned char* send_data = new unsigned char[packet_length];
    ZeroMemory(send_data, sizeof(char)*packet_length);
    unsigned char* data_buffer = send_data;

    for (int i=0; i < countof_joints; ++i)
    {
        packi32(data_buffer, data.m_joints[i].x);
        data_buffer+=4;
        packi32(data_buffer, data.m_joints[i].y);
        data_buffer+=4;
    }

    bytes_sent = send(m_sock, (const char*) send_data, packet_length, 0);
    delete[] send_data;
    return bytes_sent > 0;
}
