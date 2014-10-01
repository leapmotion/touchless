#include "stdafx.h"
#include "Overlay/LPIconWin.h"
#include "Overlay/LPIconWindowClass.h"
#include "Overlay/LPImage.h"
#include "Overlay/LPImageWin.h"

LPIconWin::LPIconWin(void) {
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
}

LPIconWin::~LPIconWin(void) {
  if(m_hWnd)
    // Delete the window used to render this icon
    DestroyWindow(m_hWnd);
}

LPIcon* LPIcon::New(void) {
  return new LPIconWin;
}

LRESULT LPIconWin::WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam)
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

void LPIconWin::SetPosition(const LPPoint& position) {
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
}

void LPIconWin::SetVisibility(bool isVisible) {
  ShowWindow(m_hWnd, isVisible ? SW_SHOW : SW_HIDE);
}

bool LPIconWin::Update(void) {
  // If the icon is currently null, abort.
  if (!m_image)
    return false;

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

  auto pWinIcon = static_cast<LPImageWin*>(m_image.get());
  BOOL rs = UpdateLayeredWindow(
    m_hWnd,          // The window to be updated.
    hDC,          // The DC for the screen where the window will be rendered
    nullptr,        // Default destination point
    &wndSize,        // Size is our own window size--won't default correctly
    pWinIcon->GetDC(),    // Source DC is the image's source DC
    &ptSrc,          // Source point is the origin
    0,            // No color key
    &blend,          // Blend function specified above
    ULW_ALPHA        // Alpha channel is in the image
  );

  // Done with our DC
  ReleaseDC(nullptr, hDC);

  // Standard return value switch
  return rs != FALSE;
}

bool LPIconWin::GetVisibility() const {
  return !!IsWindowVisible(m_hWnd);
}

LPPoint LPIconWin::GetPosition(void) const {
  WINDOWPLACEMENT wndpl;
  GetWindowPlacement(m_hWnd, &wndpl);
  wndpl.rcNormalPosition;

  LPPoint retVal(
    (LPFloat)wndpl.rcNormalPosition.left,
    (LPFloat)wndpl.rcNormalPosition.top
  );
  if (m_image) {
    const POINT& imageHotspot = m_image->GetHotspot();
    retVal.x -= imageHotspot.x;
    retVal.y -= imageHotspot.y;
  }
  return retVal;
}
