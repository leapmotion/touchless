/*==================================================================================================================

 Copyright (c) 2010 - 2013 Leap Motion. All rights reserved.

 The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
 protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
 strictly forbidden unless prior written permission is obtained from Leap Motion.

 ===================================================================================================================*/
#include "LPGesture.h"
#include "OutputPeripheralFingerMouse.h"

#if __APPLE__
#include <sys/sysctl.h>
#endif

namespace Leap {

const bool OutputPeripheralFingerMouse::UseAbsolutePositioning = true;
const bool OutputPeripheralFingerMouse::UseAcceleratedScrolling = false;//true;
const bool OutputPeripheralFingerMouse::UseAcceleratedDesktopSwipe = false;//true;
bool OutputPeripheralFingerMouse::Use3PlusFingerGestures = true;
bool OutputPeripheralFingerMouse::UseRotateandZoom = true;
bool OutputPeripheralFingerMouse::DisableAllOverlays = false;
OutputPeripheralFingerMouse::CURSOR_MOVE_TYPE OutputPeripheralFingerMouse::CursorMoveType = ANY_HOVER;
float OutputPeripheralFingerMouse::ZoomScaleFactor = 1.5;
float OutputPeripheralFingerMouse::TranslationScaleFactor = 1.5;
const int64_t OutputPeripheralFingerMouse::HoverDurationToActivateDrag = 500*OutputPeripheralFingerMouse::MILLISECONDS;

// ////////////////////////////////////////////////////
//OUTPUT_MODE_FINGER_MOUSE process helper functions

// GENERAL OVERVIEW OF FINGER MOUSE STATE MACHINE
//
// There are four groups of states:
// - 0 fingers
// - 1 finger
// - 2 fingers
// - 3+ fingers
//
// In each state group, there are states which correspond to the "hover" and "touching"
// zones (see Pointable::Zone).  There are generally multiple states associated with "touching",
// for example "clicking" (in 1 finger) and "zooming", "rotating", and "scrolling" (in 2 fingers).
//
// The states correspond roughly exactly with the state of interaction.

OutputPeripheralFingerMouse::OutputPeripheralFingerMouse(OutputPeripheralImplementation& outputPeripheral)
  :
  OutputPeripheralMode(outputPeripheral),
  m_drawOverlays(true),
  m_noTouching(true),
  m_clickDuration(0),
  m_clickNumber(0),
  m_lastClickTime(0),
  m_mountainLionOrNewer(false)
{
  m_fingerMouseStateMachine.SetOwnerClass(this, "OutputPeripheralFingerMouse");

  // This tells the state machine to log transitions to cerr.  An optional second can be used to specify
  // an event ID to ignore (such as something that happens every single frame) to avoid spamming the console.
  // Specify that OMFM__PROCESS_FRAME should be ignored.
  //m_fingerMouseStateMachine.SetTransitionLogger(&std::cerr, OMFM__PROCESS_FRAME);

#ifdef __APPLE__
  // Determine if we are running Mac OS X 10.8 (Mountain Lion) or something even newer
  {
    char str[256];
    size_t size = sizeof(str);
    if (!sysctlbyname("kern.osrelease", str, &size, nullptr, 0)) {
      m_mountainLionOrNewer = (atoi(str) >= 12);
    }
  }
#endif
}

OutputPeripheralFingerMouse::~OutputPeripheralFingerMouse() {
  for (int i = 0; i < m_numOverlayImages; ++i) {
    setIconVisibility(i, false);
  }
#if __APPLE__
  flushOverlay();
#endif
  m_fingerMouseStateMachine.Shutdown();
}

void OutputPeripheralFingerMouse::processFrameInternal() {
  m_noTouching = std::count_if(m_relevantPointables.begin(), m_relevantPointables.end(),
                          [this] (const Pointable& pointable) {return pointable.touchZone() == Pointable::ZONE_TOUCHING;}) == 0;
  // initialize finger mouse state machine if necessary
  if (!m_fingerMouseStateMachine.IsInitialized()) {
    m_fingerMouseStateMachine.Initialize(&OutputPeripheralFingerMouse::State_FingerMouse_0Fingers_NoInteraction,
                                         "State_FingerMouse_0Fingers_NoInteraction");
  }

  // clear overlays
  if (m_flushOverlay && m_drawOverlays && !DisableAllOverlays) {
    for (int i = 0; i < m_numOverlayImages; ++i) {
      setIconVisibility(i, false);
    }
  }

  // run the state machine with the "process frame" input
  m_fingerMouseStateMachine.RunCurrentState(OMFM__PROCESS_FRAME);

#if __APPLE__
  if (m_flushOverlay && m_drawOverlays && !DisableAllOverlays) {
    flushOverlay();
  }
#endif
}

Pointable::Zone OutputPeripheralFingerMouse::identifyCollectivePointableZone (const std::vector<Pointable> &pointables) const {
  // for now, use the "maximum" of the pointables' zones, where the order is given by the enum values
  // ZONE_NONE = 0, ZONE_HOVERING = 1, ZONE_TOUCHING = 2.

  Pointable::Zone max = Pointable::ZONE_NONE;
  for (size_t i = 0; i < pointables.size(); ++i) {
    if (max < pointables[i].touchZone()) {
      max = pointables[i].touchZone();
    }
  }
  return max;
}

void OutputPeripheralFingerMouse::DrawOverlays() {
  if (!m_drawOverlays || DisableAllOverlays) {
    return;
  }

  Vector screenPosition, clampVec;
  for (int i = 0; i < m_numOverlayImages; ++i) {
    if (i < static_cast<int>(m_relevantPointables.size()) &&
        normalizedToScreen(interactionBox().normalizePoint(m_relevantPointables[i].stabilizedTipPosition()),
                           screenPosition,
                           clampVec)) {
      float clampDist = clampVec.magnitude();
      if (useProceduralOverlay()) {
        float touchDistance = m_relevantPointables[i].touchDistance();
        double radius = touchDistanceToRadius(touchDistance);
        // TEMP: hacky way to do hover-drag visualization
        // if we are clicking, grow the radius of the icon to indicate drag progress
        if (m_fingerMouseStateMachine.CurrentState() ==
            &OutputPeripheralFingerMouse::State_FingerMouse_1Finger_Clicking &&
            m_clickEligibleForDrag) {
          double parameter = double(m_clickDuration) / HoverDurationToActivateDrag;
          if (parameter > 1.0) {
            parameter = 1.0;
          }
          // linearly interpolate from the touchDistance-based radius to the max dragging-icon-radius.
          radius = (1.0-parameter)*radius + parameter*30.0*0.7*0.8;
        } else if (m_fingerMouseStateMachine.CurrentState() ==
                   &OutputPeripheralFingerMouse::State_FingerMouse_1Finger_Dragging) {
          radius = 30.0*0.7;
        } else {
          if (touchDistance < 0.0) {
            radius = 10.0;
          }
        }
        drawRasterIcon(i,
                       screenPosition.x,
                       screenPosition.y,
                       true,
                       m_relevantPointables[i].tipVelocity().toVector3<Vector3>(),
                       touchDistance,
                       radius,
                       clampDist,
                       alphaFromTimeVisible(m_relevantPointables[i].timeVisible()));
      } else {
        int imageIndex = findImageIndex(m_relevantPointables[i].touchDistance(), 0, 1);
        drawImageIcon(i, imageIndex, screenPosition.x, screenPosition.y, true);
      }
    } else {
      setIconVisibility(i, false);
    }
  }
}

void OutputPeripheralFingerMouse::stopActiveEvents()
{
  m_fingerMouseStateMachine.RunCurrentState(OMFM__LOST_FOCUS);
}

void OutputPeripheralFingerMouse::DrawPointingOverlay() {
  if (!m_drawOverlays || DisableAllOverlays) {
    return;
  }

  drawOverlayForPointable(m_currentFrame.pointable(favoritePointableId()));
}

void OutputPeripheralFingerMouse::DrawClickingOrDraggingOverlay() {
  if (!m_drawOverlays || DisableAllOverlays) {
    return;
  }

  Pointable favorite_pointable = m_currentFrame.pointable(favoritePointableId());
  if (!favorite_pointable.isValid()) {
    return;
  }

  Vector screenPosition, clampVec, position;
  m_positionalDeltaTracker.setPositionToStabilizedPositionOf(favorite_pointable);
  position = m_positionalDeltaTracker.getTrackedPosition();

  {
    normalizedToScreen(interactionBox().normalizePoint(position),
                       screenPosition,
                       clampVec);
    float clampDist = clampVec.magnitude();
    float touchDistance = favorite_pointable.touchDistance();
    double radius = touchDistanceToRadius(touchDistance);
    // TEMP: hacky way to do hover-drag visualization
    // if we are clicking, grow the radius of the icon to indicate drag progress
    if (m_fingerMouseStateMachine.CurrentState() ==
        &OutputPeripheralFingerMouse::State_FingerMouse_1Finger_Clicking &&
        m_clickEligibleForDrag) {
      double parameter = double(m_clickDuration) / HoverDurationToActivateDrag;
      // linearly interpolate from the touchDistance-based radius to the max dragging-icon-radius.
      radius = (1.0-parameter)*radius + parameter*30.0*0.7*0.8;
    } else if (m_fingerMouseStateMachine.CurrentState() ==
               &OutputPeripheralFingerMouse::State_FingerMouse_1Finger_Dragging) {
      radius = 30.0*0.7;
    } else {
      if (touchDistance < 0.0) {
        radius = 10.0;
      }
    }
    drawRasterIcon(0,
                   screenPosition.x,
                   screenPosition.y,
                   true,
                   favorite_pointable.tipVelocity().toVector3<Vector3>(),
                   touchDistance,
                   radius,
                   clampDist,
                   alphaFromTimeVisible(favorite_pointable.timeVisible()));
  }
}

void OutputPeripheralFingerMouse::DrawScrollOverlay(bool doubleHorizontalDots, bool doubleVerticalDots) {
  if (!m_drawOverlays || DisableAllOverlays) {
    return;
  }

  //Find a hand
  Pointable favorite_pointable = m_currentFrame.pointable(favoritePointableId());
  if (!favorite_pointable.isValid()) {
    return;
  }

  Hand hand = favorite_pointable.hand();
  if (!hand.isValid()) {
    drawScrollOverlayForPointable(favorite_pointable);
  } else {
    //Draw the overlay
    drawScrollOverlayForHand(hand);
  }
}

void OutputPeripheralFingerMouse::DrawRotateOverlay() {
  if (!m_drawOverlays || DisableAllOverlays) {
    return;
  }

  //Find a hand
  Pointable favorite_pointable = m_currentFrame.pointable(favoritePointableId());
  if (!favorite_pointable.isValid()) {
    return;
  }

  Hand hand = favorite_pointable.hand();
  if (!hand.isValid()) {
    drawRotateOverlayForPointable(favorite_pointable);
  } else {
    //Draw the overlay
    drawRotateOverlayForHand(hand);
  }
}

void OutputPeripheralFingerMouse::DrawZoomOverlay() {
  if (!m_drawOverlays || DisableAllOverlays) {
    return;
  }

  //Find a hand
  Pointable favorite_pointable = m_currentFrame.pointable(favoritePointableId());
  if (!favorite_pointable.isValid()) {
    return;
  }

  Hand hand = favorite_pointable.hand();
  if (!hand.isValid()) {
    drawZoomOverlayForPointable(favorite_pointable);
  } else {
    //Draw the overlay
    drawZoomOverlayForHand(hand);
  }
}

void OutputPeripheralFingerMouse::DrawDesktopSwipeOverlay(bool movementGlow) {
  if (!m_drawOverlays || DisableAllOverlays) {
    return;
  }

  //Find a hand
  Pointable favorite_pointable = m_currentFrame.pointable(favoritePointableId());
  if (!favorite_pointable.isValid()) {
    return;
  }

  Hand hand = favorite_pointable.hand();
  if (!hand.isValid()) {
    drawScrollOverlayForPointable(favorite_pointable, 1.0f, nullptr, true, false, movementGlow, movementGlow); // double horizontal dots, not vertical ones
  } else {
    //Draw the overlay
    drawScrollOverlayForHand(hand, 1.0f, nullptr, true, false, movementGlow, movementGlow); // double horizontal dots, not vertical ones
  }
}

void OutputPeripheralFingerMouse::DrawVerticalPalmOverlay() {
  if (!m_drawOverlays || DisableAllOverlays) {
    return;
  }

  //Find a hand
  Hand hand;
  Pointable favorite_pointable = m_currentFrame.pointable(favoritePointableId());
  if (favorite_pointable.isValid()) {
    hand = favorite_pointable.hand();
  } else {
    if (m_currentFrame.hands().count() != 0) {
      hand = m_currentFrame.hands()[0];
    } else {
      return;
    }
  }
  if (hand.isValid()) {
    //Draw the overlay
    drawScrollOverlayForHand(hand, 1.0f, nullptr, false, true, false, false); // double vertical dots, not horizontal ones
  } else {
    drawScrollOverlayForPointable(favorite_pointable, 1.0f, nullptr, false, true, false, false); // double vertical dots, not horizontal ones
  }
}

void OutputPeripheralFingerMouse::setDeltaTrackedAbsoluteCursorPositionHand (const Hand &hand, Vector *calculatedScreenPosition) {
  if (!hand.isValid()) {
    return;
  }
  // use the hand to update the PositionalDeltaTracker
  m_positionalDeltaTracker.setPositionToStabilizedPositionOf(hand);
  OutputPeripheralMode::setAbsoluteCursorPosition(m_positionalDeltaTracker.getTrackedPosition(), calculatedScreenPosition);
}

void OutputPeripheralFingerMouse::setDeltaTrackedAbsoluteCursorPositionPointable (const Pointable &pointable, Vector *calculatedScreenPosition) {
  if (!pointable.isValid()) {
    return;
  }
  // use the hand to update the PositionalDeltaTracker
  m_positionalDeltaTracker.setPositionToStabilizedPositionOf(pointable); // false indicates it is not a hand
  OutputPeripheralMode::setAbsoluteCursorPosition(m_positionalDeltaTracker.getTrackedPosition(), calculatedScreenPosition);
}

void OutputPeripheralFingerMouse::generateScrollBetweenFrames (const Frame &currentFrame, const Frame &sinceFrame) {
  float scroll_dx, scroll_dy;
  if (UseAcceleratedScrolling) {
    if (currentFrame.pointables().count() == 0) {
      return; // do nothing if there are no pointables
    }
    // use the cdGain function to accelerate the scrolling
    // first, calculate the average fingertip velocity of the current frame.
    Vector averageTipVelocity(0.0, 0.0, 0.0);
    for (int i = 0; i < currentFrame.pointables().count(); i++) {
      averageTipVelocity += currentFrame.pointables()[i].tipVelocity();
    }
    averageTipVelocity /= static_cast<float>(currentFrame.pointables().count());
    double gain = cdGain(sqrt(averageTipVelocity.x * averageTipVelocity.x + averageTipVelocity.y * averageTipVelocity.y), 40000.0, 0.66);
    scroll_dx = static_cast<float>(gain * averageTipVelocity.x);
    scroll_dy = static_cast<float>(-1 * gain * averageTipVelocity.y);
  } else {
    // scaled scrolling

    // damp out small motions
    Vector translation = currentFrame.translation(sinceFrame);
    float timeDelta = float(currentFrame.timestamp() - sinceFrame.timestamp()) / SECONDS;
    translation *= scrollDampingFactor(translation / timeDelta);

    scroll_dx = static_cast<float>(TranslationScaleFactor * translation.x);
    scroll_dy = static_cast<float>(-TranslationScaleFactor * translation.y);
  }

  applyScroll(scroll_dx, scroll_dy, currentFrame.timestamp() - sinceFrame.timestamp());
}

void OutputPeripheralFingerMouse::generateDesktopSwipeBetweenFrames (const Frame &currentFrame, const Frame &sinceFrame) {
  float scroll_dx, scroll_dy;
  if (UseAcceleratedDesktopSwipe) {
    if (currentFrame.pointables().count() == 0) {
      return; // do nothing if there are no pointables
    }
    // use the cdGain function to accelerate the scrolling
    // first, calculate the average pointable tip velocity of the current frame.
    Vector averageTipVelocity(0.0, 0.0, 0.0);
    for (int i = 0; i < currentFrame.pointables().count(); i++) {
      averageTipVelocity += currentFrame.pointables()[i].tipVelocity();
    }
    averageTipVelocity /= static_cast<float>(currentFrame.pointables().count());
    double gain = cdGain(sqrt(averageTipVelocity.x * averageTipVelocity.x + averageTipVelocity.y * averageTipVelocity.y), 80000.0, 0.4);
    scroll_dx = static_cast<float>(gain * averageTipVelocity.x);
    scroll_dy = static_cast<float>(-1 * gain * averageTipVelocity.y);
  } else {
    // scaled scrolling

    // damp out small motions
    Vector translation = currentFrame.translation(sinceFrame);
    float timeDelta = float(currentFrame.timestamp() - sinceFrame.timestamp()) / SECONDS;
    translation *= scrollDampingFactor(translation / timeDelta);

    scroll_dx = static_cast<float>(translation.x);
    scroll_dy = static_cast<float>(-translation.y);
  }

  applyDesktopSwipe(scroll_dx, scroll_dy);
}

double OutputPeripheralFingerMouse::cdGain (double magnitude,
                                            double maxVelocity,
                                            double initialPower) const {
  //   const double maxVelocity = 20000;//3200;
  //   const double initialPower = .7;//.8;
  const double maxGain = .82378;

  double velTerm1 = (magnitude/maxVelocity);
  double velTerm2 = exp(1-(magnitude/maxVelocity));
  double term2 = pow((velTerm1*velTerm2),initialPower);
  double cdGain = maxGain * term2;

  return cdGain;
}

// double OutputPeripheralFingerMouse::awesomeAccel (double velocity, double maxVelocity) const {
//   if (velocity >= maxVelocity) {
//     return velocity;
//   } else {
//
//   }
// }

bool OutputPeripheralFingerMouse::shouldBeInPalmSwipeMode () const {
  static const double COS_X_AXIS_ANGLE_THRESHOLD = 0.75; // the cosine of the angle to the X axis

  //Find a hand
  Hand hand;
  Pointable favorite_pointable = m_currentFrame.pointable(foremostPointableId());
  if (!favorite_pointable.isValid()) {
    if (m_currentFrame.hands().count() == 0) {
      return false;
    } else {
      hand = m_currentFrame.hands()[0];
    }
  } else {
    hand = favorite_pointable.hand();
  }
  if (!hand.isValid()) {
    return false;
  }

  return Use3PlusFingerGestures && std::abs(hand.palmNormal().x) >= COS_X_AXIS_ANGLE_THRESHOLD;
}

// This macro is part of the state machine -- used for convenience, to avoid
// having to type such a long and ugly statement.
#define FINGERMOUSE_TRANSITION_TO(x) m_fingerMouseStateMachine.SetNextState(&OutputPeripheralFingerMouse::x, #x)

void OutputPeripheralFingerMouse::FingerMouse_TransitionTo_NFingers_NoInteraction () {
  if (Use3PlusFingerGestures && shouldBeInPalmSwipeMode()) {
    FINGERMOUSE_TRANSITION_TO(State_FingerMouse_Palm_GestureRecognition);
    return;
  }

  size_t fingerCount = 0;
  if (pointableCountIsUnambiguous()) {
    fingerCount = pointableCount();
  }
  switch (fingerCount) {
    case 0:
      FINGERMOUSE_TRANSITION_TO(State_FingerMouse_0Fingers_NoInteraction);
      break;

    case 1:
      FINGERMOUSE_TRANSITION_TO(State_FingerMouse_1Finger_Hovering);
      break;

    case 2:
      FINGERMOUSE_TRANSITION_TO(State_FingerMouse_2Fingers_Hovering);
      break;

    default: // 3 or higher
      if (Use3PlusFingerGestures) {
        FINGERMOUSE_TRANSITION_TO(State_FingerMouse_3PlusFingers_NoInteraction);
      } else {
        FINGERMOUSE_TRANSITION_TO(State_FingerMouse_2Fingers_Hovering);
      }
      break;
  }
}

// ////////////////////////////////////////////////////
// beginning of OUTPUT_MODE_FINGER_MOUSE state handlers

bool OutputPeripheralFingerMouse::State_FingerMouse_0Fingers_NoInteraction (StateMachineInput input) {
  switch (input) {
    case OMFM__LOST_FOCUS:
      return true;
    case SM_ENTER:
      m_drawOverlays = true;
      m_positionalDeltaTracker.clear();
    case OMFM__PROCESS_FRAME:
      // handle finger-count changes
      if ((pointableCountIsUnambiguous() && pointableCount() > 0) || shouldBeInPalmSwipeMode()) {
        FingerMouse_TransitionTo_NFingers_NoInteraction();
        return true;
      }

      // nothing else to handle, no overlays to draw
      return true;
  }
  return false; // MUST return false if an event was not handled.
}

bool OutputPeripheralFingerMouse::State_FingerMouse_1Finger_Hovering (StateMachineInput input) {
  switch (input) {
    case OMFM__LOST_FOCUS:
      FINGERMOUSE_TRANSITION_TO(State_FingerMouse_0Fingers_NoInteraction);
      return true;

    case SM_ENTER:
      m_drawOverlays = true;
      resetFavoritePointableId();
    case OMFM__PROCESS_FRAME:
      // handle finger-count changes
      if ((pointableCountIsUnambiguous() && pointableCount() != 1) || shouldBeInPalmSwipeMode()) {
        FingerMouse_TransitionTo_NFingers_NoInteraction();
        return true;
      }

      // handle zone changes
      if (collectiveZone() == Pointable::ZONE_TOUCHING) {
        FINGERMOUSE_TRANSITION_TO(State_FingerMouse_1Finger_Clicking);
        return true;
      }

      DrawPointingOverlay();

      // handle cursor positioning
      //setAbsoluteCursorPositionPointable(m_currentFrame.pointable(foremostPointableId()));
      setDeltaTrackedAbsoluteCursorPositionPointable(m_currentFrame.pointable(favoritePointableId()));

      return true;
  }
  return false; // MUST return false if an event was not handled.
}

bool OutputPeripheralFingerMouse::State_FingerMouse_1Finger_Clicking (StateMachineInput input) {
  switch (input) {
    case OMFM__LOST_FOCUS:
      FINGERMOUSE_TRANSITION_TO(State_FingerMouse_0Fingers_NoInteraction);
      return true;

    case SM_ENTER:
      m_drawOverlays = true;
      if ((pointableCountIsUnambiguous() && pointableCount() == 0) || shouldBeInPalmSwipeMode()) {
        FingerMouse_TransitionTo_NFingers_NoInteraction();
      }
      // handle cursor positioning, saving the screen position.
      //setAbsoluteCursorPositionPointable(m_currentFrame.pointable(foremostPointableId()), &m_clickDownScreenPosition);
      setDeltaTrackedAbsoluteCursorPositionPointable(m_currentFrame.pointable(favoritePointableId()), &m_clickDownScreenPosition);

      // reset click number if we're too far away (in time or space) from the last click
      if (m_currentFrame.timestamp() - m_lastClickTime > 3*SECONDS
          || m_clickDownScreenPosition.distanceTo(m_lastClickLocation) > 50 // 50 pixels (?)
          || m_clickNumber == 3) {
        m_clickNumber = 0;
      }

      // generate down-click event
      clickDown(0, ++m_clickNumber); // just do button 0 for now.
      m_deferClickUpForDrag = false;
      m_clickEligibleForDrag = true;
      m_lastClickTime = m_currentFrame.timestamp();
      m_lastClickLocation = m_clickDownScreenPosition;
      return true;

    case SM_EXIT:
      // generate up-click event
      // TODO: figure out if it should be possible to exit this state without a clickUp(0)
      if (!m_deferClickUpForDrag) {
        clickUp(0, m_clickNumber); // just do button 0 for now.
      }
      m_clickDuration = 0; // reset the click duration to sentinel value
      return true;

    case OMFM__PROCESS_FRAME:
      // handle finger-count changes
      if ((pointableCountIsUnambiguous() && pointableCount() != 1) || shouldBeInPalmSwipeMode()) {
        FingerMouse_TransitionTo_NFingers_NoInteraction();
        return true;
      }

      // handle zone changes
      if (collectiveZone() != Pointable::ZONE_TOUCHING) {
        FINGERMOUSE_TRANSITION_TO(State_FingerMouse_1Finger_Hovering);
        return true;
      }

      DrawClickingOrDraggingOverlay();

      m_clickDuration = m_currentFrame.timestamp() - m_lastClickTime;
      Vector position;
      position = m_positionalDeltaTracker.getTrackedPosition();

      {
        Vector screenPosition, clampVec;
        normalizedToScreen(interactionBox().normalizePoint(position),
                           screenPosition,
                           clampVec);
        double distance = m_lastClickLocation.distanceTo(screenPosition);
        //std::cerr << "m_clickDuration = " << m_clickDuration << ", distance = " << distance << '\n';
        if (distance >= 50) {
          m_clickEligibleForDrag = false;
        }
        if (m_clickEligibleForDrag && m_clickDuration > HoverDurationToActivateDrag) {
          m_deferClickUpForDrag = true;
          m_clickNumber = 0; // no double click after drag
          FINGERMOUSE_TRANSITION_TO(State_FingerMouse_1Finger_Dragging);
          return true;
        }
      }

      return true;
  }
  return false; // MUST return false if an event was not handled.
}

bool OutputPeripheralFingerMouse::State_FingerMouse_1Finger_Dragging (StateMachineInput input) {
  switch (input) {
    case OMFM__LOST_FOCUS:
      FINGERMOUSE_TRANSITION_TO(State_FingerMouse_0Fingers_NoInteraction);
      return true;

    case SM_ENTER:
      m_drawOverlays = true;
    case OMFM__PROCESS_FRAME:
      // handle finger-count changes
      if ((pointableCountIsUnambiguous() && pointableCount() != 1) || shouldBeInPalmSwipeMode()) {
        FingerMouse_TransitionTo_NFingers_NoInteraction();
        return true;
      }

      // handle zone changes
      if (collectiveZone() != Pointable::ZONE_TOUCHING) {
        FINGERMOUSE_TRANSITION_TO(State_FingerMouse_1Finger_Hovering);
        return true;
      }

      DrawClickingOrDraggingOverlay();

      // handle cursor positioning
      //setAbsoluteCursorPositionPointable(m_currentFrame.pointable(foremostPointableId()));
      setDeltaTrackedAbsoluteCursorPositionPointable(m_currentFrame.pointable(favoritePointableId()));

      return true;

    case SM_EXIT:
      // generate up-click event
      clickUp(0, m_clickNumber); // just do button 0 for now.
      return true;
  }
  return false; // MUST return false if an event was not handled.
}

bool OutputPeripheralFingerMouse::State_FingerMouse_2Fingers_Hovering (StateMachineInput input) {
  switch (input) {
    case OMFM__LOST_FOCUS:
      FINGERMOUSE_TRANSITION_TO(State_FingerMouse_0Fingers_NoInteraction);
      return true;

    case SM_ENTER:
      m_drawOverlays = true;
      resetFavoritePointableId();
    case OMFM__PROCESS_FRAME:
      // handle finger-count changes
      if (shouldBeInPalmSwipeMode() ||
          (pointableCountIsUnambiguous() && pointableCount() != 2 && (Use3PlusFingerGestures || pointableCount() < 3))) {
        FingerMouse_TransitionTo_NFingers_NoInteraction();
        return true;
      }

      // handle zone changes
      if (collectiveZone() == Pointable::ZONE_TOUCHING) {
        if (UseRotateandZoom){
          FINGERMOUSE_TRANSITION_TO(State_FingerMouse_2Fingers_GestureRecognition);
        } else {
          FINGERMOUSE_TRANSITION_TO(State_FingerMouse_2Fingers_Scrolling);
        }
        return true;
      }

      DrawScrollOverlay();

      if (m_currentFrame.pointable(favoritePointableId()).hand().isValid()) {
        setDeltaTrackedAbsoluteCursorPositionHand(m_currentFrame.pointable(favoritePointableId()).hand());
      } else {
        setDeltaTrackedAbsoluteCursorPositionPointable(m_currentFrame.pointable(favoritePointableId()));
      }

      return true;
  }
  return false; // MUST return false if an event was not handled.
}

bool OutputPeripheralFingerMouse::State_FingerMouse_2Fingers_GestureRecognition (StateMachineInput input) {
  switch (input) {
    case OMFM__LOST_FOCUS:
      FINGERMOUSE_TRANSITION_TO(State_FingerMouse_0Fingers_NoInteraction);
      return true;

    case SM_ENTER:
      m_drawOverlays = true;
      m_gestureStart = m_currentFrame;
      if (m_currentFrame.pointable(favoritePointableId()).hand().isValid()) {
        setDeltaTrackedAbsoluteCursorPositionHand(m_currentFrame.pointable(favoritePointableId()).hand());
      } else {
        setDeltaTrackedAbsoluteCursorPositionPointable(m_currentFrame.pointable(favoritePointableId()));
      }    case OMFM__PROCESS_FRAME: {
      // handle finger-count changes
      if ((pointableCountIsUnambiguous() && pointableCount() == 0) || shouldBeInPalmSwipeMode()) {
        FingerMouse_TransitionTo_NFingers_NoInteraction();
        return true;
      }

      // handle zone changes
      if (m_noTouching) {
        if (collectiveZone() != Pointable::ZONE_TOUCHING) {
          FINGERMOUSE_TRANSITION_TO(State_FingerMouse_2Fingers_Hovering);
          return true;
        }
      }

      DrawScrollOverlay();

      auto it = m_timedFrameHistory.getFrameHavingAgeAtLeast(100*MILLISECONDS);
      if (it != m_timedFrameHistory.end()) {
        const Frame &frame = it->second;
        Vector translation = m_currentFrame.translation(frame);
        float time_delta = float(m_currentFrame.timestamp() - frame.timestamp()) / SECONDS;
        float effective_translation_magnitude = sqrt(translation.x*translation.x + translation.y*translation.y);
        float effective_translation_velocity = effective_translation_magnitude / time_delta;
        //std::cerr << "effective_translation_magnitude = " << effective_translation_magnitude
        //          << "effective velocity = " << effective_translation_velocity << '\n';
        static const float THRESHOLD = 0.75f;
        if (m_currentFrame.rotationProbability(frame) > THRESHOLD) {
          FINGERMOUSE_TRANSITION_TO(State_FingerMouse_2Fingers_Rotating);
          return true;
        } else if (m_currentFrame.scaleProbability(frame) > THRESHOLD) {
          FINGERMOUSE_TRANSITION_TO(State_FingerMouse_2Fingers_Zooming);
          return true;
        } else if (m_currentFrame.translationProbability(frame) > THRESHOLD && effective_translation_velocity > 50.0f) {
          FINGERMOUSE_TRANSITION_TO(State_FingerMouse_2Fingers_Scrolling);
          return true;
        }
      }


//       Vector translation = m_currentFrame.translation(m_gestureStart);
//       double effective_translation_magnitude = sqrt(translation.x*translation.x + translation.y*translation.y);
//
//       if (m_currentFrame.timestamp() - m_gestureStart.timestamp() >= 20*MILLISECONDS && rtsIsUnambiguous()) {
//         if (UseRotateandZoom && rts() == ROTATING) {
//           FINGERMOUSE_TRANSITION_TO(State_FingerMouse_2Fingers_Rotating);
//           return true;
//         } else if (rts() == TRANSLATING && effective_translation_magnitude > 7) {
//           FINGERMOUSE_TRANSITION_TO(State_FingerMouse_2Fingers_Scrolling);
//           return true;
//         } else if (UseRotateandZoom && rts() == SCALING) {
//           FINGERMOUSE_TRANSITION_TO(State_FingerMouse_2Fingers_Zooming);
//           return true;
//         }
//       }

      // if no gesture "won", it's ok, it may just take more frames before it's clear.
      return true;
    }
  }
  return false; // MUST return false if an event was not handled.
}

bool OutputPeripheralFingerMouse::State_FingerMouse_2Fingers_Rotating (StateMachineInput input) {
  switch (input) {
    case OMFM__LOST_FOCUS:
      FINGERMOUSE_TRANSITION_TO(State_FingerMouse_0Fingers_NoInteraction);
      return true;

    case SM_ENTER:
      m_drawOverlays = true;
      // generate begin-rotate event
      beginGesture(LPGesture::GestureRotate);
      applyRotation(-RAD_TO_DEG*m_currentFrame.rotationAngle(m_gestureStart, Vector::zAxis()));
      return true;

    case SM_EXIT:
      // generate end-rotate event
      endGesture();
      return true;

    case OMFM__PROCESS_FRAME:
      // handle finger-count changes
      if ((pointableCountIsUnambiguous() && pointableCount() == 0) || shouldBeInPalmSwipeMode()) {
        FingerMouse_TransitionTo_NFingers_NoInteraction();
        return true;
      }

      // handle zone changes
      if (m_noTouching) {
        if (collectiveZone() != Pointable::ZONE_TOUCHING) {
          FINGERMOUSE_TRANSITION_TO(State_FingerMouse_2Fingers_Hovering);
          return true;
        }
      }

      DrawRotateOverlay();

      // handle cursor positioning
      if (CursorMoveType == ALWAYS) {
        //setAbsoluteCursorPosition();
        if (m_currentFrame.pointable(favoritePointableId()).hand().isValid()) {
          setDeltaTrackedAbsoluteCursorPositionHand(m_currentFrame.pointable(favoritePointableId()).hand());
        } else {
          setDeltaTrackedAbsoluteCursorPositionPointable(m_currentFrame.pointable(favoritePointableId()));
        }
      }

      // apply rotation
      applyRotation(-RAD_TO_DEG*m_currentFrame.rotationAngle(m_sinceFrame, Vector::zAxis()));

      return true;
  }
  return false; // MUST return false if an event was not handled.
}

bool OutputPeripheralFingerMouse::State_FingerMouse_2Fingers_Scrolling (StateMachineInput input) {
  switch (input) {
    case OMFM__LOST_FOCUS:
      FINGERMOUSE_TRANSITION_TO(State_FingerMouse_0Fingers_NoInteraction);
      return true;

    case SM_ENTER:
      m_drawOverlays = true;
      // generate begin-scroll event
      beginGesture(LPGesture::GestureScroll);
      generateScrollBetweenFrames(m_currentFrame, m_gestureStart);
      return true;

    case SM_EXIT:
      // generate end-scroll event
      endGesture();
      return true;

    case OMFM__PROCESS_FRAME:
      // handle finger-count changes
      if ((pointableCountIsUnambiguous() && pointableCount() == 0) || shouldBeInPalmSwipeMode()) {
        FingerMouse_TransitionTo_NFingers_NoInteraction();
        return true;
      }

      // handle zone changes
      if (m_noTouching) {
        if (collectiveZone() != Pointable::ZONE_TOUCHING) {
          FINGERMOUSE_TRANSITION_TO(State_FingerMouse_2Fingers_Hovering);
          return true;
        }
      }

      /*
      // do scroll-end robustification -- based on dot product and finger pitch.
      // taken from Jon's old finger mouse code.  because we need access to actual
      // fingers (and not the filtered finger count), don't do this if there isn't
      // at least one finger.
      if (m_relevantPointables.size() >= 1) {
        int numPointables = static_cast<int>(m_relevantPointables.size());

        // figure out which fingers are "best" for this determination -- based on length.
        int best0, best1;
        if (numPointables >= 2) {
          // Find the two longest fingers (Largest 2 of N)
          if (m_relevantPointables[0].length() >= m_relevantPointables[1].length()) {
            best0 = 0; best1 = 1;
          } else {
            best0 = 1; best1 = 0;
          }
          for (int i = 2; i < numPointables; i++) {
            if (m_relevantPointables[i].length() > m_relevantPointables[best1].length()) {
              if (m_relevantPointables[i].length() > m_relevantPointables[best0].length()) {
                best1 = best0;
                best0 = i;
              } else {
                best1 = i;
              }
            }
          }
        } else {
          best0 = best1 = 0;
        }

        const float pitch0 = std::fabs(m_relevantPointables[best0].direction().pitch());
        const float pitch1 = std::fabs(m_relevantPointables[best1].direction().pitch());
        const float dotProduct = m_relevantPointables[best0].direction().dot(m_relevantPointables[best1].direction());
        if (dotProduct >= 0.94 && pitch0 <= 0.4 && pitch1 <= 0.4) { // || latest.position.z > (closestZ + 22)) {
          generateScrollBetweenFrames(m_currentFrame, m_sinceFrame);
        }
      } else {
        generateScrollBetweenFrames(m_currentFrame, m_sinceFrame);
      }
      */
      generateScrollBetweenFrames(m_currentFrame, m_sinceFrame);

      DrawScrollOverlay();

      // handle cursor positioning
      if (CursorMoveType == ALWAYS) {
        //setAbsoluteCursorPosition();
        if (m_currentFrame.pointable(favoritePointableId()).hand().isValid()) {
          setDeltaTrackedAbsoluteCursorPositionHand(m_currentFrame.pointable(favoritePointableId()).hand());
        } else {
          setDeltaTrackedAbsoluteCursorPositionPointable(m_currentFrame.pointable(favoritePointableId()));
        }
      }

      return true;
  }
  return false; // MUST return false if an event was not handled.
}

bool OutputPeripheralFingerMouse::State_FingerMouse_2Fingers_Zooming (StateMachineInput input) {
  switch (input) {
    case OMFM__LOST_FOCUS:
      FINGERMOUSE_TRANSITION_TO(State_FingerMouse_0Fingers_NoInteraction);
      return true;

    case SM_ENTER:
      m_drawOverlays = true;
      // generate begin-zoom event
      beginGesture(LPGesture::GestureZoom);
      applyZoom(static_cast<float>(1 + ZoomScaleFactor * (m_currentFrame.scaleFactor(m_gestureStart) - 1)));
      return true;

    case SM_EXIT:
      // generate end-zoom event
      endGesture();
      return true;

    case OMFM__PROCESS_FRAME:
      // handle finger-count changes
      if ((pointableCountIsUnambiguous() && pointableCount() == 0) || shouldBeInPalmSwipeMode()) {
        FingerMouse_TransitionTo_NFingers_NoInteraction();
        return true;
      }

      // handle zone changes
      if (m_noTouching) {
        if (collectiveZone() != Pointable::ZONE_TOUCHING) {
          FINGERMOUSE_TRANSITION_TO(State_FingerMouse_2Fingers_Hovering);
          return true;
        }
      }

      DrawZoomOverlay();

      // handle cursor positioning
      if (CursorMoveType == ALWAYS) {
        //setAbsoluteCursorPosition();
        if (m_currentFrame.pointable(favoritePointableId()).hand().isValid()) {
          setDeltaTrackedAbsoluteCursorPositionHand(m_currentFrame.pointable(favoritePointableId()).hand());
        } else {
          setDeltaTrackedAbsoluteCursorPositionPointable(m_currentFrame.pointable(favoritePointableId()));
        }
      }

      // apply zoom
      applyZoom(static_cast<float>(1 + ZoomScaleFactor * (m_currentFrame.scaleFactor(m_sinceFrame) - 1)));

      return true;
  }
  return false; // MUST return false if an event was not handled.
}

bool OutputPeripheralFingerMouse::State_FingerMouse_3PlusFingers_NoInteraction (StateMachineInput input) {
  switch (input) {
    case OMFM__LOST_FOCUS:
      FINGERMOUSE_TRANSITION_TO(State_FingerMouse_0Fingers_NoInteraction);
      return true;

    case SM_ENTER:
      m_drawOverlays = true;
      resetFavoritePointableId();
    case OMFM__PROCESS_FRAME:
      // handle finger-count changes
      if ((pointableCountIsUnambiguous() && pointableCount() < 3) || shouldBeInPalmSwipeMode()) {
        FingerMouse_TransitionTo_NFingers_NoInteraction();
        return true;
      }

      // handle zone changes
      if (collectiveZone() == Pointable::ZONE_HOVERING) {
        FINGERMOUSE_TRANSITION_TO(State_FingerMouse_3PlusFingers_Hovering);
        return true;
      } else if (collectiveZone() == Pointable::ZONE_TOUCHING) {
        FINGERMOUSE_TRANSITION_TO(State_FingerMouse_3PlusFingers_GestureRecognition);
        return true;
      }

      //DrawScrollOverlay();

      // handle cursor positioning
      if (CursorMoveType == ALWAYS) {
        //setAbsoluteCursorPosition();
        if (m_currentFrame.pointable(favoritePointableId()).hand().isValid()) {
          setDeltaTrackedAbsoluteCursorPositionHand(m_currentFrame.pointable(favoritePointableId()).hand());
        } else {
          setDeltaTrackedAbsoluteCursorPositionPointable(m_currentFrame.pointable(favoritePointableId()));
        }
      }

      return true;
  }
  return false; // MUST return false if an event was not handled.
}

bool OutputPeripheralFingerMouse::State_FingerMouse_3PlusFingers_Hovering (StateMachineInput input) {
  switch (input) {
    case OMFM__LOST_FOCUS:
      FINGERMOUSE_TRANSITION_TO(State_FingerMouse_0Fingers_NoInteraction);
      return true;

    case OMFM__PROCESS_FRAME:
      // handle finger-count changes
      if ((pointableCountIsUnambiguous() && pointableCount() < 3) || shouldBeInPalmSwipeMode()) {
        FingerMouse_TransitionTo_NFingers_NoInteraction();
        return true;
      }

      // handle zone changes
      if (collectiveZone() == Pointable::ZONE_NONE) {
        FINGERMOUSE_TRANSITION_TO(State_FingerMouse_3PlusFingers_NoInteraction);
        return true;
      } else if (collectiveZone() == Pointable::ZONE_TOUCHING) {
        FINGERMOUSE_TRANSITION_TO(State_FingerMouse_3PlusFingers_GestureRecognition);
        return true;
      }

      DrawDesktopSwipeOverlay();

      // handle cursor positioning
      if (CursorMoveType == ALWAYS || CursorMoveType == ANY_HOVER) {
        //setAbsoluteCursorPosition();
        if (m_currentFrame.pointable(favoritePointableId()).hand().isValid()) {
          setDeltaTrackedAbsoluteCursorPositionHand(m_currentFrame.pointable(favoritePointableId()).hand());
        } else {
          setDeltaTrackedAbsoluteCursorPositionPointable(m_currentFrame.pointable(favoritePointableId()));
        }
      }

      return true;
  }
  return false; // MUST return false if an event was not handled.
}

bool OutputPeripheralFingerMouse::State_FingerMouse_3PlusFingers_GestureRecognition (StateMachineInput input) {
  switch (input) {
    case OMFM__LOST_FOCUS:
      FINGERMOUSE_TRANSITION_TO(State_FingerMouse_0Fingers_NoInteraction);
      return true;

    case SM_ENTER:
      m_gestureStart = m_currentFrame;
    case OMFM__PROCESS_FRAME: {
      // handle finger-count changes
      if ((pointableCountIsUnambiguous() && pointableCount() < 3) || shouldBeInPalmSwipeMode()) {
        FingerMouse_TransitionTo_NFingers_NoInteraction();
        return true;
      }

      // handle zone changes
      if (collectiveZone() == Pointable::ZONE_NONE) {
        FINGERMOUSE_TRANSITION_TO(State_FingerMouse_3PlusFingers_NoInteraction);
        return true;
      } else if (collectiveZone() == Pointable::ZONE_HOVERING) {
        FINGERMOUSE_TRANSITION_TO(State_FingerMouse_3PlusFingers_Hovering);
        return true;
      }

      DrawDesktopSwipeOverlay(false);

      // handle cursor positioning
      if (CursorMoveType == ALWAYS) {
        //setAbsoluteCursorPosition();
        if (m_currentFrame.pointable(favoritePointableId()).hand().isValid()) {
          setDeltaTrackedAbsoluteCursorPositionHand(m_currentFrame.pointable(favoritePointableId()).hand());
        } else {
          setDeltaTrackedAbsoluteCursorPositionPointable(m_currentFrame.pointable(favoritePointableId()));
        }
      }

      // if translation "wins", transition to the corresponding state
      if (m_currentFrame.timestamp() - m_gestureStart.timestamp() >= 10*MILLISECONDS && rtsIsUnambiguous() && rts() == TRANSLATING) {

        Vector translation = m_currentFrame.translation(m_gestureStart);
        bool horizontal_wins = false;
        bool vertical_wins = false;
        if (std::abs(translation.x) > 2*std::abs(translation.y) + 1) {
          horizontal_wins = true;
        } else if (std::abs(translation.y) > 2*std::abs(translation.x) + 1) {
          vertical_wins = true;
        }

        if (horizontal_wins) {
          FINGERMOUSE_TRANSITION_TO(State_FingerMouse_3PlusFingers_SwipeHorizontal);
        } else if (vertical_wins) {
          FINGERMOUSE_TRANSITION_TO(State_FingerMouse_3PlusFingers_SwipeVertical);
        }
        return true;
      }
      // if translation didn't "win", it's ok, it may just take more frames before it happens.

      return true;
    }
  }
  return false; // MUST return false if an event was not handled.
}

bool OutputPeripheralFingerMouse::State_FingerMouse_3PlusFingers_SwipeVertical (StateMachineInput input) {
  switch (input) {
    case OMFM__LOST_FOCUS:
      FINGERMOUSE_TRANSITION_TO(State_FingerMouse_0Fingers_NoInteraction);
      return true;

    case SM_ENTER:
      m_drawOverlays = true;
      // generate begin-scroll event
      beginGesture(LPGesture::GestureDesktopSwipeVertical);
      return true;

    case SM_EXIT:
      // generate end-scroll event
      endGesture();
      return true;

    case OMFM__PROCESS_FRAME:
      // handle finger-count changes
      if ((pointableCountIsUnambiguous() && pointableCount() == 0) || shouldBeInPalmSwipeMode()) {
        FingerMouse_TransitionTo_NFingers_NoInteraction();
        return true;
      }

      // handle zone changes
      if (m_noTouching) {
        Vector translation = m_currentFrame.translation(m_sinceFrame);
        bool horizontal_wins = false;
        if (std::abs(translation.x) > 2*std::abs(translation.y) + 1) {
          horizontal_wins = true;
        }

        if (collectiveZone() == Pointable::ZONE_NONE || collectiveZone() == Pointable::ZONE_HOVERING) {
          FINGERMOUSE_TRANSITION_TO(State_FingerMouse_3PlusFingers_NoInteraction);
          return true;
        } else if (horizontal_wins) {
          FINGERMOUSE_TRANSITION_TO(State_FingerMouse_3PlusFingers_SwipeHorizontal);
        }
      }

      // apply desktop swipe
      generateDesktopSwipeBetweenFrames(m_currentFrame, m_sinceFrame);

      DrawDesktopSwipeOverlay(false);

      // handle cursor positioning
      if (CursorMoveType == ALWAYS) {
        //setAbsoluteCursorPosition();
      }

      return true;
  }
  return false; // MUST return false if an event was not handled.
}

bool OutputPeripheralFingerMouse::State_FingerMouse_3PlusFingers_SwipeHorizontal (StateMachineInput input) {
  switch (input) {
    case OMFM__LOST_FOCUS:
      FINGERMOUSE_TRANSITION_TO(State_FingerMouse_0Fingers_NoInteraction);
      return true;

    case SM_ENTER:
//      if (m_mountainLionOrNewer) {
//        m_drawOverlays = false;
//      }
      // generate begin-scroll event
      beginGesture(LPGesture::GestureDesktopSwipeHorizontal);
      return true;

    case SM_EXIT:
      // generate end-scroll event
      endGesture();
      return true;

    case OMFM__PROCESS_FRAME:
      // handle finger-count changes
      if ((pointableCountIsUnambiguous() && pointableCount() == 0) || shouldBeInPalmSwipeMode()) {
        FingerMouse_TransitionTo_NFingers_NoInteraction();
        return true;
      }

      // handle zone changes
      if (m_noTouching) {
        if (collectiveZone() == Pointable::ZONE_NONE || collectiveZone() == Pointable::ZONE_HOVERING) {
          FINGERMOUSE_TRANSITION_TO(State_FingerMouse_3PlusFingers_NoInteraction);
          return true;
        }
      }

      // apply desktop swipe
      generateDesktopSwipeBetweenFrames(m_currentFrame, m_sinceFrame);

      DrawDesktopSwipeOverlay();

      // handle cursor positioning
      if (CursorMoveType == ALWAYS) {
        //setAbsoluteCursorPosition();
      }

      return true;
  }
  return false; // MUST return false if an event was not handled.
}

bool OutputPeripheralFingerMouse::State_FingerMouse_Palm_GestureRecognition (StateMachineInput input) {
  switch (input) {
    case OMFM__LOST_FOCUS:
      FINGERMOUSE_TRANSITION_TO(State_FingerMouse_0Fingers_NoInteraction);
      return true;

    case SM_ENTER:
      m_gestureStart = m_currentFrame;
    case OMFM__PROCESS_FRAME: {
      // handle state changes
      if (!shouldBeInPalmSwipeMode()) {
        FingerMouse_TransitionTo_NFingers_NoInteraction();
        return true;
      }

      //Find a hand
      Hand hand;
      Pointable favorite_pointable = m_currentFrame.pointable(foremostPointableId());
      if (!favorite_pointable.isValid()) {
        if (m_currentFrame.hands().count() == 0) {
          FingerMouse_TransitionTo_NFingers_NoInteraction();
          return true;
        } else {
          hand = m_currentFrame.hands()[0];
        }
      } else {
        hand = favorite_pointable.hand();
      }
      if (!hand.isValid()) {
        FingerMouse_TransitionTo_NFingers_NoInteraction();
        return true;
      }

      DrawVerticalPalmOverlay();

      // reccognize the gesture
      if (m_currentFrame.timestamp() - m_gestureStart.timestamp() >= 100*MILLISECONDS
          && rtsIsUnambiguous() && rts() == TRANSLATING
          && hand.palmPosition().z <= 150) {
        FINGERMOUSE_TRANSITION_TO(State_FingerMouse_Palm_Swipe);
      }
      return true;
    }
    case SM_EXIT:
      return true;
  }
  return false; // MUST return false if an event was not handled.
}

bool OutputPeripheralFingerMouse::State_FingerMouse_Palm_Swipe (StateMachineInput input) {
  switch (input) {
    case OMFM__LOST_FOCUS:
      FINGERMOUSE_TRANSITION_TO(State_FingerMouse_0Fingers_NoInteraction);
      return true;

    case SM_ENTER:
//      if (m_mountainLionOrNewer) {
//        m_drawOverlays = false;
//      }
      beginGesture(LPGesture::GestureDesktopSwipeHorizontal);
      return true;

    case SM_EXIT:
      endGesture();
      return true;

    case OMFM__PROCESS_FRAME:
      // handle state changes
      if (!shouldBeInPalmSwipeMode()) {
        FingerMouse_TransitionTo_NFingers_NoInteraction();
        return true;
      }

      DrawVerticalPalmOverlay();

      // apply desktop swipe
      generateDesktopSwipeBetweenFrames(m_currentFrame, m_sinceFrame);

      return true;
  }
  return false; // MUST return false if an event was not handled.
}

// end of OUTPUT_MODE_FINGER_MOUSE state handlers
/////////////////////////////////////////////////
#undef FINGERMOUSE_TRANSITION_TO
}
