#pragma once

#include "skeletal_data.h"


class NetworkClient
{
public:
    NetworkClient() : m_sock(0) {}
    bool NetworkStartup(const char* hostname, int port);
    void NetworkShutdown();
    bool NetworkBlockingSend(SkeletalDataPacket& packet);

private:
    int m_sock;
};
