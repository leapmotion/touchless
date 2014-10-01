#include "stdafx.h"
#include "OSInteractionWin.h"
#include "Utility/LPVirtualScreen.h"
#include EXCEPTION_PTR_HEADER
#include <fstream>


namespace Touchless
{

OSInteractionDriver* OSInteractionDriver::New(LPVirtualScreen* virtualScreen)
{
  return new OSInteractionDriverWin(virtualScreen);
}

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

OSInteractionDriverWin::OSInteractionDriverWin(LPVirtualScreen* virtualScreen)
  : OSInteractionDriver(virtualScreen),
    m_touchManager(nullptr),
    m_useCharmHelper(true)
{ }

OSInteractionDriverWin::~OSInteractionDriverWin()
{
  delete m_touchManager;
}

bool OSInteractionDriverWin::initializeTouch()
{
  m_touchManager = TouchManager::New(m_virtualScreen);
  return m_touchManager != nullptr;
}

void OSInteractionDriverWin::clickDown(int button, int number)
{
  INPUT input = { 0 };
  input.type = INPUT_MOUSE;
  input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
  SendInput(1, &input, sizeof(INPUT));

  OSInteractionDriver::clickDown(button, number);
}

void OSInteractionDriverWin::clickUp(int button, int number)
{
  OSInteractionDriver::clickUp(button, number);

  INPUT input = { 0 };
  input.type = INPUT_MOUSE;
  input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
  SendInput(1, &input, sizeof(INPUT));
}

bool OSInteractionDriverWin::cursorPosition(float* fx, float* fy) const
{
  POINT cursor = { 0, 0 };

  if (GetCursorPos(&cursor)) {
    if (fx) { *fx = static_cast<float>(cursor.x); }
    if (fy) { *fy = static_cast<float>(cursor.y); }
    return true;
  }
  return false;
}

void OSInteractionDriverWin::setCursorPosition(float fx, float fy, bool absolute)
{
  LPPoint position = LPPointMake(static_cast<LPFloat>(fx), static_cast<LPFloat>(fy));

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
  position = m_virtualScreen->SetPosition(position);
  position = m_virtualScreen->Normalize(position, false);

  // When specifying absolute coordinates, they must be mapped such that max val = 65535.

  input.mi.dx = (int)(65535.0f*static_cast<float>(position.x) + 0.5f);
  input.mi.dy = (int)(65535.0f*static_cast<float>(position.y) + 0.5f);

  SendInput(1, &input, sizeof(INPUT));
}

void OSInteractionDriverWin::cancelGestureEvents()
{
  m_movingCursor = false;

  if (m_touchManager != nullptr)
  {
    boost::unique_lock<boost::mutex> lock(m_touchMutex);
    m_touchManager->clearTouches();
  }
  endGesture();
}

void OSInteractionDriverWin::applyCharms(const Vector& aspectNormalized, int numPointablesActive, int& charmsMode)
{
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
}

bool OSInteractionDriverWin::checkTouching(const Vector& position, float noTouchBorder) const
{
  //TODO: this seems wrong
  return false;
}

void OSInteractionDriverWin::emitTouchEvent(const TouchEvent& evt)
{
  if(m_touchManager != nullptr)
  {
    // Translate and convert:
    std::set<Touch> unordered;

    for(auto q = evt.begin(); q != evt.end(); q++) {
      Touch cur = *q;

      // Clip to our screen:
      auto position = m_virtualScreen->ClipPosition(LPPoint((LPFloat)cur.x(), (LPFloat)cur.y()));
      cur.setPos(position.x, position.y);

      // Insert into the new map:
      unordered.insert(cur);
    }

    m_touchManager->setTouches(std::move(unordered));
  }
}

bool OSInteractionDriverWin::touchAvailable() const
{
  return m_touchManager != nullptr;
}

int OSInteractionDriverWin::touchVersion() const {
  return (m_touchManager != nullptr) ? m_touchManager->Version() : 0;
}

int OSInteractionDriverWin::numTouchScreens() const
{
  return (m_touchManager != nullptr) ? m_touchManager->numTouchScreens() : 0;
}

bool OSInteractionDriverWin::useCharmHelper() const
{
  return m_useCharmHelper;
}

void OSInteractionDriverWin::emitKeyboardEvent(int key, bool down)
{
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
}

void OSInteractionDriverWin::emitKeyboardEvents(int* keys, int numKeys, bool down)
{
  if (numKeys <= 0) {
    return;
  }

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
}


void OSInteractionDriverWin::syncPosition()
{
  LPPoint position;

  POINT cursor = { 0, 0 };

  if (GetCursorPos(&cursor)) {
    position.x = static_cast<LPFloat>(cursor.x);
    position.y = static_cast<LPFloat>(cursor.y);
  }

  position = m_virtualScreen->SetPosition(position);
  m_gesture.setPosition(position.x, position.y);
}

}
