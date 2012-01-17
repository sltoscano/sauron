#include "stdafx.h"
#include "SkeletalViewer.h"
#include <sstream>

#include "actor_detector.h"

using namespace std;

namespace Sauron
{
  ActorDetector::ActorDetector() :
      m_lastUpdateTime(0)
    , m_next(0)
  {
    m_output.open("data.csv", ios::out | ios::app);
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

    RECT rt;
    GetWindowRect(hWnd, &rt);
    rt.top = 10;
    rt.bottom = 30;

    // Only take a snapshot of the skeletal lengths if the actor is within
    // a certain distance from the camera.
    if ((spinePos.z > (SNAPSHOT_DEPTH - 0.05f)) &&
      (spinePos.z < (SNAPSHOT_DEPTH + 0.05f)))
    {
      m_spinePosHist[m_next] = spinePos.z;
      m_next ^= 1;

      if (m_spinePosHist[m_next] == 0)
      {
        m_spinePosHist[m_next] = spinePos.z;
        return;
      }

      // Ensure the actor is walking towards the camera
      if (m_spinePosHist[m_next] <= m_spinePosHist[m_next ^ 1])
      {
        return;
      }

      Beep(550, 50);
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

        // Print out lengths to screen
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

        // Print out lengths to a file
        m_output << lengths[i];
        m_output << ",";
      }
      m_output << "\n";
    }
  }
}
