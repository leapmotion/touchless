#include "stdafx.h"
#include "GestureInteractionManager.h"
#include "OutputPeripheralBasic.h"
#include "OutputPeripheralFingerMouse.h"
#include "OutputPeripheralGestureOnly.h"
#include "TouchPeripheral.h"


Touchless::GestureInteractionManager* Touchless::GestureInteractionManager::New(Touchless::GestureInteractionMode desiredMode, Touchless::OSInteractionDriver &osInteractionDriver, Touchless::OverlayDriver &overlayDriver)
{
  switch(desiredMode) {
    case Touchless::GestureInteractionMode::OUTPUT_MODE_INTRO:
      return new Touchless::OutputPeripheralGestureOnly(osInteractionDriver, overlayDriver);
    case Touchless::GestureInteractionMode::OUTPUT_MODE_BASIC:
      Touchless::OutputPeripheralBasic::SetBasicMode();
      return new Touchless::OutputPeripheralBasic(osInteractionDriver, overlayDriver);
    case Touchless::GestureInteractionMode::OUTPUT_MODE_ADVANCED:
      if (osInteractionDriver.touchAvailable())
      {
        return new Touchless::TouchPeripheral(osInteractionDriver, overlayDriver);
      }
      else
      {
        return new Touchless::OutputPeripheralFingerMouse(osInteractionDriver, overlayDriver);
      }
    default:
      return nullptr;
  }
}
