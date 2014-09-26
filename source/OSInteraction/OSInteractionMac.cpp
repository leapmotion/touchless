#include "OSInteraction/OSInteractionMac.h"
#include "Utility/LPVirtualScreen.h"
#include EXCEPTION_PTR_HEADER
#include <fstream>


namespace Touchless
{
  OSInteractionDriver* OSInteractionDriver::New(LPVirtualScreen &virtualScreen)
  {
    return new OSInteractionDriverMac(virtualScreen);
  }


OSInteractionDriverMac::OSInteractionDriverMac(LPVirtualScreen &virtualScreen)
  : OSInteractionDriver(virtualScreen)
{ }

OSInteractionDriverMac::~OSInteractionDriverMac() {}

bool OSInteractionDriverMac::initializeTouch()
{
  return true;
}

void OSInteractionDriverMac::clickDown(int button, int number)
{
  int clickNum = (number < 1) ? 1 : ((number > 3) ? 3 : number);
  const CGPoint& position = m_virtualScreen.Position();
  CGEventRef eventRef = CGEventCreateMouseEvent(0, kCGEventLeftMouseDown, position, kCGMouseButtonLeft);
  CGEventSetIntegerValueField(eventRef, kCGMouseEventClickState, clickNum);
  CGEventPost(kCGHIDEventTap, eventRef);
  CFRelease(eventRef);
  m_buttonDown = true;
  OSInteractionDriver::clickDown(button, number);
}

void OSInteractionDriverMac::clickUp(int button, int number)
{
  OSInteractionDriver::clickUp(button, number);
  int clickNum = (number < 1) ? 1 : ((number > 3) ? 3 : number);
  const CGPoint& position = m_virtualScreen.Position();
  CGEventRef eventRef = CGEventCreateMouseEvent(0, kCGEventLeftMouseUp, position, kCGMouseButtonLeft);
  CGEventSetIntegerValueField(eventRef, kCGMouseEventClickState, clickNum);
  CGEventPost(kCGHIDEventTap, eventRef);
  CFRelease(eventRef);
  m_buttonDown = false;
}

bool OSInteractionDriverMac::cursorPosition(float* fx, float* fy) const
{
  CGEventRef event = CGEventCreate(0);
  CGPoint cursor = CGEventGetLocation(event);
  if (fx) {
    *fx = static_cast<float>(cursor.x);
  }
  if (fy) {
    *fy = static_cast<float>(cursor.y);
  }
  CFRelease(event);
  return true;
}

void OSInteractionDriverMac::setCursorPosition(float fx, float fy, bool absolute)
{
  LPPoint position = LPPointMake(static_cast<LPFloat>(fx), static_cast<LPFloat>(fy));

  CGEventRef eventRef;
  if (!absolute) {
    eventRef = CGEventCreate(0);
    CGPoint cursor = CGEventGetLocation(eventRef);
    position.x += cursor.x;
    position.y += cursor.y;
    CFRelease(eventRef);
  }
  position = m_virtualScreen.SetPosition(position);
  eventRef = CGEventCreateMouseEvent(0, m_buttonDown ? kCGEventLeftMouseDragged : kCGEventMouseMoved,
                                     position, kCGMouseButtonLeft);
  CGEventPost(kCGHIDEventTap, eventRef);
  CFRelease(eventRef);
}

void OSInteractionDriverMac::cancelGestureEvents()
{
  m_movingCursor = false;

  endGesture();
}

void OSInteractionDriverMac::applyCharms(const Vector&, int, int&)
{ }

bool OSInteractionDriverMac::checkTouching(const Vector&, float) const
{
  return false;
}

void OSInteractionDriverMac::emitTouchEvent(const TouchEvent&)
{ }

bool OSInteractionDriverMac::touchAvailable() const
{
  return false;
}

int OSInteractionDriverMac::touchVersion() const
{
  return 1;
}

int OSInteractionDriverMac::numTouchScreens() const
{
  return 0;
}

bool OSInteractionDriverMac::useCharmHelper() const
{
  return false;
}

void OSInteractionDriverMac::emitKeyboardEvent(int key, bool down)
{
  CGEventRef eventRef = CGEventCreateKeyboardEvent(0, LPKeyboard::GetKeyCode(key), down);
  CGEventPost(kCGHIDEventTap, eventRef);
  CFRelease(eventRef);
}

void OSInteractionDriverMac::emitKeyboardEvents(int* keys, int numKeys, bool down)
{
  if (numKeys <= 0) {
    return;
  }
}


void OSInteractionDriverMac::syncPosition()
{
  LPPoint position;
  CGEventRef eventRef = CGEventCreate(0);
  position = CGEventGetLocation(eventRef);
  CFRelease(eventRef);

  position = m_virtualScreen.SetPosition(position);
  m_gesture.setPosition(position.x, position.y);
}

}
