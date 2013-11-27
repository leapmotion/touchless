/*==================================================================================================================

    Copyright (c) 2010 - 2013 Leap Motion. All rights reserved.

  The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
  protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
  strictly forbidden unless prior written permission is obtained from Leap Motion.

===================================================================================================================*/

#if !defined(__LPScreen_h__)
#define __LPScreen_h__

#include "LPGeometry.h"

#if __APPLE__
typedef CGDirectDisplayID LPDirectDisplayID;
#elif _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
typedef HMONITOR LPDirectDisplayID;
#else
typedef unsigned long LPDirectDisplayID; // Linux -- FIXME
#endif
#include <stdint.h>

class LPScreen {
  public:
    LPScreen(const LPDirectDisplayID& screenID, uint32_t screenIndex);

    LPDirectDisplayID ID() const { return m_screenID; }
    int Index() const { return m_screenIndex; } // For now ... hopefully we will use display ID going forward

    bool IsPrimary() const { return m_isPrimary; }

    LPRect Bounds() const { return m_bounds; }
    LPPoint Origin() const { return m_bounds.origin; }
    LPSize Size() const { return m_bounds.size; }
    LPFloat X() const { return m_bounds.origin.x; }
    LPFloat Y() const { return m_bounds.origin.y; }
    LPFloat Width() const { return m_bounds.size.width; }
    LPFloat Height() const { return m_bounds.size.height; }

    LPFloat AspectRatio() const;

    void Update();

  private:
    LPDirectDisplayID m_screenID;
    uint32_t m_screenIndex;
    LPRect m_bounds;
    bool m_isPrimary;
};

#endif // __LPScreen_h__
