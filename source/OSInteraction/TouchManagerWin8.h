#pragma once
#include "TouchManager.h"

class TouchManagerWin8:
  public TouchManager
{
public:
  /// <summary>
  /// Static member, set to true if windows 8 touch is supported
  /// </summary>
  static const bool s_supported;

  TouchManagerWin8(void);

public:
  // Overrides from TouchManager:
  void AddTouch(const Touch& touch) override;
  void UpdateTouch(const Touch& oldTouch, const Touch& newTouch) override;
  void RemoveTouch(const Touch& oldTouch) override;
};

