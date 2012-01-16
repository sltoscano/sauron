#pragma once

#include "skeletal_data.h"


class NetworkClient
{
public:
    NetworkClient() : m_sock(0) {}
    bool NetworkStartup(const char* hostname, int port);
    void NetworkShutdown();
    bool NetworkBlockingSend(SkeletalDataPacket& packet);
    bool NetworkBlockingRecv(SkeletalData& packet);
    bool NetworkBlockingRecv(SauronFrameHeader& packet);

private:
    int m_sock;
};
