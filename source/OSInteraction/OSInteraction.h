#if !defined(__OSInteraction_h__)
#define __OSInteraction_h__

#include "common.h"

#include "Leap.h"

#include "Utility/LPVirtualScreen.h"
#include "OSInteraction/LPGesture.h"

class LPImage;

namespace Touchless {
class TouchEvent;
using Leap::Frame;
using Leap::Vector;

class OSInteractionDriver
{
public:
  OSInteractionDriver(LPVirtualScreen* virtualScreen);
  virtual ~OSInteractionDriver();

  virtual bool initializeTouch() = 0;
  virtual void clickDown(int button, int number = 1) = 0;
  virtual void clickUp(int button, int number = 1) = 0;
  virtual bool cursorPosition(float* fx, float* fy) const = 0;
  virtual void setCursorPosition(float fx, float fy, bool absolute = true) = 0;
  virtual void cancelGestureEvents() = 0;
  virtual void applyCharms(const Leap::Vector& aspectNormalized, int numPointablesActive, int& charmsMode) = 0;
  virtual bool useCharmHelper() const = 0;
  virtual bool checkTouching(const Vector& position, float noTouchBorder) const = 0;
  virtual void emitTouchEvent(const TouchEvent& evt) = 0;
  virtual bool touchAvailable() const = 0;
  virtual int touchVersion() const = 0;
  virtual int numTouchScreens() const = 0;
  virtual void emitKeyboardEvent(int key, bool down) = 0;
  virtual void emitKeyboardEvents(int* keys, int numKeys, bool down) = 0;
  virtual void syncPosition() = 0;

  static OSInteractionDriver* New(LPVirtualScreen* virtualScreen);

  void useDefaultScreen(bool use);

  bool isClickedDown(int button) const;
  void keyDown(int code);
  void keyUp(int code);

  uint32_t gestureType() const;
  bool beginGesture(uint32_t gestureType);
  bool endGesture();
  bool applyZoom(float zoom);
  bool applyRotation(float rotation);
  bool applyScroll(float dx, float dy, int64_t timeDiff = 0);
  bool applyDesktopSwipe(float dx, float dy);

protected:
  enum { NUM_BUTTONS = 8 };

  LPVirtualScreen* m_virtualScreen;
  LPGesture        m_gesture;
  bool             m_movingCursor;
  bool             m_buttonDown;
  bool             m_clickedButtons[NUM_BUTTONS];


};

}

#endif // __OSInteraction_h__
