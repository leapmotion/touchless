/*==================================================================================================================

    Copyright (c) 2010 - 2013 Leap Motion. All rights reserved.

  The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
  protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
  strictly forbidden unless prior written permission is obtained from Leap Motion.

===================================================================================================================*/

#if !defined(__LPMac_h__)
#define __LPMac_h__

#include "Utility/LPGeometry.h"

#include <string>

// Helper class to bridge Objective-C++ implementations to C++
class LPMac {
  public:
    static CGEventTimestamp systemUptime();
    static void cursorPosition(float &x, float &y);

    static CGEventType EventTypeGesture;

  private:
    LPMac(); // No constructor, just static helper methods
};

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

class LPKeyboard {
  public:
    static CGKeyCode GetKeyCode(int key);

  private:
    LPKeyboard();

    enum {
      VK_BACK           = 0x08,
      VK_TAB            = 0x09,
      VK_RETURN         = 0x0D,
      VK_SHIFT          = 0x10,
      VK_CONTROL        = 0x11,
      VK_MENU           = 0x12,
      VK_CAPITAL        = 0x14,
      VK_ESCAPE         = 0x1B,
      VK_SPACE          = 0x20,
      VK_PRIOR          = 0x21,
      VK_NEXT           = 0x22,
      VK_END            = 0x23,
      VK_HOME           = 0x24,
      VK_LEFT           = 0x25,
      VK_UP             = 0x26,
      VK_RIGHT          = 0x27,
      VK_DOWN           = 0x28,
      VK_DELETE         = 0x2E,
      VK_HELP           = 0x2F,
      VK_F1             = 0x70,
      VK_F2             = 0x71,
      VK_F3             = 0x72,
      VK_F4             = 0x73,
      VK_F5             = 0x74,
      VK_F6             = 0x75,
      VK_F7             = 0x76,
      VK_F8             = 0x77,
      VK_F9             = 0x78,
      VK_F10            = 0x79,
      VK_F11            = 0x7A,
      VK_F12            = 0x7B,
      VK_F13            = 0x7C,
      VK_F14            = 0x7D,
      VK_F15            = 0x7E,
      VK_F16            = 0x7F,
      VK_F17            = 0x80,
      VK_F18            = 0x81,
      VK_F19            = 0x82,
      VK_F20            = 0x83,
      VK_LSHIFT         = 0xA0,
      VK_RSHIFT         = 0xA1,
      VK_LCONTROL       = 0xA2,
      VK_RCONTROL       = 0xA3,
      VK_LMENU          = 0xA4
    };

    enum {
      kVK_Return        = 0x24,
      kVK_Tab           = 0x30,
      kVK_Space         = 0x31,
      kVK_Delete        = 0x33,
      kVK_Escape        = 0x35,
      kVK_Command       = 0x37,
      kVK_Shift         = 0x38,
      kVK_CapsLock      = 0x39,
      kVK_Option        = 0x3A,
      kVK_Control       = 0x3B,
      kVK_RightShift    = 0x3C,
      kVK_RightOption   = 0x3D,
      kVK_RightControl  = 0x3E,
      kVK_Function      = 0x3F,
      kVK_F17           = 0x40,
      kVK_VolumeUp      = 0x48,
      kVK_VolumeDown    = 0x49,
      kVK_Mute          = 0x4A,
      kVK_F18           = 0x4F,
      kVK_F19           = 0x50,
      kVK_F20           = 0x5A,
      kVK_F5            = 0x60,
      kVK_F6            = 0x61,
      kVK_F7            = 0x62,
      kVK_F3            = 0x63,
      kVK_F8            = 0x64,
      kVK_F9            = 0x65,
      kVK_F11           = 0x67,
      kVK_F13           = 0x69,
      kVK_F16           = 0x6A,
      kVK_F14           = 0x6B,
      kVK_F10           = 0x6D,
      kVK_F12           = 0x6F,
      kVK_F15           = 0x71,
      kVK_Help          = 0x72,
      kVK_Home          = 0x73,
      kVK_PageUp        = 0x74,
      kVK_ForwardDelete = 0x75,
      kVK_F4            = 0x76,
      kVK_End           = 0x77,
      kVK_F2            = 0x78,
      kVK_PageDown      = 0x79,
      kVK_F1            = 0x7A,
      kVK_LeftArrow     = 0x7B,
      kVK_RightArrow    = 0x7C,
      kVK_DownArrow     = 0x7D,
      kVK_UpArrow       = 0x7E,
      kVK_Invalid       = 0xFF
    };
};

#endif // __LPMac_h__
