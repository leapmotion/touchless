#include "OSInteraction/OSInteractionLinux.h"
#include "Utility/LPVirtualScreen.h"
#include EXCEPTION_PTR_HEADER
#include <fstream>


namespace Touchless
{

OSInteractionDriver* OSInteractionDriver::New(LPVirtualScreen &virtualScreen)
{
  return new OSInteractionDriverLinux(virtualScreen);
}


OSInteractionDriverLinux::OSInteractionDriverLinux(LPVirtualScreen &virtualScreen)
  : OSInteractionDriver(virtualScreen)
{ }

OSInteractionDriverLinux::~OSInteractionDriverLinux() {}

bool OSInteractionDriverLinux::initializeTouch()
{
  return true;
}

void OSInteractionDriverLinux::clickDown(int button, int number)
{
  // TODO: write real code
//   int clickNum = (number < 1) ? 1 : ((number > 3) ? 3 : number);
//   const CGPoint& position = m_virtualScreen.Position();
//   CGEventRef eventRef = CGEventCreateMouseEvent(0, kCGEventLeftMouseDown, position, kCGMouseButtonLeft);
//   CGEventSetIntegerValueField(eventRef, kCGMouseEventClickState, clickNum);
//   CGEventPost(kCGHIDEventTap, eventRef);
//   CFRelease(eventRef);
//   m_buttonDown = true;
//   OSInteractionDriver::clickDown(button, number);
}

void OSInteractionDriverLinux::clickUp(int button, int number)
{
  // TODO: write real code
//   OSInteractionDriver::clickUp(button, number);
//   int clickNum = (number < 1) ? 1 : ((number > 3) ? 3 : number);
//   const CGPoint& position = m_virtualScreen.Position();
//   CGEventRef eventRef = CGEventCreateMouseEvent(0, kCGEventLeftMouseUp, position, kCGMouseButtonLeft);
//   CGEventSetIntegerValueField(eventRef, kCGMouseEventClickState, clickNum);
//   CGEventPost(kCGHIDEventTap, eventRef);
//   CFRelease(eventRef);
//   m_buttonDown = false;
}

bool OSInteractionDriverLinux::cursorPosition(float* fx, float* fy) const
{
  // TODO: write real code
//   CGEventRef event = CGEventCreate(0);
//   CGPoint cursor = CGEventGetLocation(event);
//   if (fx) {
//     *fx = static_cast<float>(cursor.x);
//   }
//   if (fy) {
//     *fy = static_cast<float>(cursor.y);
//   }
//   CFRelease(event);
  return true;
}

void OSInteractionDriverLinux::setCursorPosition(float fx, float fy, bool absolute)
{
  // TODO: write real code
//   LPPoint position = LPPointMake(static_cast<LPFloat>(fx), static_cast<LPFloat>(fy));
//
//   CGEventRef eventRef;
//   if (!absolute) {
//     eventRef = CGEventCreate(0);
//     CGPoint cursor = CGEventGetLocation(eventRef);
//     position.x += cursor.x;
//     position.y += cursor.y;
//     CFRelease(eventRef);
//   }
//   position = m_virtualScreen.SetPosition(position);
//   eventRef = CGEventCreateMouseEvent(0, m_buttonDown ? kCGEventLeftMouseDragged : kCGEventMouseMoved,
//                                      position, kCGMouseButtonLeft);
//   CGEventPost(kCGHIDEventTap, eventRef);
//   CFRelease(eventRef);
}

void OSInteractionDriverLinux::cancelGestureEvents()
{
  // TODO: write real code
//   m_movingCursor = false;
//
  endGesture();
}

void OSInteractionDriverLinux::applyCharms(const Vector&, int, int&)
{ }

bool OSInteractionDriverLinux::checkTouching(const Vector&, float) const
{
  return false;
}

void OSInteractionDriverLinux::emitTouchEvent(const TouchEvent&)
{ }

bool OSInteractionDriverLinux::touchAvailable() const
{
  return false;
}

int OSInteractionDriverLinux::numTouchScreens() const
{
  return 0;
}

bool OSInteractionDriverLinux::useCharmHelper() const
{
  return false;
}

void OSInteractionDriverLinux::emitKeyboardEvent(int key, bool down)
{
  // TODO: write real code
//   CGEventRef eventRef = CGEventCreateKeyboardEvent(0, LPKeyboard::GetKeyCode(key), down);
//   CGEventPost(kCGHIDEventTap, eventRef);
//   CFRelease(eventRef);
}

void OSInteractionDriverLinux::emitKeyboardEvents(int* keys, int numKeys, bool down)
{
  // TODO: write real code
//   if (numKeys <= 0) {
//     return;
//   }
}


void OSInteractionDriverLinux::syncPosition()
{
  // TODO: write real code
//   LPPoint position;
//   CGEventRef eventRef = CGEventCreate(0);
//   position = CGEventGetLocation(eventRef);
//   CFRelease(eventRef);
//
//   position = m_virtualScreen.SetPosition(position);
//   m_gesture.setPosition(position.x, position.y);
}

}
