#pragma once


class SkeletalData
{
public:
  SkeletalData() : 
      m_actorId(-1),
      m_transmitted(false)
  {
      ZeroMemory(m_joints, sizeof(m_joints));
  };
  POINT m_joints[20];
  int m_actorId;
  bool m_transmitted;
};
