/*==================================================================================================================

    Copyright (c) 2010 - 2013 Leap Motion. All rights reserved.

  The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
  protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
  strictly forbidden unless prior written permission is obtained from Leap Motion.

===================================================================================================================*/
#include "stdafx.h"
#include "LPVirtualScreen.h"

//
// LPVirtualScreen
//

LPVirtualScreen::LPVirtualScreen() :
  m_position(LPPointZero), m_screenIndex(0), m_activeBounds(LPRectZero), m_detectedBounds(LPRectZero), m_useDefaultScreen(false)
{
#if __APPLE__
  CGDisplayRegisterReconfigurationCallback(ConfigurationChangeCallback, this);
#elif _WIN32
  makeDummyWindow();
#endif
  Update();
}

LPVirtualScreen::~LPVirtualScreen()
{
#if __APPLE__
  CGDisplayRemoveReconfigurationCallback(ConfigurationChangeCallback, this);
#elif _WIN32
  if (m_hWnd) {
    DestroyWindow(m_hWnd);
  }
#endif
}

#if __APPLE__
// Called when the the display configuration changes
void LPVirtualScreen::ConfigurationChangeCallback(CGDirectDisplayID display,
                                                  CGDisplayChangeSummaryFlags flags,
                                                  void *that)
{
  if (that) {
    static_cast<LPVirtualScreen*>(that)->Update();
  }
}
#endif

LPPoint LPVirtualScreen::Normalize(const LPPoint& position, bool activeOnly) const
{
  const LPPoint& origin = activeOnly ? m_activeBounds.origin : m_detectedBounds.origin;
  const LPSize& size = activeOnly ? m_activeBounds.size : m_detectedBounds.size;

  if (size.width > 0 && size.height > 0) {
    return LPPointMake((position.x - origin.x)/size.width, (position.y - origin.y)/size.height);
  }
  return LPPointZero;
}

LPPoint LPVirtualScreen::Denormalize(const LPPoint& position, bool activeOnly) const
{
  const LPPoint& origin = activeOnly ? m_activeBounds.origin : m_detectedBounds.origin;
  const LPSize& size = activeOnly ? m_activeBounds.size : m_detectedBounds.size;

  return LPPointMake(position.x*size.width + origin.x, position.y*size.height + origin.y);
}

LPPoint LPVirtualScreen::SetPosition(const LPPoint& position, uint32_t* screenIndex)
{
  m_position = ClipPosition(position, &m_screenIndex);
  if (screenIndex) {
    *screenIndex = m_screenIndex;
  }
  return m_position;
}

LPPoint LPVirtualScreen::ClipPosition(const LPPoint& position, uint32_t* screenIndex) const
{
  uint32_t numDisplays = static_cast<uint32_t>(m_activeScreens.size());
  LPPoint clippedPosition(position);
  LPFloat best_distance_squared;
  uint32_t index = 0;

  for (uint32_t i = 0; i < numDisplays; i++) {
    const LPRect& rect = m_activeScreens[i].Bounds();
    const LPFloat minX = LPRectGetMinX(rect);
    const LPFloat minY = LPRectGetMinY(rect);
    const LPFloat maxX = LPRectGetMaxX(rect);
    const LPFloat maxY = LPRectGetMaxY(rect);
    LPFloat x = position.x;
    LPFloat y = position.y;

    if (x <= minX) {
      x = minX;
    } else if (x >= maxX) {
      x = maxX - 1;
    }
    if (y <= minY) {
      y = minY;
    } else if (y >= maxY) {
      y = maxY - 1;
    }
    LPFloat dx = position.x - x;
    LPFloat dy = position.y - y;

    if (i == 0) {
      best_distance_squared = dx*dx + dy*dy;
      clippedPosition = LPPointMake(x, y);
    } else {
      LPFloat distance_squared = dx*dx + dy*dy;

      if (distance_squared < best_distance_squared) {
        clippedPosition = LPPointMake(x, y);
        best_distance_squared = distance_squared;
        index = i;
      }
    }
  }
  if (screenIndex) {
    *screenIndex = index;
  }
  return clippedPosition;
}

LPScreen LPVirtualScreen::ClosestScreen(const LPPoint& position) const
{
  uint32_t numDisplays = static_cast<uint32_t>(m_activeScreens.size());

  if (numDisplays > 1) {
    for (uint32_t i = 0; i < numDisplays; i++) {
      if (LPRectContainsPoint(m_activeScreens[i].Bounds(), position)) {
        return m_activeScreens[i];
      }
    }
    uint32_t index = 0;
    ClipPosition(position, &index);
    return m_activeScreens[index];
  }
  return m_activeScreens[0]; // We better have a screen if this is called -- FIXME
}

LPFloat LPVirtualScreen::AspectRatio() const
{
  if (m_activeBounds.size.height < 1) {
    return static_cast<LPFloat>(1);
  }
  return m_activeBounds.size.width/m_activeBounds.size.height;
}

void LPVirtualScreen::Update()
{
  std::vector<LPScreen> screens;
  uint32_t numDisplays = 0;

#if __APPLE__
  if (CGGetActiveDisplayList(0, 0, &numDisplays) == kCGErrorSuccess && numDisplays > 0) {
    CGDirectDisplayID *screenIDs = new CGDirectDisplayID[numDisplays];

    if (screenIDs) {
      if (CGGetActiveDisplayList(numDisplays, screenIDs, &numDisplays) == kCGErrorSuccess) {
        for (int i = 0; i < numDisplays; i++) {
          screens.push_back(LPScreen(screenIDs[i], i));
        }
      }
      delete [] screenIDs;
    }
  }
  if (screens.empty()) {
    screens.push_back(LPScreen(CGMainDisplayID(), 0));
    numDisplays = 1;
  }
#elif _WIN32
  EnumDisplayMonitors(0, 0, EnumerateDisplays, reinterpret_cast<LPARAM>(&screens));
  numDisplays = static_cast<uint32_t>(screens.size());
#else
  // Linux -- FIXME
#endif
  m_detectedScreens = screens;
  if (m_useDefaultScreen && numDisplays > 1) {
    m_activeScreens.clear();
    if (!m_detectedScreens.empty()) {
      for (size_t i = 0; i < m_detectedScreens.size(); i++) {
        if (m_detectedScreens[i].IsPrimary()) {
          m_activeScreens.push_back(m_detectedScreens[i]);
          break;
        }
      }
      if (m_activeScreens.empty()) {
        // This shouldn't happen, as there should always be a primary screen
        m_activeScreens.push_back(m_detectedScreens[0]);
      }
    }
  } else {
    m_activeScreens = screens;
  }
  m_activeBounds = ComputeBounds(m_activeScreens);
  m_detectedBounds = ComputeBounds(m_detectedScreens);
}

LPRect LPVirtualScreen::ComputeBounds(const std::vector<LPScreen>& screens)
{
  size_t numScreens = screens.size();

  if (numScreens == 1) {
    return screens[0].Bounds();
  } else if (numScreens > 1) {
    LPRect bounds = screens[0].Bounds();

    for (size_t i = 1; i < numScreens; i++) {
      bounds = LPRectUnion(bounds, screens[i].Bounds());
    }
    return bounds;
  } else {
    return LPRectZero;
  }
}

#if _WIN32
LPDummyWindowClass::LPDummyWindowClass()
{
  m_wndClass.style = CS_NOCLOSE | CS_SAVEBITS;
  m_wndClass.lpfnWndProc = WindowProc;
  m_wndClass.cbWndExtra = sizeof(LPIcon*);
  m_wndClass.cbClsExtra = 0;
  m_wndClass.hInstance = nullptr;
  m_wndClass.hIcon = nullptr;
  m_wndClass.hCursor = nullptr;
  m_wndClass.hbrBackground = nullptr;
  m_wndClass.lpszMenuName = nullptr;
  m_wndClass.lpszClassName = L"LPDummyWindow";
  m_atom = RegisterClassW(&m_wndClass);
}

LPDummyWindowClass::~LPDummyWindowClass()
{
  UnregisterClassW(L"LPDummyWindow", nullptr);
}

LRESULT CALLBACK LPDummyWindowClass::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  LONG_PTR val = GetWindowLongPtr(hwnd, GWLP_USERDATA);
  return val ? ((LPVirtualScreen*)val)->WindowProc(uMsg, wParam, lParam) : DefWindowProc(hwnd, uMsg, wParam, lParam);
}

BOOL CALLBACK LPVirtualScreen::EnumerateDisplays(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData)
{
  std::vector<LPScreen>& screens = *reinterpret_cast<std::vector<LPScreen>*>(dwData);
  screens.push_back(LPScreen(hMonitor, static_cast<uint32_t>(screens.size())));
  return true;
}

void LPVirtualScreen::makeDummyWindow()
{
  m_hWnd = CreateWindowExW(
    WS_EX_LAYERED |
    WS_EX_TOPMOST |
    WS_EX_NOACTIVATE |
    WS_EX_TRANSPARENT,
    MAKEINTRESOURCEW(LPDummyWindowClass::GetAtom()),
    L"",
    WS_POPUP |
    WS_VISIBLE,
    0, 0, 0, 0,
    nullptr,
    nullptr,
    nullptr,
    this
  );
  SetWindowLongPtr(m_hWnd, GWLP_USERDATA, (LONG_PTR)this);
  ShowWindow(m_hWnd, SW_HIDE);
}

LRESULT LPVirtualScreen::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch(uMsg) {
    case WM_DISPLAYCHANGE: Update(); break;
    default: break;
  }
  return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
}
#endif

void LPVirtualScreen::UseDefaultScreen(bool useDefaultScreen)
{
  if (m_useDefaultScreen != useDefaultScreen) {
    if (useDefaultScreen) {
      if (m_activeScreens.size() > 1) {
        m_activeScreens.clear();
        if (!m_detectedScreens.empty()) {
          for (size_t i = 0; i < m_detectedScreens.size(); i++) {
            if (m_detectedScreens[i].IsPrimary()) {
              m_activeScreens.push_back(m_detectedScreens[i]);
              break;
            }
          }
          if (m_activeScreens.empty()) {
            // This shouldn't happen, as there should always be a primary screen
            m_activeScreens.push_back(m_detectedScreens[0]);
          }
        }
      }
    } else if (m_detectedScreens.size() > m_activeScreens.size()) {
      m_activeScreens = m_detectedScreens;
    }
    m_useDefaultScreen = useDefaultScreen;
    m_activeBounds = ComputeBounds(m_activeScreens);
    m_detectedBounds = ComputeBounds(m_detectedScreens);
  }
}

bool LPVirtualScreen::UsingDefaultScreen() const {
  return m_useDefaultScreen;
}
