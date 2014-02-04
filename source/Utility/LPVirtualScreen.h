/*==================================================================================================================

    Copyright (c) 2010 - 2013 Leap Motion. All rights reserved.

  The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
  protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
  strictly forbidden unless prior written permission is obtained from Leap Motion.

===================================================================================================================*/

#if !defined(__LPVirtualScreen_h__)
#define __LPVirtualScreen_h__

#include "LPScreen.h"
#include <vector>

class LPVirtualScreen {
  public:
    LPVirtualScreen();
    ~LPVirtualScreen();

    LPRect Bounds(bool activeOnly = true) const { return activeOnly ? m_activeBounds : m_detectedBounds; }

    LPPoint Normalize(const LPPoint& position, bool activeOnly = true) const;
    LPPoint Denormalize(const LPPoint& position, bool activeOnly = true) const;

    LPPoint Position() const { return m_position; }
    LPPoint SetPosition(const LPPoint& position, uint32_t* screenIndex = 0);
    LPPoint ClipPosition(const LPPoint& position, uint32_t* screenIndex = 0) const;

    LPScreen ClosestScreen() const { return ClosestScreen(m_position); }
    LPScreen ClosestScreen(const LPPoint& position) const;

    LPFloat AspectRatio() const;

    void UseDefaultScreen(bool useDefaultScreen);
    bool UsingDefaultScreen() const;

    void Update();
    size_t NumScreens(bool activeOnly = true) const { return activeOnly ? m_activeScreens.size() : m_detectedScreens.size(); }

  private:
    LPRect ComputeBounds(const std::vector<LPScreen>& screens);

#if _WIN32
    static BOOL CALLBACK EnumerateDisplays(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData);
    void makeDummyWindow();
    LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);
    HWND m_hWnd;
    friend class LPDummyWindowClass;
#elif __APPLE__
    static void ConfigurationChangeCallback(CGDirectDisplayID display, CGDisplayChangeSummaryFlags flags, void *that);
#endif
    std::vector<LPScreen> m_activeScreens;
    std::vector<LPScreen> m_detectedScreens;
    uint32_t m_screenIndex;
    LPPoint m_position;
    LPRect m_activeBounds;
    LPRect m_detectedBounds;
    bool m_useDefaultScreen;
};

#if _WIN32
class LPDummyWindowClass
{
public:
  static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
  static ATOM GetAtom() { static LPDummyWindowClass s_instance; return s_instance.m_atom; }
private:
  LPDummyWindowClass();
  ~LPDummyWindowClass();
  WNDCLASSW m_wndClass;
  ATOM m_atom;
};
#endif

#endif // __LPVirtualScreen_h__
