#if defined(_WIN32) && !defined(_TOUCH_MANAGER_WIN7_DRIVER)
#define _TOUCH_MANAGER_WIN7_DRIVER
#include "TouchManager.h"
#include "OcuHidInstance.h"

class TouchManagerWin7Driver:
  public TouchManager
{
public:
  /// <summary>
  /// Static member, set to true if windows 8 touch is supported
  /// </summary>
  static const bool s_supported;

  TouchManagerWin7Driver(LPVirtualScreen* virtualScreen);
  ~TouchManagerWin7Driver(void);

  static bool formTouchInput(const Touch& touch, TOUCHINPUT* ti);

  virtual int Version() { return 7; }

private:
  COcuHidInstance* m_hidInstance;

  Touch NormalizeTouch(const Touch& touch) const;

  // Overrides from TouchManager:
  void AddTouch(const Touch& touch) override;
  void UpdateTouch(const Touch& oldTouch, const Touch& newTouch) override;
  void RemoveTouch(const Touch& touch) override;
};
#endif
