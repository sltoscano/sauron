//
// protocol.cpp - Protocol definitions for network packets
//

#pragma once

#include "memory.h"

#define SRN_MAX_JOINTS 20


class RGBData
{
public:
  RGBData() :
    m_width(0),
    m_height(0),
    m_buffer(0)
  {
  }
  int m_width;
  int m_height;
  unsigned char* m_buffer;
};

class DepthData : public RGBData
{
public:
  int Serialize(unsigned char* buffer);
  void DeSerialize(unsigned char* buffer);
};

class VideoData : public RGBData
{
public:
  int Serialize(unsigned char* buffer);
  void DeSerialize(unsigned char* buffer);
};

class SkeletalData
{
public:
  SkeletalData()
  {
    memset(m_joints, 0, sizeof(m_joints));
  };
  struct Point
  {
    int x;
    int y;
  };
  Point m_joints[SRN_MAX_JOINTS];

  static char Format[];

  int Serialize(unsigned char* buffer);
  void DeSerialize(unsigned char* buffer);
};

typedef enum _SauronFrameKind
{
  kInvalidFrameKind = -1,
  kVideoFrame = 0,
  kDepthData,
  kSkeletonData,
  kFrameKindCount,
} SauronFrameKind;

class SauronFrameHeader
{
public:
  SauronFrameHeader() :
    m_signature('SRN1'),
    m_headerSize(0),
    m_payloadSize(0),
    m_payloadKind(kInvalidFrameKind),
    m_cameraId(0),
    m_actorId(0),
    m_clientTimestamp(0)
  {
  }
  int m_signature;
  int m_headerSize;
  int m_payloadSize;
  SauronFrameKind m_payloadKind;
  int m_cameraId;
  int m_actorId;
  unsigned int m_clientTimestamp;

  static char Format[];

  int Serialize(unsigned char* buffer);
  void DeSerialize(unsigned char* buffer);
};
