#include "stdafx.h"
#include "OSInteraction.h"
#include "Utility/LPVirtualScreen.h"
#include EXCEPTION_PTR_HEADER
#include <fstream>


namespace Touchless
{

OSInteractionDriver::OSInteractionDriver(LPVirtualScreen* virtualScreen)
  : m_virtualScreen(virtualScreen),
  m_movingCursor(false)
{}

OSInteractionDriver::~OSInteractionDriver() {}

bool OSInteractionDriver::initializeTouch() {return true;}

void OSInteractionDriver::useDefaultScreen(bool use)
{
  m_virtualScreen->UseDefaultScreen(use);
}


void OSInteractionDriver::clickDown(int button, int number)
{
  if (button >= 0 && button < NUM_BUTTONS) {
    m_clickedButtons[button] = true;
  }
}

void OSInteractionDriver::clickUp(int button, int number)
{
  if (button >= 0 && button < NUM_BUTTONS) {
    m_clickedButtons[button] = false;
  }
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

}
