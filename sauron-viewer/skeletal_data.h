#pragma once

#include "linked_queue.cpp"
#include "../sauron-protocol/protocol.h"


class SkeletalDataPacket
{
public:
  SauronFrameHeader header;
  SkeletalData data;
};

typedef LinkedQueue<SkeletalDataPacket> SkeletalDataQueue;
