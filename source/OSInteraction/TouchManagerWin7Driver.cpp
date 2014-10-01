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

void TouchManagerWin7Driver::AddTouch(const Touch& touchDenorm) {
  Touch touch = NormalizeTouch(touchDenorm);
  m_hidInstance->SendReport((BYTE)touch.id(), touch.touching(), touch.x(), touch.y(), 0.5f, 0.5f);
}

void TouchManagerWin7Driver::UpdateTouch(const Touch& oldTouch, const Touch& touchDenorm) {
  Touch touch = NormalizeTouch(touchDenorm);
  m_hidInstance->SendReport((BYTE)touch.id(), touch.touching(), touch.x(), touch.y(), 0.5f, 0.5f);
}

void TouchManagerWin7Driver::RemoveTouch(const Touch& touchDenorm) {
  Touch touch = NormalizeTouch(touchDenorm);
  m_hidInstance->SendReport((BYTE)touch.id(), touch.touching(), touch.x(), touch.y(), 0.5f, 0.5f);
}

Touch TouchManagerWin7Driver::NormalizeTouch(const Touch& touch) const {
  const LPPoint input((float)touch.x(), (float)touch.y());
  const LPPoint output = m_virtualScreen.Normalize(input);
  Touch retVal(touch);
  retVal.setPos(output.x, output.y);
  return retVal;
}
