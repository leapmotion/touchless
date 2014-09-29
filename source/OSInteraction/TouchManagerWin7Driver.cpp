#include "stdafx.h"
#include "TouchManagerWin7Driver.h"
#include "Utility/LPGeometry.h"
#include "Utility/LPVirtualScreen.h"
#include "OcuInterface.h"

static bool DetectDriver(void) {
  COcuHidInstance* hidInstance = COcuInterface().GetFirstCompliantInterface();
  if(!hidInstance)
    return false;

  delete hidInstance;
  return true;
}

const bool TouchManagerWin7Driver::s_supported = DetectDriver();

TouchManagerWin7Driver::TouchManagerWin7Driver(void):
  m_hidInstance(COcuInterface().GetFirstCompliantInterface())
{
}

TouchManagerWin7Driver::~TouchManagerWin7Driver(void) {
  delete m_hidInstance;
}

void TouchManagerWin7Driver::AddTouch(const Touch& touch) {
  // TODO:  Normalize:
  // Send the report over:
  m_hidInstance->SendReport((BYTE)touch.id(), true, touch.x(), touch.y(), 0.5f, 0.5f);
}

void TouchManagerWin7Driver::UpdateTouch(const Touch& oldTouch, const Touch& newTouch) {
  // TODO:  Normalize:
  m_hidInstance->SendReport((BYTE)newTouch.id(), true, newTouch.x(), newTouch.y(), 0.5f, 0.5f);
}

void TouchManagerWin7Driver::RemoveTouch(const Touch& touch) {
  // TODO:  Normalize:
  m_hidInstance->SendReport((BYTE)touch.id(), false, touch.x(), touch.y(), 0.5f, 0.5f);
}
