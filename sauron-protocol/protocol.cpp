//
// protocol.cpp - Protocol definitions for network packets
//

#pragma once

#include "transport.h"
#include "protocol.h"


int DepthData::Serialize(unsigned char* buffer)
{
  return 0;
}

bool DepthData::DeSerialize(const char* buffer, DepthData& data)
{
  return false;
}

int VideoData::Serialize(unsigned char* buffer)
{
  return 0;
}

bool VideoData::DeSerialize(const char* buffer, VideoData& data)
{
  return false;
}

int SkeletalData::Serialize(unsigned char* buffer)
{
  int size = 0;
  int totalSize = 0;
  for (int i=0; i < SRN_MAX_JOINTS; ++i)
  {
    size = pack(buffer, "ll", m_joints[i].x, m_joints[i].y);
    buffer += size;
    totalSize += size;
  }
  return totalSize;
}

bool SkeletalData::DeSerialize(const char* buffer, SkeletalData& data)
{
  return false;
}

int SauronFrameHeader::Serialize(unsigned char* buffer)
{
  return pack(buffer, "lllllL",
    m_headerSize,
    m_payloadSize,
    m_payloadKind,
    m_cameraId,
    m_actorId,
    m_clientTimestamp);
}

bool SauronFrameHeader::DeSerialize(const char* buffer, SauronFrameHeader& data)
{
  return false;
}
