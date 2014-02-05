#pragma once
#include "Overlay/LPIcon.h"

class LPIconWin:
  public LPIcon
{
public:
  LPIconWin(void);
  ~LPIconWin(void);

  /// <summary>
  /// Native window procedure
  /// </summary>
  LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
  HWND m_hWnd;

  friend class LPIconWindowClass;

public:
  // Base overrides:
  void SetPosition(const LPPoint& position) override;
  void SetVisibility(bool isVisible) override;
  bool Update(void) override;
  LPPoint GetPosition() const override;
  bool GetVisibility(void) const override;
};

