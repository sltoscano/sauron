
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

bool NetworkClient::NetworkBlockingSend(SkeletalDataPacket& packet)
{
    if (m_sock == 0)
    {
        return false;
    }

    int packet_size = packet.header.m_headerSize + packet.header.m_payloadSize;
    unsigned char* send_header = new unsigned char[packet_size];
    memset(send_header, 0, packet_size);
    unsigned char* packet_buffer = send_header;
    packet.header.Serialize(send_header);
    packet_buffer += packet.header.m_headerSize;
    packet.data.Serialize(packet_buffer);

    int bytes_sent = 0;
    int send_result = -1;
    int total_packet_size = packet_size;
    packet_buffer = send_header;

    do
    {
      send_result = send(m_sock, (const char*) packet_buffer, packet_size, 0);
      bytes_sent += send_result;
      packet_buffer += send_result;
      packet_size -= send_result;
    }
    while (send_result > 0 && bytes_sent < total_packet_size);

    delete[] send_header;
    return bytes_sent == total_packet_size;
}

bool NetworkClient::NetworkBlockingRecv(SauronFrameHeader& packet)
{
  if (m_sock == 0)
  {
    return false;
  }

  int packet_size = sizeof(packet);
  unsigned char* recv_header = new unsigned char[packet_size];
  memset(recv_header, 0, packet_size);
  unsigned char* packet_buffer = recv_header;

  int bytes_recv = 0;
  int recv_result = -1;
  int total_packet_size = packet_size;
  packet_buffer = recv_header;

  do
  {
    recv_result = recv(m_sock, (char*) packet_buffer, packet_size, 0);
    bytes_recv += recv_result;
    packet_buffer += recv_result;
    packet_size -= recv_result;
  }
  while (recv_result > 0 && bytes_recv < total_packet_size);

  if (recv_result == 0)
  {
    OutputDebugString(L"peer disconnected!");
  }
  else if (recv_result == -1)
  {
    OutputDebugString(L"received error.");
  }
  else
  {
    packet.DeSerialize(recv_header);
  }

  delete[] recv_header;
  return bytes_recv == total_packet_size;
}

bool NetworkClient::NetworkBlockingRecv(SkeletalData& packet)
{
  if (m_sock == 0)
  {
    return false;
  }

  int packet_size = sizeof(packet);
  unsigned char* recv_buffer = new unsigned char[packet_size];
  memset(recv_buffer, 0, packet_size);
  unsigned char* packet_buffer = recv_buffer;

  int bytes_recv = 0;
  int recv_result = -1;
  int total_packet_size = packet_size;
  packet_buffer = recv_buffer;

  do
  {
    recv_result = recv(m_sock, (char*) packet_buffer, packet_size, 0);
    bytes_recv += recv_result;
    packet_buffer += recv_result;
    packet_size -= recv_result;
  }
  while (recv_result > 0 && bytes_recv < total_packet_size);

  if (recv_result == 0)
  {
    OutputDebugString(L"peer disconnected!");
  }
  else if (recv_result == -1)
  {
    OutputDebugString(L"received error.");
  }
  else
  {
    packet.DeSerialize(recv_buffer);
  }

  delete[] recv_buffer;
  return bytes_recv == total_packet_size;
}
