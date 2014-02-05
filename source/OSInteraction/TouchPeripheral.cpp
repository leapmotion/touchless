#include "stdafx.h"
#include "TouchPeripheral.h"

using namespace Leap;

namespace Touchless
{

TouchPeripheral::TouchPeripheral(OSInteractionDriver& osInteractionDriver, OverlayDriver& overlayDriver) :
  GestureInteractionManager(osInteractionDriver, overlayDriver),
  m_charmsMode(-1),
  m_lastNumIcons(0)
{
}

void TouchPeripheral::processFrameInternal() {
  const std::vector<Leap::Pointable>& pointables = m_relevantPointables;
  const Leap::InteractionBox& box = interactionBox();

  if (!touchAvailable()) {
    return;
  }

  int curNumIcons = 0;
  int numPointablesInteractive = static_cast<int>(pointables.size());
  //Reset charm mode so new fingers must move to screen before activating
  if (numPointablesInteractive == 0) {
    m_charmsMode = -1;
  }
  for (size_t i=0; i<pointables.size(); i++) {
    if (pointables[i].id() >= 0) {
      Vector screenPos, clampVec;
      Vector normPos = box.normalizePoint(pointables[i].stabilizedTipPosition(), false);
      if (pointables[i].touchZone() != Leap::Pointable::ZONE_NONE) {
        // insert actual touch event
        Vector aspectNormalized;
        normalizedToAspect(normPos, aspectNormalized, clampVec, 1.0f, false);
        if (clampVec.magnitude() <= acceptableClampDistance()) {
          if (touchVersion() == 8 && useCharmHelper()) {
            //Windows 8 specific features
            if (pointables[i].touchZone() == Leap::Pointable::ZONE_HOVERING) {
              applyCharms(aspectNormalized, numPointablesInteractive, m_charmsMode);
            } else {
              m_charmsMode = -1;
            }
          }
          addTouchPointForPointable(pointables[i].id(), pointables[i], pointables[i].touchZone() == Leap::Pointable::ZONE_TOUCHING, false);
          drawOverlayForPointable(pointables[i], curNumIcons++, 1.0f, false);
        }
      }
    }
  }
  emitTouchEvent();

  // hide any overlay images that weren't updated
  for (int i=curNumIcons; i<m_lastNumIcons; i++) {
    setIconVisibility(i, false);
  }
  m_lastNumIcons = curNumIcons;
}

void TouchPeripheral::identifyRelevantPointables(const Leap::PointableList &pointables, std::vector<Pointable> &relevantPointables) const {
  // just remove all ZONE_NONE pointables, and backwards pointables.

  relevantPointables.clear();
  for (int i=0; i<pointables.count(); i++) {
    relevantPointables.push_back(pointables[i]);
  }

  auto new_end = std::remove_if(relevantPointables.begin(),
                                relevantPointables.end(),
                                [this] (const Pointable& pointable) -> bool {
                                  return pointable.touchZone() == Pointable::ZONE_NONE || pointable.direction().z > 0;
                                });
  relevantPointables.erase(new_end, relevantPointables.end());
}

}
