#ifndef __TouchPeripheral_h__
#define __TouchPeripheral_h__

#include "OutputPeripheralImplementation.h"
#include "OutputPeripheralMode.h"
#include "Leap.h"

class TouchPeripheral : public Leap::OutputPeripheralMode {

public:

  TouchPeripheral(Leap::OutputPeripheralImplementation& outputPeripheral);

protected:

  virtual void processFrameInternal();
  virtual void identifyRelevantPointables(const Leap::PointableList &pointables, std::vector<Leap::Pointable> &relevantPointables) const;

private:

  int m_charmsMode;
  int m_lastNumIcons;

};

#endif
