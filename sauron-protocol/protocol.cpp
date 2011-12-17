//
// protocol.cpp - Protocol definitions for network packets
//

#include "transport.h"
#include "protocol.h"


int DepthData::Serialize(unsigned char* buffer)
{
  return 0;
}

void DepthData::DeSerialize(unsigned char* buffer)
{
}

int VideoData::Serialize(unsigned char* buffer)
{
  return 0;
}

void VideoData::DeSerialize(unsigned char* buffer)
{
}

char SkeletalData::Format[] = "ll";

int SkeletalData::Serialize(unsigned char* buffer)
{
  int size = 0;
  int totalSize = 0;
  for (int i=0; i < SRN_MAX_JOINTS; ++i)
  {
    size = pack(buffer, SkeletalData::Format,
                m_joints[i].x, m_joints[i].y);
    buffer += size;
    totalSize += size;
  }
  return totalSize;
}

void SkeletalData::DeSerialize(unsigned char* buffer)
{
  for (int i=0; i < SRN_MAX_JOINTS; ++i)
  {
    unpack(buffer, SkeletalData::Format,
      &m_joints[i].x,
      &m_joints[i].y);
    buffer += 8;
  }
}

char SauronFrameHeader::Format[] = "llllllL";

int SauronFrameHeader::Serialize(unsigned char* buffer)
{
  return pack(buffer, SauronFrameHeader::Format,
    m_signature,
    m_headerSize,
    m_payloadSize,
    m_payloadKind,
    m_cameraId,
    m_actorId,
    m_clientTimestamp);
}

void SauronFrameHeader::DeSerialize(unsigned char* buffer)
{
  unpack(buffer, SauronFrameHeader::Format,
    &m_signature,
    &m_headerSize,
    &m_payloadSize,
    &m_payloadKind,
    &m_cameraId,
    &m_actorId,
    &m_clientTimestamp);
}
