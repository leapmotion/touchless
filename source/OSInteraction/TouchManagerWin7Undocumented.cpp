#include "stdafx.h"
#include "TouchManagerWin7Undocumented.h"

typedef LONG NTSTATUS;

typedef NTSTATUS (__stdcall *t_ZwUserSendTouchInput)(HWND hWnd, int rdx, TOUCHINPUT* touchData, DWORD touchDataSize);
static auto ZwUserSendTouchInput = (t_ZwUserSendTouchInput)GetProcAddress(GetHmodUser32(), LPCSTR(1500));

bool TouchManagerWin7Undocumented::s_supported = !!ZwUserSendTouchInput;

static void TranslateAndSend(HWND hwnd, const Touch& touch, DWORD dwFlags) {
  ULONG flags = 0;
  if(IsTouchWindow(hwnd, &flags)) {
    // Compose the touch event on the target window:
    const LONG x = static_cast<LONG>(100 * touch.x());
    const LONG y = static_cast<LONG>(100 * touch.y());

    TOUCHINPUT contact;
    contact.x = x;
    contact.y = y;
    contact.hSource = 0;
    contact.dwID = touch.id();
    contact.dwFlags = 0;
    contact.dwMask = TOUCHINPUTMASKF_CONTACTAREA;
    contact.dwTime = 0;
    contact.dwExtraInfo = 0;
    contact.cxContact = 4;
    contact.cyContact = 4;
    contact.dwFlags = dwFlags;

    ZwUserSendTouchInput(hwnd, 1, &contact, sizeof(contact));
  }

  // Mouse input stuff:
  DWORD style = GetWindowLong(hwnd, GWL_EXSTYLE);

  // Do not attempt to descend transparent windows:
  if (!(style & WS_EX_TRANSPARENT)) {
    // Convert to client coordinates for this window:
    POINT pt = {(LONG)touch.x(), (LONG)touch.y()};
    ScreenToClient(hwnd, &pt);

    // Identify a child window at this point, if possible:
    HWND hwndChild = ChildWindowFromPointEx(hwnd, pt, CWP_SKIPINVISIBLE);
    if (hwndChild) {
      hwnd = hwndChild;
    }

    // TODO: Send a mouse click:
  }
}

TouchManagerWin7Undocumented::TouchManagerWin7Undocumented(void):
  m_capture(nullptr),
  m_captureIsTouch(false)
{
}

void TouchManagerWin7Undocumented::AddTouch(const Touch& touch) {
  DWORD dwFlags;

  // Determine whether capture must be evaluated:
  if(!m_capture) {
    POINT pt = {(LONG)touch.x(), (LONG)touch.y()};

    // Try to find a window located at this coordinate:
    m_capture = WindowFromPhysicalPoint(pt);

    wchar_t str[250];
    GetWindowTextW(m_capture, str, 250);
    OutputDebugStringW(str);
    OutputDebugStringA("\n");

    // We set the capture, and this touch point made the capture current, so this
    // touch point becomes the primary touch point
    m_primaryTouchID = touch.id();
    dwFlags = TOUCHEVENTF_PRIMARY;
  }
  else
    dwFlags = 0;

  dwFlags |=
    TOUCHEVENTF_INRANGE |
    (
      touch.touching() ?
      TOUCHEVENTF_DOWN :
      0
    );

  // Compose and post the touch event:
  TranslateAndSend(m_capture, touch, dwFlags);
}

void TouchManagerWin7Undocumented::UpdateTouch(const Touch& oldTouch, const Touch& touch) {
  // Trivial return optimization
  if(!m_capture)
    return;

  TranslateAndSend(
    m_capture,
    touch,
    TOUCHEVENTF_INRANGE |
    (
      touch.touching() ?
      (
        // Detect transition from down-to-up
        oldTouch.touching() ?
        TOUCHEVENTF_DOWN :
        TOUCHEVENTF_MOVE
      ) :
      (
        // Detect transition from up-to-down
        oldTouch.touching() ?
        TOUCHEVENTF_MOVE :
        TOUCHEVENTF_UP
      )
    )
  );
}

void TouchManagerWin7Undocumented::RemoveTouch(const Touch& touch) {
  TranslateAndSend(
    m_capture,
    touch,
    TOUCHEVENTF_UP
  );
}

void TouchManagerWin7Undocumented::OnRemoveAllTouches(void) {
  m_capture = nullptr;
}