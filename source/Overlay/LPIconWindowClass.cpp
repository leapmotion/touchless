#include "stdafx.h"
#include "Overlay/LPIconWindowClass.h"
#include "Overlay/LPIconWin.h"

LPIconWindowClass::LPIconWindowClass() {
  // Set up the class structure.
  m_wndClass.style =
    CS_NOCLOSE |    // No close button
    CS_SAVEBITS;    // Required to enable click-through transparency

  // Window procedure is defined at the end of this file
  m_wndClass.lpfnWndProc = WindowProc;

  // We need enough space to store a pointer to the LPIcon that will drive the window
  m_wndClass.cbWndExtra = sizeof(LPIconWin*);

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

LPIconWindowClass::~LPIconWindowClass() {
  // Clean up the window class.
  UnregisterClassW(L"LPOverlay", nullptr);
}

LRESULT CALLBACK LPIconWindowClass::WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
  // Get a pointer to the LPIcon instance.
  LONG_PTR val = GetWindowLongPtr(hwnd, GWLP_USERDATA);

  // If the pointer is non-null, control is passed to the icon itself.
  // Otherwise, we allow the default window procedure to handle this message.
  return
    val ?
    ((LPIconWin*)val)->WindowProc(uMsg, wParam, lParam) :
    DefWindowProc(hwnd, uMsg, wParam, lParam);
}