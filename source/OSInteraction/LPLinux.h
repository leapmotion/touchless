/*==================================================================================================================

    Copyright (c) 2010 - 2013 Leap Motion. All rights reserved.

  The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
  protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
  strictly forbidden unless prior written permission is obtained from Leap Motion.

===================================================================================================================*/

#if !defined(__LPLinux_h__)
#define __LPLinux_h__

#include "Utility/LPGeometry.h"
#include <string>

class CFocusAppInfo {
  public:
    CFocusAppInfo();
    std::wstring m_windowTitle;
    std::wstring m_ownerExeName;
    RECT m_rcClient;

    void Update();

  private:
    bool m_hasFrontmostApplication;
};

#endif // __LPLinux_h__
