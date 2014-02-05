#pragma once

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