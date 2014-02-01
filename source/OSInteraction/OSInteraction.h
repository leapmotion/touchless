#if !defined(__OSInteraction_h__)
#define __OSInteraction_h__

#include "common.h"

#include "Leap.h"

#if __APPLE__
#include "Peripherals/LPMac.h"
#include "Peripherals/LPOverlay.h"
#elif !defined _WIN32
#include "Peripherals/LPLinux.h"
#endif
#include "Peripherals/LPVirtualScreen.h"
#include "Touch.h"
#include "TouchManager.h"

#include "Peripherals/LPGesture.h"
#include <boost/thread/mutex.hpp>

class LPIcon;
class LPImage;

namespace Touchless {
using Leap::Frame;
using Leap::Vector;
using Leap::TouchEvent;

class OSInteractionDriver
{
public:
  OSInteractionDriver(LPVirtualScreen &virtualScreen);
  ~OSInteractionDriver();

  static OSInteractionDriver* New(LPVirtualScreen &virtualScreen);

  void destroy();

  bool initializeTouch();

  void useDefaultScreen(bool use);

  void clickDown(int button, int number = 1);
  void clickUp(int button, int number = 1);
  bool isClickedDown(int button) const;
  void keyDown(int code);
  void keyUp(int code);

  bool cursorPosition(float* fx, float* fy) const;
  void setCursorPosition(float fx, float fy, bool absolute = true);

  bool emitGestureEvents(const Frame& frame, const Frame& sinceFrame);
  void cancelGestureEvents();

  uint32_t gestureType() const;
  bool beginGesture(uint32_t gestureType);
  bool endGesture();
  bool applyZoom(float zoom);
  bool applyRotation(float rotation);
  bool applyScroll(float dx, float dy, int64_t timeDiff = 0);
  bool applyDesktopSwipe(float dx, float dy);
  void applyCharms(const Leap::Vector& aspectNormalized, int numPointablesActive, int& charmsMode);

  bool useCharmHelper() const;

  bool checkTouching(const Vector& position, float noTouchBorder) const;
  void emitTouchEvent(const TouchEvent& evt);

  bool touchAvailable() const;

  int numTouchScreens() const;

  void emitKeyboardEvent(int key, bool down);
  void emitKeyboardEvents(int* keys, int numKeys, bool down);
  void syncPosition();

  enum { NUM_BUTTONS = 8 };

  LPVirtualScreen &m_virtualScreen;
  LPGesture        m_gesture;
  TouchManager    *m_touchManager;
  bool             m_movingCursor;
  boost::mutex     m_touchMutex;
  bool             m_buttonDown;
  bool             m_clickedButtons[NUM_BUTTONS];
  bool             m_useCharmHelper;


};

}

#endif // __OSInteraction_h__
