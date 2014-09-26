#pragma once
#include "TouchManager.h"

class TouchManagerWin7Undocumented:
  public TouchManager
{
public:
  /// <summary>
  /// Static member, set to true if windows 8 touch is supported
  /// </summary>
  static bool s_supported;

  TouchManagerWin7Undocumented(void);

private:
  /// <summary>
  /// Indicates the window currently receiving capture, if one exists
  /// </summary>
  HWND m_capture;

  /// <summary>
  /// Optimization flag--set if the capture window is a touch window
  /// </summary>
  bool m_captureIsTouch;

  /// <summary>
  /// Primary touch ID
  /// </summary>
  size_t m_primaryTouchID;

public:
  // Overrides from TouchManager:
  int Version() override { return 7; }
  void AddTouch(const Touch& touch) override;
  void UpdateTouch(const Touch& oldTouch, const Touch& newTouch) override;
  void RemoveTouch(const Touch& touch) override;
  void OnRemoveAllTouches(void) override;

  /// <remarks>
  /// We have to override the default implementation, because this version cannot output to more than one monitor
  /// </remarks>
  size_t numTouchScreens(void) const override {return 1;}
};

