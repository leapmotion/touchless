/*==================================================================================================================

    Copyright (c) 2010 - 2013 Leap Motion. All rights reserved.

  The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
  protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
  strictly forbidden unless prior written permission is obtained from Leap Motion.

===================================================================================================================*/
#include "stdafx.h"
#include "LPScreen.h"

//
// LPScreen
//

LPScreen::LPScreen(const LPDirectDisplayID& screenID, uint32_t screenIndex) : m_screenID(screenID),
                                                                              m_screenIndex(screenIndex),
                                                                              m_isPrimary(false)
{
  Update();
}

LPFloat LPScreen::AspectRatio() const
{
  if (Height() < 1) {
    return static_cast<LPFloat>(1);
  }
  return Width()/Height();
}

void LPScreen::Update()
{
#if __APPLE__
  m_bounds = CGDisplayBounds(m_screenID);
  m_isPrimary = CGDisplayIsMain(m_screenID);
#elif _WIN32
  MONITORINFOEX info;
  info.cbSize = sizeof(MONITORINFOEX);
  GetMonitorInfo(m_screenID, &info);
  info.rcMonitor.left;
  info.rcMonitor.top;
  info.rcMonitor.right;
  info.rcMonitor.bottom;
  m_bounds = LPRect(static_cast<LPFloat>(info.rcMonitor.left),
                    static_cast<LPFloat>(info.rcMonitor.top),
                    static_cast<LPFloat>(info.rcMonitor.right - info.rcMonitor.left),
                    static_cast<LPFloat>(info.rcMonitor.bottom - info.rcMonitor.top));
  m_isPrimary = ((info.dwFlags & MONITORINFOF_PRIMARY) == MONITORINFOF_PRIMARY);
#else
  // Linux -- FIXME
#endif
}
