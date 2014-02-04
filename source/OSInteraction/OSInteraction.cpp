// #include "stdafx.h"
#include "OSInteraction.h"
#include "Utility/LPVirtualScreen.h"
#include EXCEPTION_PTR_HEADER
#include <fstream>


namespace Touchless
{

#ifdef _WIN32
void windowsKeyDown(INPUT& input, WORD key) {
  input.type = INPUT_KEYBOARD;
  input.ki.wVk = key;
  input.ki.dwFlags = 0;
}
void windowsKeyUp(INPUT& input, WORD key) {
  input.type = INPUT_KEYBOARD;
  input.ki.wVk = key;
  input.ki.dwFlags = KEYEVENTF_KEYUP;
}
void windowsKeyCombo(WORD key1) {
  INPUT input[2] = {0};
  windowsKeyDown(input[0], key1);
  windowsKeyUp(input[1], key1);
  SendInput(2,input,sizeof(INPUT));
}
void windowsKeyCombo(WORD key1, WORD key2) {
  INPUT input[4] = {0};
  windowsKeyDown(input[0], key1);
  windowsKeyDown(input[1], key2);
  windowsKeyUp(input[2], key2);
  windowsKeyUp(input[3], key1);
  SendInput(4,input,sizeof(INPUT));
}
void windowsKeyCombo(WORD key1, WORD key2, WORD key3) {
  INPUT input[6] = {0};
  windowsKeyDown(input[0], key1);
  windowsKeyDown(input[1], key2);
  windowsKeyDown(input[2], key3);
  windowsKeyUp(input[3], key3);
  windowsKeyUp(input[4], key2);
  windowsKeyUp(input[5], key1);
  SendInput(6,input,sizeof(INPUT));
}
#endif


OSInteractionDriver::OSInteractionDriver(LPVirtualScreen &virtualScreen)
  : m_virtualScreen(virtualScreen),
  m_movingCursor(false),
  m_useCharmHelper(true)
{}

OSInteractionDriver::~OSInteractionDriver() {}

OSInteractionDriver* OSInteractionDriver::New(LPVirtualScreen &virtualScreen) {return new OSInteractionDriver(virtualScreen);}

bool OSInteractionDriver::initializeTouch() {return true;}

void OSInteractionDriver::useDefaultScreen(bool use)
{
  m_virtualScreen.UseDefaultScreen(use);
}


void OSInteractionDriver::clickDown(int button, int number)
{
#if __APPLE__
  int clickNum = (number < 1) ? 1 : ((number > 3) ? 3 : number);
  const CGPoint& position = m_virtualScreen.Position();
  CGEventRef eventRef = CGEventCreateMouseEvent(0, kCGEventLeftMouseDown, position, kCGMouseButtonLeft);
  CGEventSetIntegerValueField(eventRef, kCGMouseEventClickState, clickNum);
  CGEventPost(kCGHIDEventTap, eventRef);
  CFRelease(eventRef);
  m_buttonDown = true;
#elif _WIN32
  INPUT input = { 0 };
  input.type = INPUT_MOUSE;
  input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
  SendInput(1, &input, sizeof(INPUT));
#else
  throw_rethrowable std::logic_error("OSInteractionDriver not implemented"); // Linux -- FIXME
#endif
  if (button >= 0 && button < NUM_BUTTONS) {
    m_clickedButtons[button] = true;
  }
}

void OSInteractionDriver::clickUp(int button, int number)
{
  if (button >= 0 && button < NUM_BUTTONS) {
    m_clickedButtons[button] = false;
  }
#if __APPLE__
  int clickNum = (number < 1) ? 1 : ((number > 3) ? 3 : number);
  const CGPoint& position = m_virtualScreen.Position();
  CGEventRef eventRef = CGEventCreateMouseEvent(0, kCGEventLeftMouseUp, position, kCGMouseButtonLeft);
  CGEventSetIntegerValueField(eventRef, kCGMouseEventClickState, clickNum);
  CGEventPost(kCGHIDEventTap, eventRef);
  CFRelease(eventRef);
  m_buttonDown = false;
#elif _WIN32
  INPUT input = { 0 };
  input.type = INPUT_MOUSE;
  input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
  SendInput(1, &input, sizeof(INPUT));
#else
  throw_rethrowable std::logic_error("OSInteractionDriver not implemented"); // Linux -- FIXME
#endif
}

bool OSInteractionDriver::isClickedDown(int button) const
{
  if (button >= 0 && button < NUM_BUTTONS) {
    return m_clickedButtons[button];
  }
  return false;
}

void OSInteractionDriver::keyDown(int code)
{
  emitKeyboardEvent(code, true);
}

void OSInteractionDriver::keyUp(int code)
{
  emitKeyboardEvent(code, false);
}

bool OSInteractionDriver::cursorPosition(float* fx, float* fy) const
{
#if __APPLE__
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
#elif _WIN32
  POINT cursor = { 0, 0 };

  if (GetCursorPos(&cursor)) {
    if (fx) { *fx = static_cast<float>(cursor.x); }
    if (fy) { *fy = static_cast<float>(cursor.y); }
    return true;
  }
  return false;
#else
  throw_rethrowable std::logic_error("OutputPeripheral not implemented"); // Linux -- FIXME
#endif
}

void OSInteractionDriver::setCursorPosition(float fx, float fy, bool absolute)
{
#if defined(__APPLE__) || defined(_WIN32)
  LPPoint position = LPPointMake(static_cast<LPFloat>(fx), static_cast<LPFloat>(fy));
#endif

#if __APPLE__
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
#elif _WIN32
  INPUT input = { 0 };
  input.type = INPUT_MOUSE;
  input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK;

  if (!absolute) {
    POINT cursor = { 0, 0 };

    if (GetCursorPos(&cursor)) {
      position.x += static_cast<LPFloat>(cursor.x);
      position.y += static_cast<LPFloat>(cursor.y);
    }
  } else {
    position.x += 0.5f;
    position.y += 0.5f;
  }
  position = m_virtualScreen.SetPosition(position);
  position = m_virtualScreen.Normalize(position, false);

  // When specifying absolute coordinates, they must be mapped such that max val = 65535.

  input.mi.dx = (int)(65535.0f*static_cast<float>(position.x) + 0.5f);
  input.mi.dy = (int)(65535.0f*static_cast<float>(position.y) + 0.5f);

  SendInput(1, &input, sizeof(INPUT));
#else
  throw_rethrowable std::logic_error("OutputPeripheral not implemented"); // Linux -- FIXME
#endif
}

void OSInteractionDriver::cancelGestureEvents()
{
  m_movingCursor = false;

#if _WIN32
  boost::unique_lock<boost::mutex> lock(m_touchMutex);
  m_touchManager->clearTouches();
#endif
  endGesture();
}

uint32_t OSInteractionDriver::gestureType() const
{
  return m_gesture.type();
}

bool OSInteractionDriver::beginGesture(uint32_t gestureType)
{
  syncPosition();
  return m_gesture.begin(gestureType);
}

bool OSInteractionDriver::endGesture()
{
  syncPosition();
  return m_gesture.end();
}

bool OSInteractionDriver::applyZoom(float zoom)
{
  syncPosition();
  return m_gesture.applyZoom(zoom - 1.0f);
}

bool OSInteractionDriver::applyRotation(float rotation)
{
  syncPosition();
  return m_gesture.applyRotation(rotation);
}

bool OSInteractionDriver::applyScroll(float dx, float dy, int64_t timeDiff)
{
  syncPosition();
  return m_gesture.applyScroll(dx, dy, timeDiff);
}

bool OSInteractionDriver::applyDesktopSwipe(float dx, float dy)
{
  syncPosition();
  return m_gesture.applyDesktopSwipe (dx, dy);
}

void OSInteractionDriver::applyCharms(const Vector& aspectNormalized, int numPointablesActive, int& charmsMode)
{
#if _WIN32
  if (charmsMode == 0 && numPointablesActive == 1) {
    if (aspectNormalized.x >= 1.07f) {
      //Send a "Win + C" to bring out or hide the charm bar
      windowsKeyCombo(VK_LWIN, VkKeyScan('c'));
      charmsMode = 1;
    } else if (aspectNormalized.x <= -0.07f) {
      //Send out a "Win + Ctrl + Tab" to bring out the switchable programs
      windowsKeyCombo(VK_LWIN, VK_CONTROL, VK_TAB);
      charmsMode = 2;
    } else if (aspectNormalized.y <= -0.07f) {
      //Send a "Win + Z" to bring out or hide the app commands
      windowsKeyCombo(VK_LWIN, VkKeyScan('z'));
      charmsMode = 3;
    }
  } else if (aspectNormalized.x >= 0.05f && aspectNormalized.x <= 0.95f &&
             aspectNormalized.y >= 0.05f && aspectNormalized.y <= 0.95f) {
    charmsMode = 0;
  }
#endif
}

bool OSInteractionDriver::checkTouching(const Vector& position, float noTouchBorder) const
{
  return false;
}

void OSInteractionDriver::emitTouchEvent(const TouchEvent& evt)
{
#if _WIN32
  // Translate and convert:
  std::set<Touch> unordered;

  for(auto q = evt.begin(); q != evt.end(); q++) {
    const Touch& cur = *q;

    // Clip to our screen:
    auto position = m_virtualScreen.ClipPosition(LPPoint((LPFloat)cur.x(), (LPFloat)cur.y()));

    // Insert into the new map:
    unordered.insert(Touch(
                           cur.id(),
                           position.x,
                           position.y,
                           cur.touching()
                           ));
  }
  m_touchManager->setTouches(std::move(unordered));
#endif
}

bool OSInteractionDriver::touchAvailable() const
{
#if _WIN32
  return true;
#else
  return false;
#endif
}


int OSInteractionDriver::numTouchScreens() const
{
#if _WIN32
  return m_touchManager->numTouchScreens();
#else
  return 0;
#endif
}

bool OSInteractionDriver::useCharmHelper() const
{
  return m_useCharmHelper;
}

void OSInteractionDriver::emitKeyboardEvent(int key, bool down)
{
#if __APPLE__
  CGEventRef eventRef = CGEventCreateKeyboardEvent(0, LPKeyboard::GetKeyCode(key), down);
  CGEventPost(kCGHIDEventTap, eventRef);
  CFRelease(eventRef);
#elif _WIN32
  INPUT input = { 0 };
  KEYBDINPUT kb = { 0 };
  kb.dwFlags = KEYEVENTF_EXTENDEDKEY;
  if (!down) {
    kb.dwFlags |= KEYEVENTF_KEYUP;
  }
  kb.wVk = key;
  input.type = INPUT_KEYBOARD;
  input.ki = kb;
  SendInput(1, &input, sizeof(INPUT));
#else
  throw_rethrowable std::logic_error("OutputPeripheral not implemented"); // Linux -- FIXME
#endif
}

void OSInteractionDriver::emitKeyboardEvents(int* keys, int numKeys, bool down)
{

if (numKeys <= 0) {
    return;
  }
#if __APPLE__

#elif _WIN32
  INPUT* inputs = new INPUT[numKeys];
  for (int i=0; i<numKeys; i++) {
    KEYBDINPUT kb = { 0 };
    kb.dwFlags = KEYEVENTF_EXTENDEDKEY;
    if (!down) {
      kb.dwFlags |= KEYEVENTF_KEYUP;
    }
    kb.wVk = keys[i];
    inputs[i].type = INPUT_KEYBOARD;
    inputs[i].ki = kb;
  }
  SendInput(static_cast<UINT>(numKeys), inputs, sizeof(INPUT));
  delete[] inputs;
#else
  throw_rethrowable std::logic_error("OutputPeripheral not implemented"); // Linux -- FIXME
#endif
}


void OSInteractionDriver::syncPosition()
{
  LPPoint position;
#if __APPLE__
  CGEventRef eventRef = CGEventCreate(0);
  position = CGEventGetLocation(eventRef);
  CFRelease(eventRef);
#elif _WIN32
  POINT cursor = { 0, 0 };

  if (GetCursorPos(&cursor)) {
    position.x = static_cast<LPFloat>(cursor.x);
    position.y = static_cast<LPFloat>(cursor.y);
  }
#else
  throw_rethrowable std::logic_error("OutputPeripheralImplementation not implemented"); // Linux -- FIXME
#endif
  position = m_virtualScreen.SetPosition(position);
  m_gesture.setPosition(position.x, position.y);
}

}
