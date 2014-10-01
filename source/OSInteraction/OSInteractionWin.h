#if !defined(__OSInteractionWin_h__)
#define __OSInteractionWin_h__

#include "common.h"

#include "Leap.h"

#include "OSInteraction/OSInteraction.h"

#include "Touch.h"
#include "TouchManager.h"

#include "OSInteraction/LPGesture.h"
#include <boost/thread/mutex.hpp>

class LPImage;

namespace Touchless {
using Leap::Frame;
using Leap::Vector;

class OSInteractionDriverWin : public OSInteractionDriver
{
public:
  OSInteractionDriverWin(LPVirtualScreen* virtualScreen);
  ~OSInteractionDriverWin();

  bool initializeTouch();
  void clickDown(int button, int number = 1);
  void clickUp(int button, int number = 1);
  bool cursorPosition(float* fx, float* fy) const;
  void setCursorPosition(float fx, float fy, bool absolute = true);
  void cancelGestureEvents();
  void applyCharms(const Leap::Vector& aspectNormalized, int numPointablesActive, int& charmsMode);
  bool useCharmHelper() const;
  bool checkTouching(const Vector& position, float noTouchBorder) const;
  void emitTouchEvent(const TouchEvent& evt);
  bool touchAvailable() const;
  int touchVersion() const;
  int numTouchScreens() const;
  void emitKeyboardEvent(int key, bool down);
  void emitKeyboardEvents(int* keys, int numKeys, bool down);
  void syncPosition();

protected:
  TouchManager    *m_touchManager;
  boost::mutex     m_touchMutex;
  bool             m_useCharmHelper;
};

}

#endif // __OSInteractionWin_h__
