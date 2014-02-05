#pragma once

#include "common.h"

#include "Leap.h"

#include "OSInteraction/OSInteraction.h"

#include "Utility/LPVirtualScreen.h"

#include "OSInteraction/LPGesture.h"

class LPImage;

namespace Touchless {
using Leap::Frame;
using Leap::Vector;

class OSInteractionDriverLinux : public OSInteractionDriver
{
public:
  OSInteractionDriverLinux(LPVirtualScreen &virtualScreen);
  ~OSInteractionDriverLinux();

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
  int numTouchScreens() const;
  void emitKeyboardEvent(int key, bool down);
  void emitKeyboardEvents(int* keys, int numKeys, bool down);
  void syncPosition();
};

}

