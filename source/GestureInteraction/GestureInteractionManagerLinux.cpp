#include "GestureInteractionManager.h"
#include "BasicMode.h"
#include "FingerMouse.h"
#include "GestureOnlyMode.h"

Touchless::GestureInteractionManager* Touchless::GestureInteractionManager::New(Touchless::GestureInteractionMode desiredMode, Touchless::OSInteractionDriver &osInteractionDriver, Touchless::OverlayDriver &overlayDriver)
{
  switch(desiredMode) {
    case Touchless::GestureInteractionMode::OUTPUT_MODE_INTRO:
      return new Touchless::GestureOnlyMode(osInteractionDriver, overlayDriver);
    case Touchless::GestureInteractionMode::OUTPUT_MODE_BASIC:
      Touchless::BasicMode::SetBasicMode();
      return new Touchless::BasicMode(osInteractionDriver, overlayDriver);
    case Touchless::GestureInteractionMode::OUTPUT_MODE_ADVANCED:
      return new Touchless::FingerMouse(osInteractionDriver, overlayDriver);
    default:
      return nullptr;
  }
}
