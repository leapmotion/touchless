/*==================================================================================================================

 Copyright (c) 2010 - 2013 Leap Motion. All rights reserved.

 The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
 protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
 strictly forbidden unless prior written permission is obtained from Leap Motion.

 ===================================================================================================================*/
#include "LPGesture.h"
#include "OutputPeripheralBasic.h"
#include <algorithm>

#if __APPLE__
#include <sys/sysctl.h>
#endif

namespace Touchless {

#if _WIN32
const bool OutputPeripheralBasic::UseMouseOutput = false;
#else
const bool OutputPeripheralBasic::UseMouseOutput = true;
#endif
bool OutputPeripheralBasic::DisableClicking = false;
bool OutputPeripheralBasic::DisablePalmSwipe = true;
bool OutputPeripheralBasic::DisableAllOverlays = false;
bool OutputPeripheralBasic::DisableHorizontalScrolling = true;
float OutputPeripheralBasic::TranslationScaleFactor = 1.5;
float OutputPeripheralBasic::ZoomScaleFactor = 1.5;

// GENERAL OVERVIEW OF BASIC MODE STATE MACHINE
//
// There are five groups of states:
// - 0 pointables
// - 1 pointable
// - 2+ pointables
// - 2 hands
// - sideways palm
//
// In each state group, there are states which correspond to the "hover" and "touching"
// zones (see Pointable::Zone).
//
// The states correspond roughly exactly with the state of interaction.

OutputPeripheralBasic::OutputPeripheralBasic(OSInteractionDriver& osInteractionDriver, OverlayDriver& overlayDriver)
  :
  GestureInteractionManager(osInteractionDriver, overlayDriver),
  m_drawOverlays(true),
  m_noTouching(true),
  m_clickDuration(0),
  m_clickNumber(0),
  m_clickDown(false),
  m_lastClickTime(0),
  m_lastNumIcons(0),
  m_charmsMode(-1),
  m_updateCursor(true)
{
  m_basicModeStateMachine.SetOwnerClass(this, "OutputPeripheralBasic");

  // This tells the state machine to log transitions to cerr.  An optional second can be used to specify
  // an event ID to ignore (such as something that happens every single frame) to avoid spamming the console.
  // Specify that OMFM__PROCESS_FRAME should be ignored.
  //m_basicModeStateMachine.SetTransitionLogger(&std::cerr, OMB__PROCESS_FRAME);
}

OutputPeripheralBasic::~OutputPeripheralBasic() {
  m_basicModeStateMachine.Shutdown();
#if __APPLE__
  flushOverlay();
#endif
}

void OutputPeripheralBasic::SetIntroMode(){
  OutputPeripheralBasic::DisableClicking = true;
  OutputPeripheralBasic::DisablePalmSwipe = false;
}

void OutputPeripheralBasic::SetBasicMode(){
  OutputPeripheralBasic::DisableClicking = false;
  OutputPeripheralBasic::DisablePalmSwipe = true;
}

void OutputPeripheralBasic::processFrameInternal() {
  m_noTouching = std::count_if(m_relevantPointables.begin(), m_relevantPointables.end(),
                          [this] (const Pointable& pointable) {return pointable.touchZone() == Pointable::ZONE_TOUCHING;}) == 0;
  // initialize basic mode state machine if necessary
  if (!m_basicModeStateMachine.IsInitialized()) {
    m_basicModeStateMachine.Initialize(&OutputPeripheralBasic::State_BasicMode_0Pointables_NoInteraction,
                                       "State_BasicMode_0Pointables_NoInteraction");
  }

#if __APPLE__
  // clear overlays
  if (m_flushOverlay) {
    for (int i = 0; i < m_numOverlayImages; ++i) {
      setIconVisibility(i, false);
    }
    m_lastNumIcons = 0;
  }
#else
  Vector screenPosition, clampVec;
  for (int i = m_lastNumIcons; i < m_numOverlayImages; ++i) {
    setIconVisibility(i, false);
  }
  m_lastNumIcons = 0;
#endif

  // run the state machine with the "process frame" input
  m_basicModeStateMachine.RunCurrentState(OMB__PROCESS_FRAME);

#if __APPLE__
  flushOverlay();
#endif

  if (!UseMouseOutput) {
    emitTouchEvent();
  }
}

void OutputPeripheralBasic::stopActiveEvents()
{
  m_basicModeStateMachine.RunCurrentState(OMB__LOST_FOCUS);
}

void OutputPeripheralBasic::setAbsoluteCursorPositionHand (Vector *calculatedScreenPosition) {
  if (!m_updateCursor) {
    return;
  }
  Hand favoriteHand = m_currentFrame.hand(m_favoriteHandId);
  // call the baseclass version
  GestureInteractionManager::setAbsoluteCursorPositionHand(favoriteHand, calculatedScreenPosition);
}

void OutputPeripheralBasic::setAbsoluteCursorPositionPointable (Vector *calculatedScreenPosition) {
  if (!m_updateCursor) {
    return;
  }
  Pointable favoritePointable = m_currentFrame.pointable(favoritePointableId());
  // call the baseclass version
  GestureInteractionManager::setAbsoluteCursorPositionPointable(favoritePointable, calculatedScreenPosition);
}

void OutputPeripheralBasic::generateScrollBetweenFrames (const Frame &currentFrame, const Frame &sinceFrame) {
  float scroll_dx, scroll_dy;

  // damp out small motions
  Vector translation = currentFrame.translation(sinceFrame);
  float timeDelta = float(currentFrame.timestamp() - sinceFrame.timestamp()) / SECONDS;
  translation *= scrollDampingFactor(translation / timeDelta);

  if (DisableHorizontalScrolling) {
    scroll_dx = 0.0f;
  } else {
    scroll_dx = static_cast<float>(TranslationScaleFactor * translation.x);
  }
  scroll_dy = static_cast<float>(-TranslationScaleFactor * translation.y);

  applyScroll(scroll_dx, scroll_dy, currentFrame.timestamp() - sinceFrame.timestamp());
}

void OutputPeripheralBasic::generateDesktopSwipeBetweenFrames (const Frame &currentFrame, const Frame &sinceFrame) {
  float scroll_dx, scroll_dy;

  // damp out small motions
  Vector translation = currentFrame.translation(sinceFrame);
  float timeDelta = float(currentFrame.timestamp() - sinceFrame.timestamp()) / SECONDS;
  translation *= scrollDampingFactor(translation / timeDelta);

  scroll_dx = static_cast<float>(translation.x);
  scroll_dy = static_cast<float>(-translation.y);

  applyDesktopSwipe(scroll_dx, scroll_dy);
}

bool OutputPeripheralBasic::shouldBeInMultiHandMode(int32_t *pointableId1, int32_t *pointableId2) {
  std::list<Hand> handList;
  if (m_currentFrame.hands().count() > 1) {
    for (int i = 0; i < m_currentFrame.hands().count(); ++i) {
      if (m_currentFrame.hands()[i].pointables().count() != 0 && m_currentFrame.hands()[i].direction().z < 0) {
        handList.push_back(m_currentFrame.hands()[i]);
      }
    }
    if (handList.size() > 1)
    {
      auto it = handList.begin();
      if (pointableId1 != nullptr) {
        *pointableId1 = foremostPointableIdOfHand(*it);
      }
      if (pointableId2 != nullptr) {
        *pointableId2 = foremostPointableIdOfHand(*(++it));
      }
      return true;
    }
  }
  return false;
}

bool OutputPeripheralBasic::shouldBeInPalmSwipeMode () const {
  static const double COS_X_AXIS_ANGLE_THRESHOLD = 0.75; // the cosine of the angle to the X axis
  return !DisablePalmSwipe && !m_currentFrame.hands().isEmpty()
  && std::abs(m_currentFrame.hands()[0].palmNormal().x) >= COS_X_AXIS_ANGLE_THRESHOLD
  && m_currentFrame.hands()[0].palmPosition().z <= 150;
}

bool OutputPeripheralBasic::hasFingersTouching(const Hand& hand) const {
  if (!hand.isValid()) {
    return false;
  }
  PointableList pointables = hand.pointables();
  for (int i=0; i<pointables.count(); i++) {
    if (pointables[i].touchZone() == Pointable::ZONE_TOUCHING) {
      return true;
    }
  }
  return false;
}

// This macro is part of the state machine -- used for convenience, to avoid
// having to type such a long and ugly statement.
#define BASICMODE_TRANSITION_TO(x) m_basicModeStateMachine.SetNextState(&OutputPeripheralBasic::x, #x)

void OutputPeripheralBasic::BasicMode_TransitionTo_NPointables_NoInteraction () {
  m_lastStateChangeTime = m_currentFrame.timestamp();
  if (shouldBeInMultiHandMode()) {
    BASICMODE_TRANSITION_TO(State_BasicMode_2Hands_Hovering);
  } else if (shouldBeInPalmSwipeMode()) {
    BASICMODE_TRANSITION_TO(State_BasicMode_Palm_GestureRecognition);
  } else {
    size_t pointable_count = 0;
    if (pointableCountIsUnambiguous()) {
      pointable_count = pointableCount();
    }
    switch (pointable_count) {
      case 0:
        BASICMODE_TRANSITION_TO(State_BasicMode_0Pointables_NoInteraction);
        break;

      case 1:
        if (DisableClicking) {
          BASICMODE_TRANSITION_TO(State_BasicMode_2PlusPointables_Hovering);
        } else {
          BASICMODE_TRANSITION_TO(State_BasicMode_1Pointable_Hovering);
        }
        break;

      default: // 2 or higher
        BASICMODE_TRANSITION_TO(State_BasicMode_2PlusPointables_Hovering);
        break;
    }
  }
}

// ////////////////////////////////////////////////////
// beginning of OUTPUT_MODE_BASIC state handlers

bool OutputPeripheralBasic::State_BasicMode_0Pointables_NoInteraction (StateMachineInput input) {
  switch (input) {
    case SM_ENTER:
      m_updateCursor = true;
      m_drawOverlays = true;
    case OMB__PROCESS_FRAME:
      // handle state group changes
      if ((pointableCountIsUnambiguous() && pointableCount() > 0) || shouldBeInMultiHandMode() || shouldBeInPalmSwipeMode()) {
        BasicMode_TransitionTo_NPointables_NoInteraction();
        return true;
      }

      for (int i = 0; i < m_numOverlayImages; ++i) {
        setIconVisibility(i, false);
      }

      m_positionalDeltaTracker.clear();

      m_charmsMode = -1;

      // nothing else to handle, no overlays to draw
      return true;
    case OMB__LOST_FOCUS:
      return true;
  }
  return false; // MUST return false if an event was not handled.
}

bool OutputPeripheralBasic::State_BasicMode_1Pointable_Hovering (StateMachineInput input) {
  switch (input) {
    case OMB__LOST_FOCUS:
      BASICMODE_TRANSITION_TO(State_BasicMode_0Pointables_NoInteraction);
      return true;
    case SM_ENTER:
      m_updateCursor = true;
      m_drawOverlays = true;
      resetFavoritePointableId();
    case OMB__PROCESS_FRAME:
      Pointable favoritePointable = m_currentFrame.pointable(favoritePointableId());

      // handle state group changes
      if ((pointableCountIsUnambiguous() && pointableCount() != 1) || shouldBeInMultiHandMode() || shouldBeInPalmSwipeMode()) {
        BasicMode_TransitionTo_NPointables_NoInteraction();
        return true;
      }

      // handle zone changes
      if (collectiveZone() == Pointable::ZONE_TOUCHING) {
        if (UseMouseOutput) {
          setAbsoluteCursorPositionPointable(&m_clickDownScreenPosition);
          // reset click number if we're too far away (in time or space) from the last click
          if (m_currentFrame.timestamp() - m_lastClickTime > 3*SECONDS
              || m_clickDownScreenPosition.distanceTo(m_lastClickLocation) > 50 // 50 pixels (?)
              || m_clickNumber == 3) {
            m_clickNumber = 0;
          }

          clickDown(0, ++m_clickNumber);
          m_clickDown = true;
          m_lastClickLocation = m_clickDownScreenPosition;
        } else {
          m_clickPointable = m_sinceFrame.pointable(favoritePointableId());
          addTouchPointForPointable(favoritePointableId(), m_clickPointable, true);
        }
        m_lastClickTime = m_currentFrame.timestamp();
        BASICMODE_TRANSITION_TO(State_BasicMode_1Pointable_ClickCooldown);
        return true;
      }

      if (UseMouseOutput) {
        // handle cursor positioning
        setAbsoluteCursorPositionPointable();
      } else {
        addTouchPointForPointable(favoritePointableId(), favoritePointable, false);
      }

      if (touchVersion() == 8 && useCharmHelper()) {
        Vector aspectNormalized, clampVec;
        if (favoritePointable.isValid()) {
          m_positionalDeltaTracker.setPositionToStabilizedPositionOf(favoritePointable);
        }
        Vector normPos = interactionBox().normalizePoint(m_positionalDeltaTracker.getTrackedPosition(), false);
        normalizedToAspect(normPos, aspectNormalized, clampVec, 1.0f, false);
        applyCharms(aspectNormalized, static_cast<int>(m_relevantPointables.size()), m_charmsMode);
      }

      float alphaMult = alphaFromTimeVisible((1.0f/SECONDS)*static_cast<float>(m_currentFrame.timestamp() - m_lastStateChangeTime));
      drawOverlayForPointable(favoritePointable, 0, alphaMult);
      m_lastNumIcons++;

      return true;
  }
  return false; // MUST return false if an event was not handled.
}

bool OutputPeripheralBasic::State_BasicMode_1Pointable_ClickCooldown (StateMachineInput input) {
  switch (input) {
    case OMB__LOST_FOCUS:
      BASICMODE_TRANSITION_TO(State_BasicMode_0Pointables_NoInteraction);
      return true;

    case SM_EXIT:
      if (m_clickDown) {
        if (UseMouseOutput) {
          // generate up-click event
          m_clickDown = false;
          clickUp(0, m_clickNumber);
        } else {
          addTouchPointForPointable(favoritePointableId(), m_clickPointable, false);
        }
      }
      m_clickDuration = 0; // reset the click duration to sentinel value
      return true;
    case SM_ENTER:
    case OMB__PROCESS_FRAME:
      // handle state group changes
      if ((pointableCountIsUnambiguous() && pointableCount() != 1) || shouldBeInMultiHandMode() || shouldBeInPalmSwipeMode()) {
        BasicMode_TransitionTo_NPointables_NoInteraction();
        return true;
      }

      // handle zone changes
      if (collectiveZone() != Pointable::ZONE_TOUCHING) {
        BASICMODE_TRANSITION_TO(State_BasicMode_1Pointable_Hovering);
        return true;
      }

      m_clickDuration = m_currentFrame.timestamp() - m_lastClickTime;

      Pointable favoritePointable = m_currentFrame.pointable(favoritePointableId());

      if (m_clickDuration > 150*MILLISECONDS) {
        if (m_clickDown) {
          m_clickDown = false;
          if (UseMouseOutput) {
            clickUp(0, m_clickNumber); // just do button 0 for now.
          } else {
            addTouchPointForPointable(favoritePointableId(), m_clickPointable, false);
          }
        }
        if (UseMouseOutput) {
          setAbsoluteCursorPositionPointable();
        }
      } else if (!UseMouseOutput) {
        addTouchPointForPointable(favoritePointableId(), m_clickPointable, true);
      }

      float alphaMult = alphaFromTimeVisible((1.0f/SECONDS)*static_cast<float>(m_currentFrame.timestamp() - m_lastStateChangeTime));
      drawOverlayForPointable(favoritePointable, 0, alphaMult);
      m_lastNumIcons++;

      return true;
  }
  return false; // MUST return false if an event was not handled.
}

bool OutputPeripheralBasic::State_BasicMode_2PlusPointables_Hovering (StateMachineInput input) {
  switch (input) {
    case OMB__LOST_FOCUS:
      BASICMODE_TRANSITION_TO(State_BasicMode_0Pointables_NoInteraction);
      return true;

    case SM_ENTER: {
      m_drawOverlays = true;
      resetFavoritePointableId();
      m_favoriteHandId = -1;
      if (favoritePointableId() == -1) {
        return true;
      }
      if (UseMouseOutput) {
        m_lastStateChangeTime = m_currentFrame.timestamp();
      }
    }
    case OMB__PROCESS_FRAME:
      // handle state group changes
      if ((pointableCountIsUnambiguous() && ((DisableClicking && pointableCount() == 0) || (!DisableClicking && pointableCount() < 2)))
          || shouldBeInMultiHandMode() || shouldBeInPalmSwipeMode()) {
        BasicMode_TransitionTo_NPointables_NoInteraction();
        return true;
      }

      // see if hand has come into view
      Pointable favoritePointable = m_currentFrame.pointable(favoritePointableId());
      if (m_favoriteHandId < 0) {
        // handle pointable going out of scope
        if (!favoritePointable.isValid()) {
          return true;
        }

        m_favoriteHandId = m_currentFrame.pointable(favoritePointableId()).hand().id();
      }

      // if no hand in view, just draw normal overlay
      if (m_favoriteHandId < 0) {
        if (!UseMouseOutput) {
          addTouchPointForPointable(favoritePointableId(), favoritePointable, false);
        }

        float alphaMult = alphaFromTimeVisible((1.0f/SECONDS)*static_cast<float>(m_currentFrame.timestamp() - m_lastStateChangeTime));
        drawOverlayForPointable(favoritePointable, 0, alphaMult);
      } else {
        // handle hand going out of scope
        Hand favoriteHand = m_currentFrame.hand(m_favoriteHandId);
        if (!favoriteHand.isValid()) {
          BasicMode_TransitionTo_NPointables_NoInteraction();
          return true;
        }

        // handle zone changes
        if (hasFingersTouching(favoriteHand)) {
          BASICMODE_TRANSITION_TO(State_BasicMode_2PlusPointables_Scrolling);
          return true;
        }

        if (!UseMouseOutput) {
          addTouchPointForHand(favoriteHand, false);
        }

        if (favoriteHand.isValid()) {
          m_positionalDeltaTracker.setPositionToStabilizedPositionOf(favoriteHand);
        }
        drawScrollOverlayForHand(favoriteHand);
      }
      m_lastNumIcons += 5;

      return true;
  }
  return false; // MUST return false if an event was not handled.
}

bool OutputPeripheralBasic::State_BasicMode_2PlusPointables_Scrolling (StateMachineInput input) {
  switch (input) {
    case OMB__LOST_FOCUS:
      BASICMODE_TRANSITION_TO(State_BasicMode_0Pointables_NoInteraction);
      return true;

    case SM_ENTER: {
      m_drawOverlays = true;
      m_scrollDrawCount = 0;
      if (UseMouseOutput) {
        m_lastStateChangeTime = m_currentFrame.timestamp();
      }

      //Get the favorite pointable and hand
      Hand favoriteHand = m_currentFrame.hand(m_favoriteHandId);
      assert(favoriteHand.isValid());

      if (UseMouseOutput) {
        setAbsoluteCursorPositionHand();
        beginGesture(LPGesture::GestureScroll);
      } else {
        addTouchPointForHand(favoriteHand, true);
      }
      if (m_updateCursor) {
        m_scrollStartPosition = m_positionalDeltaTracker.getTrackedPosition();
      }
//      m_updateCursor = false;

      return true;
    }
    case SM_EXIT:
      if (UseMouseOutput) {
        // generate end-scroll event
        endGesture();
      } else {
        Hand favoriteHand = m_currentFrame.hand(m_favoriteHandId);
        assert(favoriteHand.isValid());
        addTouchPointForHand(favoriteHand, false);
      }
      m_flushOverlay = true;

      return true;

    case OMB__PROCESS_FRAME:
      // handle state group changes
      if ((pointableCountIsUnambiguous() && pointableCount() == 0) || shouldBeInMultiHandMode() || shouldBeInPalmSwipeMode()) {
        BasicMode_TransitionTo_NPointables_NoInteraction();
        return true;
      }

      // hand hand going out of scope
      Hand favoriteHand = m_currentFrame.hand(m_favoriteHandId);
      if (!favoriteHand.isValid()) {
        BasicMode_TransitionTo_NPointables_NoInteraction();
        return true;
      }

      // handle zone changes
      if (!hasFingersTouching(favoriteHand)) {
        BasicMode_TransitionTo_NPointables_NoInteraction();
        return true;
      }

      if (UseMouseOutput) {
        generateScrollBetweenFrames(m_currentFrame, m_sinceFrame);
      } else {
        addTouchPointForHand(favoriteHand, true);
      }

      if (favoriteHand.isValid()) {
        m_positionalDeltaTracker.setPositionToStabilizedPositionOf(favoriteHand);
      }
      if (UseMouseOutput) {
        float alphaMult = 1.0f - 0.4f*alphaFromTimeVisible((1.0f/SECONDS)*static_cast<float>(m_currentFrame.timestamp() - m_lastStateChangeTime));
        drawGestureOverlayForHand(favoriteHand, 0.0f, 1.0f, alphaMult, nullptr, false, false, true, false);
      } else {
        float alphaMult = alphaFromTimeVisible((1.0f/SECONDS)*static_cast<float>(m_currentFrame.timestamp() - m_lastStateChangeTime));
        drawGestureOverlayForHand(favoriteHand, 0.0f, 1.0f, alphaMult);
      }
      m_lastNumIcons += 5;

      return true;
  }
  return false; // MUST return false if an event was not handled.
}

bool OutputPeripheralBasic::State_BasicMode_2Hands_Hovering (StateMachineInput input) {
  int32_t pointable1, pointable2;
  switch (input) {
    case OMB__LOST_FOCUS:
      BASICMODE_TRANSITION_TO(State_BasicMode_0Pointables_NoInteraction);
      return true;

    case SM_ENTER:
      m_updateCursor = true;
      m_drawOverlays = true;
      resetFavoritePointableId();
    case OMB__PROCESS_FRAME:
      // handle state group changes
      if (!shouldBeInMultiHandMode(&pointable1, &pointable2)) {
        BasicMode_TransitionTo_NPointables_NoInteraction();
        return true;
      }

      // handle zone changes
      if (collectiveZone() == Pointable::ZONE_TOUCHING) {
        if (UseMouseOutput) {
          BASICMODE_TRANSITION_TO(State_BasicMode_2Hands_GestureRecognition);
        } else {
          BASICMODE_TRANSITION_TO(State_BasicMode_2Hands_Touch);
        }
        return true;
      }

      if (UseMouseOutput) {
        // handle cursor positioning
        GestureInteractionManager::setAbsoluteCursorPositionPointable(m_currentFrame.pointable(pointable1));
      } else {
        addTouchPointForPointable(pointable1, m_currentFrame.pointable(pointable1), false, false);
        addTouchPointForPointable(pointable2, m_currentFrame.pointable(pointable2), false, false);
      }

      float alphaMult = alphaFromTimeVisible((1.0f/SECONDS)*static_cast<float>(m_currentFrame.timestamp() - m_lastStateChangeTime));
      drawOverlayForPointable(m_currentFrame.pointable(pointable1), 0, alphaMult, false);
      m_lastNumIcons++;
      drawOverlayForPointable(m_currentFrame.pointable(pointable2), 1, alphaMult, false);
      m_lastNumIcons++;

      return true;
  }
  return false; // MUST return false if an event was not handled.
}

// Mouse mode only
bool OutputPeripheralBasic::State_BasicMode_2Hands_GestureRecognition (StateMachineInput input) {
  int32_t pointable1, pointable2;
  switch (input) {
    case OMB__LOST_FOCUS:
      BASICMODE_TRANSITION_TO(State_BasicMode_0Pointables_NoInteraction);
      return true;

    case SM_ENTER:
      m_updateCursor = true;
      m_drawOverlays = true;
      m_gestureStart = m_currentFrame;
    case OMB__PROCESS_FRAME: {
      // handle state group changes
      if (!shouldBeInMultiHandMode(&pointable1, &pointable2)) {
        BasicMode_TransitionTo_NPointables_NoInteraction();
        return true;
      }

      // handle zone changes
      if (m_noTouching) {
        if (collectiveZone() != Pointable::ZONE_TOUCHING) {
          BASICMODE_TRANSITION_TO(State_BasicMode_2Hands_Hovering);
          return true;
        }
      }

      if (m_currentFrame.timestamp() - m_gestureStart.timestamp() >= 20*MILLISECONDS && rtsIsUnambiguous()) {
        if (rts() == ROTATING) {
          BASICMODE_TRANSITION_TO(State_BasicMode_2Hands_Rotating);
          return true;
        } else if (rts() == SCALING) {
          BASICMODE_TRANSITION_TO(State_BasicMode_2Hands_Zooming);
          return true;
        }
      }

      float alphaMult = alphaFromTimeVisible((1.0f/SECONDS)*static_cast<float>(m_currentFrame.timestamp() - m_lastStateChangeTime));
      drawOverlayForPointable(m_currentFrame.pointable(pointable1), 0, alphaMult, false);
      m_lastNumIcons++;
      drawOverlayForPointable(m_currentFrame.pointable(pointable2), 1, alphaMult, false);
      m_lastNumIcons++;

      // if no gesture "won", it's ok, it may just take more frames before it's clear.
      return true;
    }
  }
  return false; // MUST return false if an event was not handled.
}

// Mouse mode only
bool OutputPeripheralBasic::State_BasicMode_2Hands_Rotating (StateMachineInput input) {
  int32_t pointable1, pointable2;
  switch (input) {
    case OMB__LOST_FOCUS:
      BASICMODE_TRANSITION_TO(State_BasicMode_0Pointables_NoInteraction);
      return true;

    case SM_ENTER:
      m_updateCursor = true;
      m_drawOverlays = true;
      // generate begin-rotate event
      beginGesture(LPGesture::GestureRotate);
      applyRotation(-Leap::RAD_TO_DEG*m_currentFrame.rotationAngle(m_gestureStart, Vector::zAxis()));
      return true;

    case SM_EXIT:
      // generate end-rotate event
      endGesture();
      return true;

    case OMB__PROCESS_FRAME:
      // handle state group changes
      if (!shouldBeInMultiHandMode(&pointable1, &pointable2)) {
        BasicMode_TransitionTo_NPointables_NoInteraction();
        return true;
      }

      // handle zone changes
      if (m_noTouching) {
        if (collectiveZone() != Pointable::ZONE_TOUCHING) {
          BASICMODE_TRANSITION_TO(State_BasicMode_2Hands_Hovering);
          return true;
        }
      }

      // apply rotation
      applyRotation(-Leap::RAD_TO_DEG*m_currentFrame.rotationAngle(m_sinceFrame, Vector::zAxis()));

      float alphaMult = alphaFromTimeVisible((1.0f/SECONDS)*static_cast<float>(m_currentFrame.timestamp() - m_lastStateChangeTime));
      drawOverlayForPointable(m_currentFrame.pointable(pointable1), 0, alphaMult, false);
      m_lastNumIcons++;
      drawOverlayForPointable(m_currentFrame.pointable(pointable2), 1, alphaMult, false);
      m_lastNumIcons++;

      return true;
  }
  return false; // MUST return false if an event was not handled.
}

// Mouse mode only
bool OutputPeripheralBasic::State_BasicMode_2Hands_Zooming (StateMachineInput input) {
  int32_t pointable1, pointable2;
  switch (input) {
    case OMB__LOST_FOCUS:
      BASICMODE_TRANSITION_TO(State_BasicMode_0Pointables_NoInteraction);
      return true;

    case SM_ENTER:
      m_updateCursor = true;
      m_drawOverlays = true;
      // generate begin-zoom event
      beginGesture(LPGesture::GestureZoom);
      applyZoom(static_cast<float>(1 + ZoomScaleFactor * (m_currentFrame.scaleFactor(m_gestureStart) - 1)));
      return true;

    case SM_EXIT:
      // generate end-zoom event
      endGesture();
      return true;

    case OMB__PROCESS_FRAME:
      // handle state group changes
      if (!shouldBeInMultiHandMode(&pointable1, &pointable2)) {
        BasicMode_TransitionTo_NPointables_NoInteraction();
        return true;
      }

      // handle zone changes
      if (m_noTouching) {
        if (collectiveZone() != Pointable::ZONE_TOUCHING) {
          BASICMODE_TRANSITION_TO(State_BasicMode_2Hands_Hovering);
          return true;
        }
      }

      // apply zoom
      applyZoom(static_cast<float>(1 + ZoomScaleFactor * (m_currentFrame.scaleFactor(m_sinceFrame) - 1)));

      float alphaMult = alphaFromTimeVisible((1.0f/SECONDS)*static_cast<float>(m_currentFrame.timestamp() - m_lastStateChangeTime));
      drawOverlayForPointable(m_currentFrame.pointable(pointable1), 0, alphaMult, false);
      m_lastNumIcons++;
      drawOverlayForPointable(m_currentFrame.pointable(pointable2), 1, alphaMult, false);
      m_lastNumIcons++;

      return true;
  }
  return false; // MUST return false if an event was not handled.
}

// Touch mode only
bool OutputPeripheralBasic::State_BasicMode_2Hands_Touch (StateMachineInput input) {
  int32_t pointable1, pointable2;
  switch (input) {
    case OMB__LOST_FOCUS:
      BASICMODE_TRANSITION_TO(State_BasicMode_0Pointables_NoInteraction);
      return true;

    case SM_ENTER:
      m_updateCursor = true;
      m_drawOverlays = true;
    case OMB__PROCESS_FRAME:
      // handle state group changes
      if (!shouldBeInMultiHandMode(&pointable1, &pointable2)) {
        BasicMode_TransitionTo_NPointables_NoInteraction();
        return true;
      }

      // handle zone changes
      if (collectiveZone() != Pointable::ZONE_TOUCHING) {
        BASICMODE_TRANSITION_TO(State_BasicMode_2Hands_Hovering);
      }

      addTouchPointForPointable(pointable1, m_currentFrame.pointable(pointable1), true, false);
      addTouchPointForPointable(pointable2, m_currentFrame.pointable(pointable2), true, false);

      float alphaMult = alphaFromTimeVisible((1.0f/SECONDS)*static_cast<float>(m_currentFrame.timestamp() - m_lastStateChangeTime));
      drawOverlayForPointable(m_currentFrame.pointable(pointable1), 0, alphaMult, false);
      m_lastNumIcons++;
      drawOverlayForPointable(m_currentFrame.pointable(pointable2), 1, alphaMult, false);
      m_lastNumIcons++;

      return true;
  }
  return false; // MUST return false if an event was not handled.
}

bool OutputPeripheralBasic::State_BasicMode_Palm_GestureRecognition (StateMachineInput input) {
  switch (input) {
    case OMB__LOST_FOCUS:
      BASICMODE_TRANSITION_TO(State_BasicMode_0Pointables_NoInteraction);
      return true;

    case SM_ENTER:
      m_gestureStart = m_currentFrame;
    case OMB__PROCESS_FRAME:
      // handle state changes
      if (!shouldBeInPalmSwipeMode()) {
        BasicMode_TransitionTo_NPointables_NoInteraction();
      }

      // reccognize the gesture
      if (m_currentFrame.timestamp() - m_gestureStart.timestamp() >= 100*MILLISECONDS && rtsIsUnambiguous() && rts() == TRANSLATING) {
        BASICMODE_TRANSITION_TO(State_BasicMode_Palm_Swipe);
      }
      return true;

    case SM_EXIT:
      return true;
  }
  return false; // MUST return false if an event was not handled.
}

bool OutputPeripheralBasic::State_BasicMode_Palm_Swipe (StateMachineInput input) {
  switch (input) {
    case OMB__LOST_FOCUS:
      BASICMODE_TRANSITION_TO(State_BasicMode_0Pointables_NoInteraction);
      return true;

    case SM_ENTER:
      beginGesture(LPGesture::GestureDesktopSwipeHorizontal);
      return true;

    case SM_EXIT:
      endGesture();
      return true;

    case OMB__PROCESS_FRAME:
      // handle state changes
      if (!shouldBeInPalmSwipeMode()) {
        BasicMode_TransitionTo_NPointables_NoInteraction();
      }

      // apply desktop swipe
      generateDesktopSwipeBetweenFrames(m_currentFrame, m_sinceFrame);

      return true;
  }
  return false; // MUST return false if an event was not handled.
}

// end of OUTPUT_MODE_BASIC state handlers
/////////////////////////////////////////////////
#undef BASICMODE_TRANSITION_TO
}
