/*==================================================================================================================

 Copyright (c) 2010 - 2013 Leap Motion. All rights reserved.

 The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
 protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
 strictly forbidden unless prior written permission is obtained from Leap Motion.

 ===================================================================================================================*/
#include "LPGesture.h"
#include "GestureOnlyMode.h"

#if __APPLE__
#include <sys/sysctl.h>
#endif

namespace Touchless {

float GestureOnlyMode::ZoomScaleFactor = 1.5;
float GestureOnlyMode::TranslationScaleFactor = 1.5;
const int64_t GestureOnlyMode::GestureRecognitionDuration = 100*MILLISECONDS;
const float GestureOnlyMode::BUCKET_THRESHOLD = 0.75f;
const float GestureOnlyMode::BUCKET_THRESHOLD_LOW = 0.50f;


GestureOnlyMode::GestureOnlyMode(OSInteractionDriver& osInteractionDriver, OverlayDriver& overlayDriver)
  :
  GestureInteractionManager(osInteractionDriver, overlayDriver),
  m_noTouching(true),
  m_justScrolled(false),
  m_timedCountHistory(500*MILLISECONDS),
  m_favoriteHandId(-1)
{
  m_stateMachine.SetOwnerClass(this, "GestureOnlyMode");

  // This tells the state machine to log transitions to cerr.  An optional second can be used to specify
  // an event ID to ignore (such as something that happens every single frame) to avoid spamming the console.
  // Specify that OMGO__PROCESS_FRAME should be ignored.
  //m_stateMachine.SetTransitionLogger(&std::cerr, OMGO__PROCESS_FRAME);
}

GestureOnlyMode::~GestureOnlyMode() {
  m_stateMachine.Shutdown();
}

void GestureOnlyMode::processFrameInternal() {
  PointableCountBucketCategory category = PCBC_OTHER;
  if (shouldBeInPalmSwipeMode()) {
    category = PCBC_PALM;
  } else if (m_relevantPointables.size() > 0 && m_relevantPointables.size() <=2) {
    category = PCBC_TWO;
  } else if (m_relevantPointables.size() > 2) {
    category = PCBC_THREEPLUS;
  }

  m_timedCountHistory.addFrame(m_currentFrame.timestamp(), category);

  m_noTouching = std::count_if(m_relevantPointables.begin(), m_relevantPointables.end(),
                               [this] (const Pointable& pointable) {
                                  return pointable.touchZone() == Pointable::ZONE_TOUCHING;
                               }) == 0;
  // initialize finger mouse state machine if necessary
  if (!m_stateMachine.IsInitialized()) {
    m_stateMachine.Initialize(&GestureOnlyMode::State_GestureRecognition, "State_GestureRecognition");
  }

  // make the pointable count buckets
  for (int i = 0; i < PCBC__COUNT; ++i) {
    m_pointableCountBucket[i] = 0.0f;
  }
  auto frameWindowEnd = m_timedCountHistory.getFrameHavingAgeAtLeast(GestureRecognitionDuration);
  if (frameWindowEnd != m_timedCountHistory.end()) {
    unsigned int frameCount = 0;
    for (auto it = m_timedCountHistory.begin(); it != frameWindowEnd; ++it) {
      ++frameCount;
      m_pointableCountBucket[it->second] += 1.0f;
    }
    assert(frameCount > 0);
    for (int i = 0; i < PCBC__COUNT; ++i) {
      m_pointableCountBucket[i] /= frameCount;
    }
  } else {
    // not enough history
    m_pointableCountBucket[PCBC_OTHER] = 1.0f;
  }

  // If any bucket breaks the threshold emit the appropriate event, if all buckets are low enough, emit an uncertain event.
  if (m_pointableCountBucket[PCBC_TWO] >= BUCKET_THRESHOLD) {
    m_stateMachine.RunCurrentState(OMGO__COUNT_TWO);
  } else if (m_pointableCountBucket[PCBC_THREEPLUS] >= BUCKET_THRESHOLD) {
    m_stateMachine.RunCurrentState(OMGO__COUNT_THREEPLUS);
  } else if (m_pointableCountBucket[PCBC_OTHER] >= BUCKET_THRESHOLD) {
    m_stateMachine.RunCurrentState(OMGO__COUNT_OTHER);
  } else if (m_pointableCountBucket[PCBC_PALM] >= BUCKET_THRESHOLD) {
    m_stateMachine.RunCurrentState(OMGO__COUNT_PALM);
  } else if (m_pointableCountBucket[PCBC_TWO] < BUCKET_THRESHOLD_LOW
             && m_pointableCountBucket[PCBC_THREEPLUS] < BUCKET_THRESHOLD_LOW
             && m_pointableCountBucket[PCBC_OTHER] < BUCKET_THRESHOLD_LOW
             && m_pointableCountBucket[PCBC_PALM] < BUCKET_THRESHOLD_LOW) {
    m_stateMachine.RunCurrentState(OMGO__COUNT_UNCERTAIN);
  }

  // If no fingers are touching emit a not touching event, if our favorite hand has touching fingers, or alternately
  // if our favorite hand doesn't exist, but our favorite pointable is touching, emit a touching event.
  if (m_noTouching) {
    m_stateMachine.RunCurrentState(OMGO__NOT_TOUCHING);
  } else if (hasFingersTouching(m_currentFrame.hand(m_favoriteHandId))
             || (!m_currentFrame.hand(m_favoriteHandId).isValid() && m_currentFrame.pointable(favoritePointableId()).touchZone() == Pointable::ZONE_TOUCHING)) {
    m_stateMachine.RunCurrentState(OMGO__TOUCHING);
  }

#if __APPLE__
  // clear overlays
  if (m_flushOverlay) {
    for (int i = 0; i < m_numOverlayImages; ++i) {
      setIconVisibility(i, false);
    }
  }
#endif

  // run the state machine with the "process frame" input
  m_stateMachine.RunCurrentState(OMGO__PROCESS_FRAME);

#if __APPLE__
  flushOverlay();
#endif
}

void GestureOnlyMode::identifyRelevantPointables (const PointableList &pointables, std::vector<Pointable> &relevantPointables) const {
  // Remove all ZONE_NONE pointables, backwards pointables, pointables pointing too close to straight up or down,
  // and compare each pointable to the foremost pointable with the same hand id with a linear discriminant.
  std::map<int32_t, int32_t> best_pointables; //map hand id to pointable id

  relevantPointables.clear();
  for (int i=0; i<pointables.count(); i++) {
    relevantPointables.push_back(pointables[i]);

    // Look for a matching hand in the map
    auto it = best_pointables.find(pointables[i].hand().id());
    if (it == best_pointables.end()) {
      // This is the first pointable on this hand
      best_pointables[pointables[i].hand().id()] = pointables[i].id();
    } else {
      // We've seen this hand before
      if (m_currentFrame.pointable(it->first).tipPosition().z > pointables[i].tipPosition().z) {
        // but this pointable is better so use it
        best_pointables[pointables[i].hand().id()] = pointables[i].id();
      }
    }
  }

  auto new_end = std::remove_if(relevantPointables.begin(),
                                relevantPointables.end(),
                                [this, best_pointables] (const Pointable& pointable) -> bool {
                                  Pointable best = m_currentFrame.pointable(best_pointables.find(pointable.hand().id())->second);
                                  return pointable.touchZone() == Pointable::ZONE_NONE
                                  || pointable.direction().z > 0
                                  || 160 * (1 - pointable.direction().dot(best.direction())) + sqrt((pointable.tipPosition().z - best.tipPosition().z) * (pointable.tipPosition().z - best.tipPosition().z) + (pointable.tipPosition().y - best.tipPosition().y) * (pointable.tipPosition().y - best.tipPosition().y)) > 50
                                  || std::abs(pointable.direction().y) > 0.60f;
                                });
  relevantPointables.erase(new_end, relevantPointables.end());
}

void GestureOnlyMode::setForemostPointable (const std::vector<Pointable> &relevantPointables, int32_t &foremostPointableId) const {
    identifyForemostPointable(relevantPointables, foremostPointableId);
}

void GestureOnlyMode::stopActiveEvents()
{
  m_stateMachine.RunCurrentState(OMGO__LOST_FOCUS);
}

void GestureOnlyMode::generateScrollBetweenFrames (const Frame &currentFrame, const Frame &sinceFrame) {
  float scroll_dx, scroll_dy;

  // scaled scrolling

  // damp out small motions
  Vector translation = currentFrame.translation(sinceFrame);
  translation.z = 0;
  float timeDelta = float(currentFrame.timestamp() - sinceFrame.timestamp()) / SECONDS;
  translation *= scrollDampingFactor(translation / timeDelta);

  scroll_dx = static_cast<float>(TranslationScaleFactor * translation.x);
  scroll_dy = static_cast<float>(-TranslationScaleFactor * translation.y);

  applyScroll(scroll_dx, scroll_dy, currentFrame.timestamp() - sinceFrame.timestamp());
}

void GestureOnlyMode::generateDesktopSwipeBetweenFrames (const Frame &currentFrame, const Frame &sinceFrame) {
  float scroll_dx, scroll_dy;

  // scaled scrolling

  // damp out small motions
  Vector translation = currentFrame.translation(sinceFrame);
  float timeDelta = float(currentFrame.timestamp() - sinceFrame.timestamp()) / SECONDS;
  translation *= scrollDampingFactor(translation / timeDelta);

  scroll_dx = static_cast<float>(translation.x);
  scroll_dy = static_cast<float>(-translation.y);

  applyDesktopSwipe(scroll_dx, scroll_dy);
}

bool GestureOnlyMode::shouldBeInPalmSwipeMode () const {
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

  return std::abs(hand.palmNormal().x) >= COS_X_AXIS_ANGLE_THRESHOLD;
}

bool GestureOnlyMode::hasFingersTouching(const Hand& hand) const {
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
#define GESTUREONLY_TRANSITION_TO(x) m_stateMachine.SetNextState(&GestureOnlyMode::x, #x)

// ////////////////////////////////////////////////////
// beginning of OUTPUT_MODE_FINGER_MOUSE state handlers

bool GestureOnlyMode::State_GestureRecognition (StateMachineInput input) {
  m_positionalDeltaTracker.clear();
  m_justScrolled = false;
  auto frameWindowEnd = m_timedFrameHistory.getFrameHavingAgeAtLeast(GestureRecognitionDuration);
  if (frameWindowEnd == m_timedFrameHistory.end()) {
    return true; // not enough history
  }

  const Frame &frame = frameWindowEnd->second;

  switch (input) {
    case OMGO__COUNT_PALM:
      GESTUREONLY_TRANSITION_TO(State_Palm_Swipe);
      return true;

    case OMGO__COUNT_TWO:
        // 2 finger gestures win
        m_gestureStart = frame;
        GESTUREONLY_TRANSITION_TO(State_2Fingers_Hovering);
        return true;

    case OMGO__COUNT_THREEPLUS: {
      // 3+ gestures win
      Vector translation = m_currentFrame.translation(frame);
      float x_y_distance = sqrtf(translation.x*translation.x + translation.y*translation.y);
      static const float TRANSLATION_DISTANCE_THRESHOLD = 50.0f;
      //std::cerr << "x_y_distance = " << x_y_distance << '\n';
      if (x_y_distance >= TRANSLATION_DISTANCE_THRESHOLD) {
        bool horizontal_wins = false;
        bool vertical_wins = false;
        if (x_y_distance > 2*std::abs(translation.z)) {
          if (std::abs(translation.x) > 2*std::abs(translation.y) + 1) {
              horizontal_wins = true;
          } else if (std::abs(translation.y) > 2*std::abs(translation.x) + 1) {
            vertical_wins = true;
          }
        }

        if (horizontal_wins) {
          GESTUREONLY_TRANSITION_TO(State_3PlusFingers_SwipeHorizontal);
        } else if (vertical_wins) {
          GESTUREONLY_TRANSITION_TO(State_3PlusFingers_SwipeVertical);
        }
      }
      return true;
    }
    default:
     return true;
  }
  return false; // MUST return false if an event was not handled.
}

bool GestureOnlyMode::State_Cooldown (StateMachineInput input) {
  switch (input) {
    case OMGO__LOST_FOCUS:
      GESTUREONLY_TRANSITION_TO(State_GestureRecognition);
      return true;

    case SM_ENTER:
      m_cooldownStartTime = m_currentFrame.timestamp();
    case OMGO__PROCESS_FRAME: {
      static const int64_t COOLDOWN_DURATION = 500*MILLISECONDS;
      if (m_currentFrame.timestamp() - m_cooldownStartTime >= COOLDOWN_DURATION) {
        GESTUREONLY_TRANSITION_TO(State_GestureRecognition);
        return true;
      }
      return true;
    }
    case SM_EXIT:
      return true;

    default:
      return true;
  }
  return false; // MUST return false if an event was not handled.
}


bool GestureOnlyMode::State_2Fingers_Hovering (StateMachineInput input) {
  switch (input) {
    case OMGO__LOST_FOCUS:
      GESTUREONLY_TRANSITION_TO(State_GestureRecognition);
      return true;

    case SM_ENTER: {
      resetFavoritePointableId();
      m_lastStateChangeTime = m_currentFrame.timestamp();
      if (m_currentFrame.pointable(favoritePointableId()).hand().isValid()) {
        m_favoriteHandId = m_currentFrame.pointable(favoritePointableId()).hand().id();
      } else {
        m_favoriteHandId = -1;
      }
      return true;
    }

    case OMGO__TOUCHING:
      GESTUREONLY_TRANSITION_TO(State_2Fingers_Scrolling);
      return true;

    case OMGO__COUNT_OTHER:
    case OMGO__COUNT_PALM:
    case OMGO__COUNT_THREEPLUS:
    case OMGO__COUNT_UNCERTAIN:
      if (m_justScrolled) {
        GESTUREONLY_TRANSITION_TO(State_Cooldown);
      } else {
        GESTUREONLY_TRANSITION_TO(State_GestureRecognition);
      }
      return true;

    case OMGO__PROCESS_FRAME: {
      Pointable favoritePointable = m_currentFrame.pointable(favoritePointableId());
      // handle pointable going out of scope
      if (!favoritePointable.isValid()) {
        return true;
      }

      // see if hand has come into view
      if (m_favoriteHandId < 0 && favoritePointable.hand().isValid()) {
        m_favoriteHandId = favoritePointable.hand().id();
      }

      // if no hand in view, just draw normal overlay
      if (m_favoriteHandId < 0) {
        float alphaMult = alphaFromTimeVisible((1.0f/SECONDS)*static_cast<float>(m_currentFrame.timestamp() - m_lastStateChangeTime));
        m_positionalDeltaTracker.setPositionToStabilizedPositionOf(favoritePointable);
        drawScrollOverlayForPointable(favoritePointable, alphaMult);
      } else {
        Hand favoriteHand = m_currentFrame.hand(m_favoriteHandId);

        m_positionalDeltaTracker.setPositionToStabilizedPositionOf(favoriteHand);
        drawScrollOverlayForHand(favoriteHand);
      }
      return true;
    }

    default:
      return true;
  }
  return false; // MUST return false if an event was not handled.
}

bool GestureOnlyMode::State_2Fingers_Scrolling (StateMachineInput input) {
  switch (input) {
    case OMGO__LOST_FOCUS:
      GESTUREONLY_TRANSITION_TO(State_GestureRecognition);
      return true;

    case SM_ENTER: {
      m_justScrolled = true;
      m_lastStateChangeTime = m_currentFrame.timestamp();

      //Get the favorite pointable and hand
      Hand favoriteHand = m_currentFrame.hand(m_favoriteHandId);
      Pointable favoritePointable = m_currentFrame.pointable(favoritePointableId());

      if (favoriteHand.isValid()) {
        setAbsoluteCursorPositionHand(favoriteHand);
      } else if(favoritePointable.hand().isValid()) {
        m_favoriteHandId = favoritePointable.hand().id();
        setAbsoluteCursorPositionHand(favoritePointable.hand());
      } else {
        setAbsoluteCursorPositionPointable(favoritePointable);
      }

      beginGesture(LPGesture::GestureScroll);

      return true;
    }

    case SM_EXIT:
      // generate end-scroll event
      endGesture();
      m_flushOverlay = true;

      return true;

    case OMGO__NOT_TOUCHING:
      GESTUREONLY_TRANSITION_TO(State_2Fingers_Hovering);
      return true;

    case OMGO__COUNT_UNCERTAIN:
    case OMGO__COUNT_THREEPLUS:
    case OMGO__COUNT_OTHER:
    case OMGO__COUNT_PALM:
      GESTUREONLY_TRANSITION_TO(State_GestureRecognition);
      return true;

    case OMGO__PROCESS_FRAME: {
      // hand hand going out of scope
      Hand favoriteHand = m_currentFrame.hand(m_favoriteHandId);
      Pointable favoritePointable = m_currentFrame.pointable(favoritePointableId());

      generateScrollBetweenFrames(m_currentFrame, m_sinceFrame);

      float alphaMult = 1.0f - 0.4f*alphaFromTimeVisible((1.0f/SECONDS)*static_cast<float>(m_currentFrame.timestamp() - m_lastStateChangeTime));
      if (favoriteHand.isValid()) {
        m_positionalDeltaTracker.setPositionToStabilizedPositionOf(favoriteHand);
        drawScrollOverlayForHand(favoriteHand, alphaMult);
      } else {
        m_positionalDeltaTracker.setPositionToStabilizedPositionOf(favoritePointable);
        drawScrollOverlayForPointable(favoritePointable, alphaMult);
      }

      return true;
    }
    default:
      return true;
  }
  return false; // MUST return false if an event was not handled.
}

bool GestureOnlyMode::State_3PlusFingers_SwipeVertical (StateMachineInput input) {
  switch (input) {
    case OMGO__LOST_FOCUS:
      GESTUREONLY_TRANSITION_TO(State_GestureRecognition);
      return true;

    case SM_ENTER:
      // generate begin-scroll event
      beginGesture(LPGesture::GestureDesktopSwipeVertical);
    case OMGO__PROCESS_FRAME: {
      // the distance that is required to trigger this gesture is enough to
      // cause the expose to show
      auto it = m_timedFrameHistory.getFrameHavingAgeAtLeast(100*MILLISECONDS);
      if (it != m_timedFrameHistory.end()) {
        generateDesktopSwipeBetweenFrames(m_currentFrame, it->second);
        GESTUREONLY_TRANSITION_TO(State_Cooldown);
        return true;
      }
      return true;
    }

    case SM_EXIT:
      // generate end-scroll event
      endGesture();
      return true;

    default:
      return true;
  }
  return false; // MUST return false if an event was not handled.
}

bool GestureOnlyMode::State_3PlusFingers_SwipeHorizontal (StateMachineInput input) {
  switch (input) {
    case OMGO__LOST_FOCUS:
      GESTUREONLY_TRANSITION_TO(State_GestureRecognition);
      return true;

    case SM_ENTER:
      // generate begin-scroll event
      beginGesture(LPGesture::GestureDesktopSwipeHorizontal);
    case OMGO__PROCESS_FRAME: {
      // the distance that is required to trigger this gesture is enough to
      // cause a swipe of exactly one desktop
      auto it = m_timedFrameHistory.getFrameHavingAgeAtLeast(100*MILLISECONDS);
      if (it != m_timedFrameHistory.end()) {
        generateDesktopSwipeBetweenFrames(m_currentFrame, it->second);
        GESTUREONLY_TRANSITION_TO(State_Cooldown);
      }
      return true;
    }

    case SM_EXIT:
      // generate end-scroll event
      endGesture();
      return true;

    default:
      return true;
  }
  return false; // MUST return false if an event was not handled.
}

bool GestureOnlyMode::State_Palm_Swipe (StateMachineInput input) {
  switch (input) {
    case OMGO__LOST_FOCUS:
      GESTUREONLY_TRANSITION_TO(State_GestureRecognition);
      return true;

    case SM_ENTER:
      beginGesture(LPGesture::GestureDesktopSwipeHorizontal);
      return true;

    case SM_EXIT: {
      auto it = m_timedFrameHistory.getFrameHavingAgeAtLeast(100*MILLISECONDS + static_cast<int64_t>((1-BUCKET_THRESHOLD_LOW)*GestureRecognitionDuration));
      auto it2 = m_timedFrameHistory.getFrameHavingAgeAtLeast(static_cast<int64_t>((1-BUCKET_THRESHOLD_LOW)*GestureRecognitionDuration));
      if (it != m_timedFrameHistory.end() && it2 != m_timedFrameHistory.end()) {
        generateDesktopSwipeBetweenFrames(it2->second, it->second);
      }
      endGesture();
      return true;
    }

    case OMGO__COUNT_UNCERTAIN:
    case OMGO__COUNT_TWO:
    case OMGO__COUNT_THREEPLUS:
    case OMGO__COUNT_OTHER:
      GESTUREONLY_TRANSITION_TO(State_Cooldown);
      return true;

    default:
      return true;
  }
  return false; // MUST return false if an event was not handled.
}

// end of state handlers
/////////////////////////////////////////////////
#undef GESTUREONLY_TRANSITION_TO
}
