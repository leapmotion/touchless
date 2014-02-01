#ifndef __TouchPeripheral_h__
#define __TouchPeripheral_h__

#include "OutputPeripheralImplementation.h"
#include "OutputPeripheralMode.h"
#include "Leap.h"

namespace Touchless
{

class TouchPeripheral : public GestureInteractionManager {

public:

  TouchPeripheral(OSInteractionDriver& osInteractionDriver, OverlayDriver& overlayDriver);

protected:

  virtual void processFrameInternal();
  virtual void identifyRelevantPointables(const Leap::PointableList &pointables, std::vector<Leap::Pointable> &relevantPointables) const;

private:

  int m_charmsMode;
  int m_lastNumIcons;

};

}

#endif
