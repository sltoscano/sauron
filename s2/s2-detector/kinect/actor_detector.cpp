#include "stdafx.h"
#include <time.h>
#include <sstream>
#include "SkeletalViewer.h"

#include "actor_detector.h"

using namespace std;

namespace Sauron
{
  ActorDetector::ActorDetector() :
      m_lastUpdateTime(0)
    , m_next(0)
  {
    m_output.open("data.csv", ios::out | ios::app);

    // Header
    m_output << "date,";
    m_output << "time,";
    m_output << "actor,";
    m_output << "HIP_CENTER to SPINE,";
    m_output << "SPINE to SHOULDER_CENTER,";
    m_output << "SHOULDER_CENTER to HEAD,";

    // Top left side of body
    m_output << "SHOULDER_CENTER to SHOULDER_LEFT,";
    m_output << "SHOULDER_LEFT to ELBOW_LEFT,";
    m_output << "ELBOW_LEFT to WRIST_LEFT,";
    m_output << "WRIST_LEFT to HAND_LEFT,";

    // Top right side of body
    m_output << "SHOULDER_CENTER to SHOULDER_RIGHT,";
    m_output << "SHOULDER_RIGHT to ELBOW_RIGHT,";
    m_output << "ELBOW_RIGHT to WRIST_RIGHT,";
    m_output << "WRIST_RIGHT to HAND_RIGHT,";

    // Bottom left side of body
    m_output << "HIP_CENTER to HIP_LEFT,";
    m_output << "HIP_LEFT to KNEE_LEFT,";
    m_output << "KNEE_LEFT to ANKLE_LEFT,";
    m_output << "ANKLE_LEFT to FOOT_LEFT,";

    // Bottom right side of body
    m_output << "HIP_CENTER to HIP_RIGHT,";
    m_output << "HIP_RIGHT to KNEE_RIGHT,";
    m_output << "KNEE_RIGHT to ANKLE_RIGHT,";
    m_output << "ANKLE_RIGHT to FOOT_RIGHT,";
    m_output << "\n";
  }

  ActorDetector::~ActorDetector()
  {
    m_output.close();
  }

  void ActorDetector::FindSkeletalLengths( NUI_SKELETON_DATA * pSkel, HWND hWnd, int WhichSkeletonColor )
  {
    DWORD currentTime = GetTickCount();
    if ((currentTime - m_lastUpdateTime) > 50)
    {
      m_spinePosHist[m_next] = 0;
      m_spinePosHist[m_next ^ 1] = 0;
    }
    m_lastUpdateTime = currentTime;

    static const FLOAT SNAPSHOT_DEPTH = 2.80f;
    const Vector4& spinePos = pSkel->SkeletonPositions[NUI_SKELETON_POSITION_SPINE];

    FLOAT lengths[SEGMENT_COUNT];

    /*
    RECT rt;
    GetWindowRect(hWnd, &rt);
    rt.top = 10;
    rt.bottom = 30;
    */

    FLOAT f = SQUARE(pSkel->SkeletonPositions[NUI_SKELETON_POSITION_KNEE_LEFT].x - pSkel->SkeletonPositions[NUI_SKELETON_POSITION_ANKLE_LEFT].x) +
      SQUARE(pSkel->SkeletonPositions[NUI_SKELETON_POSITION_KNEE_LEFT].y - pSkel->SkeletonPositions[NUI_SKELETON_POSITION_ANKLE_LEFT].y);

    FLOAT g = SQUARE(pSkel->SkeletonPositions[NUI_SKELETON_POSITION_KNEE_RIGHT].x - pSkel->SkeletonPositions[NUI_SKELETON_POSITION_ANKLE_RIGHT].x) +
      SQUARE(pSkel->SkeletonPositions[NUI_SKELETON_POSITION_KNEE_RIGHT].y - pSkel->SkeletonPositions[NUI_SKELETON_POSITION_ANKLE_RIGHT].y);

    // Only take a snapshot of the skeletal lengths if the actor is within
    // a certain distance from the camera.
    if (((spinePos.z > (SNAPSHOT_DEPTH - 0.2f)) &&
         (spinePos.z < (SNAPSHOT_DEPTH + 0.2f))) &&
        ((f - g) < 0.001f))
    {
      m_spinePosHist[m_next] = spinePos.z;
      m_next ^= 1;

      if (m_spinePosHist[m_next] == 0)
      {
        m_spinePosHist[m_next] = spinePos.z;
        return;
      }

      // Ensure the actor is walking towards the camera.
      if (m_spinePosHist[m_next] <= m_spinePosHist[m_next ^ 1])
      {
        return;
      }

      // Sound for audio feedback of recognized person.
      Beep(550, 50);

      char dateStr[9];
      char timeStr[9];
      _strdate_s(dateStr);
      _strtime_s(timeStr);
      m_output << dateStr;
      m_output << ",";
      m_output << timeStr;
      m_output << ",";
      m_output << WhichSkeletonColor;
      m_output << ",";

      for (int i = 0; i < SEGMENT_COUNT; ++i)
      {
        const NUI_SKELETON_POSITION_INDEX& a = g_Segments[i].a;
        const NUI_SKELETON_POSITION_INDEX& b = g_Segments[i].b;

        // Distance Formula:  x(a), y(a) to x(b), y(b)
        // SQRT(
        //       ( (x(b) - x(a)) ^ 2 ) +
        //       ( (y(b) - y(a)) ^ 2 )
        // )
        // To save processing time skip the SQRT.
        //
        lengths[i] = SQUARE(pSkel->SkeletonPositions[b].x - pSkel->SkeletonPositions[a].x) +
                     SQUARE(pSkel->SkeletonPositions[b].y - pSkel->SkeletonPositions[a].y);

        // Print out lengths to screen.
        /*
        std::wstringstream s;
        s << i << L"=" << lengths[i];
        DrawText(GetDC(hWnd), s.str().c_str(), 10, &rt, DT_LEFT);
        rt.top += 20;
        rt.bottom += 20;

        if (i == 9)
        {
          rt.top = 10;
          rt.bottom = 30;
          rt.left += 100;
          rt.right += 100;
        }
        */

        // Print out lengths to a file.
        m_output << lengths[i];
        m_output << ",";
      }
      m_output << "\n";
      m_output.flush();
    }
  }
}
