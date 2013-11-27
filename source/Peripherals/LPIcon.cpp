/*==================================================================================================================

    Copyright (c) 2010 - 2013 Leap Motion. All rights reserved.

  The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
  protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
  strictly forbidden unless prior written permission is obtained from Leap Motion.

===================================================================================================================*/
#include "stdafx.h"
#include "LPIcon.h"
#include "LPImage.h"

#if _WIN32
/// <summary>
/// This is a utility class that registers the LPIcon window class.  Do not use it directly.
/// </summary>
class LPIconWindowClass
{
  public:
  // The window class procedure:
  static LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

  /// <returns>
  /// The class atom used to create windows of this class
  /// </returns>
  static ATOM GetAtom() {
    static LPIconWindowClass s_instance;
    return s_instance.m_atom;
  }

  private:
  LPIconWindowClass();
  ~LPIconWindowClass();

  // The actual class definition:
  WNDCLASSW m_wndClass;

  // The window class atom:
  ATOM m_atom;
};

LPIconWindowClass::LPIconWindowClass()
{
  // Set up the class structure.

  m_wndClass.style =
    CS_NOCLOSE |    // No close button
    CS_SAVEBITS;    // Required to enable click-through transparency

  // Window procedure is defined at the end of this file
  m_wndClass.lpfnWndProc = WindowProc;

  // We need enough space to store a pointer to the LPIcon that will drive the window
  m_wndClass.cbWndExtra = sizeof(LPIcon*);

  // No class-specific data required
  m_wndClass.cbClsExtra = 0;

  // The remainder of these are alld efault values
  m_wndClass.hInstance = nullptr;
  m_wndClass.hIcon = nullptr;
  m_wndClass.hCursor = nullptr;
  m_wndClass.hbrBackground = nullptr;
  m_wndClass.lpszMenuName = nullptr;

  // The name of the class will be locally defined as LPOverlay
  m_wndClass.lpszClassName = L"LPOverlay";

  // Finally, conduct the registration.
  m_atom = RegisterClassW(&m_wndClass);
}

LPIconWindowClass::~LPIconWindowClass()
{
  // Clean up the window class.
  UnregisterClassW(L"LPOverlay", nullptr);
}

LRESULT CALLBACK LPIconWindowClass::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  // Get a pointer to the LPIcon instance.
  LONG_PTR val = GetWindowLongPtr(hwnd, GWLP_USERDATA);

  // If the pointer is non-null, control is passed to the icon itself.
  // Otherwise, we allow the default window procedure to handle this message.
  return
    val ?
    ((LPIcon*)val)->WindowProc(uMsg, wParam, lParam) :
    DefWindowProc(hwnd, uMsg, wParam, lParam);
}
#endif

LPIcon::LPIcon() : m_position(LPPointZero), m_isVisible(false)
{
#if _WIN32
  // Create the window with the class registered as part of application
  // initialization.  This window will be used to draw overlay icons.
  m_hWnd = CreateWindowExW(
    WS_EX_TOOLWINDOW | // Don't draw this window in the taskbar or alt-tab menu
    WS_EX_LAYERED |  // We'd like to specify a permanent DC with this window that we can use with transparency effects
    WS_EX_TOPMOST |  // The overlay window should, of course, be always-on-top.
    WS_EX_NOACTIVATE | // The window should never receive focus
    WS_EX_TRANSPARENT, // Transparency ensures that mouse clicks go through the window.
    MAKEINTRESOURCEW(LPIconWindowClass::GetAtom()),
    L"",             // No window title
    WS_POPUP |       // The window will not have a title bar, either.
    WS_VISIBLE,      // The window should be initially visible.
    0, 0, 0, 0,      // Initial size and position:  All zeroes.
    nullptr,         // No parent window
    nullptr,         // No menu required
    nullptr,         // Default hinstance
    this             // Creation parameter is this
  );

  // We want to receive messages from this window to our WindowProc.  Thus, we set our USERDATA to be
  // equal to the this pointer, so that the window class can route messages correctly.
  SetWindowLongPtr(m_hWnd, GWLP_USERDATA, (LONG_PTR)this);
#endif
}

LPIcon::~LPIcon()
{
  SetVisibility(false);
  // Unregister the parent image prior to destruction
  SetImage(std::shared_ptr<LPImage>(), false);
#if _WIN32
  if (m_hWnd) {
    // Delete the window used to render this icon
    DestroyWindow(m_hWnd);
  }
#endif
}

#if _WIN32
LRESULT LPIcon::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch(uMsg) {
    case WM_DISPLAYCHANGE:
      // Something changed with monitor resolutions.  The extent of the display needs to be
      // correspondingly changed.
// Tickle Virtual Screen -- FIXME
      break;
    case WM_NCHITTEST:
      // Hit testing.  In order to allow for proper pass-through to underlying windows, this
      // window must not report a hit on any area of its extent.  Thus, we always return the
      // same value here.
      return HTTRANSPARENT;
    default:
      break;
  }

  // Don't know what's going on, let the default window procedure handle things.
  return DefWindowProc(m_hWnd, uMsg, wParam, lParam);
}
#endif

void LPIcon::SetVisibility(bool isVisible)
{
  if (m_isVisible != isVisible) {
    m_isVisible = isVisible;
#if _WIN32
    // Simple forwarding call to Win32 to show or hide this window based on caller request.
    ShowWindow(m_hWnd, isVisible ? SW_SHOW : SW_HIDE);
#endif
  }
}

void LPIcon::SetPosition(const LPPoint& position)
{
  // Update our own coordinate.
  m_position = position;

#if _WIN32
  // Recover the hotspot of the base image, if an image is assigned.  Otherwise, the
  // hot spot will be all zeroes.
  POINT hotspot = { 0, 0 };

  if (m_image) {
    const POINT& imageHotspot = m_image->GetHotspot();
    hotspot.x = imageHotspot.x;
    hotspot.y = imageHotspot.y;
  }

  // The window position is the base offset plus the translation of the requested coordinate.
  SetWindowPos(
    m_hWnd,
    nullptr,
    (DWORD)(position.x - hotspot.x),
    (DWORD)(position.y - hotspot.y),
    0,
    0,
    SWP_NOSIZE // We don't want SetWindowPos to touch the size
  );
#endif
}

void LPIcon::SetImage(const std::shared_ptr<LPImage>& image, bool update)
{
  if (m_image != image) {
    m_image = image;
    if (update) {
      Update();
    }
  }
}

bool LPIcon::Update()
{
#if _WIN32
  // If the icon is currently null, abort.
  if (!m_image) {
    return false;
  }
  // Some utility structures, corresponding to the origin and image extents
  SIZE wndSize = {m_image->GetWidth(), m_image->GetHeight()};
  POINT ptSrc = {0, 0};

  // The blend function is used to describe how to blend the image we
  // assign with the background.  Our technique is AC_SRC_OVER, which means
  // that the alpha channel of the source image is used to set transparency,
  // and that the source image has a premultiplied alpha channel.
  BLENDFUNCTION blend;
  blend.BlendOp = AC_SRC_OVER;
  blend.BlendFlags = 0;
  blend.SourceConstantAlpha = ~0;
  blend.AlphaFormat = AC_SRC_ALPHA;

  // Need a DC for the screen
  HDC hDC = GetDC(nullptr);

  BOOL rs = UpdateLayeredWindow(
    m_hWnd,          // The window to be updated.
    hDC,          // The DC for the screen where the window will be rendered
    nullptr,        // Default destination point
    &wndSize,        // Size is our own window size--won't default correctly
    m_image->GetDC(),    // Source DC is the image's source DC
    &ptSrc,          // Source point is the origin
    0,            // No color key
    &blend,          // Blend function specified above
    ULW_ALPHA        // Alpha channel is in the image
  );

  // Done with our DC
  ReleaseDC(nullptr, hDC);

  // Standard return value switch
  return (rs != FALSE);
#elif __APPLE__
  return false; // LPIcon objects are rendeerd using LPOverlay
#else
  return false; // Linux -- FIXME
#endif
}
