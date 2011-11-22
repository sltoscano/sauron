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
  static bool DeSerialize(const char* buffer, DepthData& data);
};

class VideoData : public RGBData
{
public:
  int Serialize(unsigned char* buffer);
  static bool DeSerialize(const char* buffer, VideoData& data);
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

  int Serialize(unsigned char* buffer);
  static bool DeSerialize(const char* buffer, SkeletalData& data);
};

typedef enum _SauronFrameKind
{
  kVideoFrame = 0,
  kDepthData,
  kSkeletonData,
  kKindCount,
} SauronFrameKind;

class SauronFrameHeader
{
public:
  int m_headerSize;
  int m_payloadSize;
  SauronFrameKind m_payloadKind;
  int m_cameraId;
  int m_actorId;
  unsigned int m_clientTimestamp;

  int Serialize(unsigned char* buffer);
  static bool DeSerialize(const char* buffer, SauronFrameHeader& data);
};
