/*==================================================================================================================

 Copyright (c) 2010 - 2013 Leap Motion. All rights reserved.

 The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
 protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
 strictly forbidden unless prior written permission is obtained from Leap Motion.

 ===================================================================================================================*/

#if !defined(__OutputPeripheralGestureOnly_h__)
#define __OutputPeripheralGestureOnly_h__

#include "OutputPeripheralMode.h"

namespace Leap {

/*
The design criteria of gesture-only mode are to provide gesture-recognition (e.g. multiple finger
swipes, palm swipes, perhaps circle recognition), provide no cursor control, and do no overlay drawing.

Not using the touch zones is a general guideline for this design.

Currently only 3+ finger gestures are supported (horiz/vert swipe), but a possible development direction
is to include 2 finger gestures (rotate, scroll, zoom) to the currently focused application in a way that
doesn't require engaging the laggy plane, and requires no extra visual feedback (no overlays).

Perhaps cursor control and hover-to-click could also be added, though this would cause the design to
diverge slightly from "gesture only".
*/

class OutputPeripheralGestureOnly : public OutputPeripheralMode {
public:
  OutputPeripheralGestureOnly(OutputPeripheralImplementation& outputPeripheral);
  virtual ~OutputPeripheralGestureOnly();
  virtual void stopActiveEvents();

protected:

  virtual void processFrameInternal();
  virtual void identifyRelevantPointables (const PointableList &pointables, std::vector<Pointable> &relevantPointables) const;
  virtual void setForemostPointable (const std::vector<Pointable> &relevantPointables, int32_t &foremostPointableId) const;

private:

  enum {
    OMGO__PROCESS_FRAME = 0, // state machine input for each frame
    OMGO__LOST_FOCUS,
    OMGO__COUNT_UNCERTAIN,
    OMGO__COUNT_TWO,
    OMGO__COUNT_THREEPLUS,
    OMGO__COUNT_OTHER,
    OMGO__COUNT_PALM,
    OMGO__TOUCHING,
    OMGO__NOT_TOUCHING
  };

  // Configuration Parameters
  static float ZoomScaleFactor;
  static float TranslationScaleFactor;
  static const int64_t GestureRecognitionDuration;
  static const float BUCKET_THRESHOLD;
  static const float BUCKET_THRESHOLD_LOW;

  enum PointableCountBucketCategory {
    PCBC_TWO = 0,
    PCBC_THREEPLUS,
    PCBC_OTHER,
    PCBC_PALM,
    // TODO: add "vertical palm" category and add palm swipe logic

    PCBC__COUNT
  };

  typedef TimedHistory<PointableCountBucketCategory,int64_t> TimedCountHistory;

  bool                                        m_noTouching;
  bool                                        m_justScrolled;
  Frame                                       m_gestureStart;
  StateMachine<OutputPeripheralGestureOnly>   m_stateMachine;
  int64_t                                     m_cooldownStartTime;
  int64_t                                     m_lastStateChangeTime;
  // TODO: write "history" filter (based on Gabe's Filter interface) and use CategoricalFilter with it
  TimedCountHistory                           m_timedCountHistory;
  float                                       m_pointableCountBucket[PCBC__COUNT];
  int32_t                                     m_favoriteHandId;

  void generateScrollBetweenFrames (const Frame &currentFrame, const Frame &sinceFrame);
  void generateDesktopSwipeBetweenFrames (const Frame &currentFrame, const Frame &sinceFrame);
  bool shouldBeInPalmSwipeMode () const;
  bool hasFingersTouching(const Hand& hand) const;

  // beginning of state handlers
  bool State_GestureRecognition (StateMachineInput input);
  bool State_Cooldown (StateMachineInput input);
  bool State_2Fingers_Hovering (StateMachineInput input);
  bool State_2Fingers_GestureRecognition (StateMachineInput input);
  bool State_2Fingers_Rotating (StateMachineInput input);
  bool State_2Fingers_Scrolling (StateMachineInput input);
  bool State_2Fingers_Zooming (StateMachineInput input);
  bool State_3PlusFingers_SwipeVertical (StateMachineInput input);
  bool State_3PlusFingers_SwipeHorizontal (StateMachineInput input);
  bool State_Palm_GestureRecognition (StateMachineInput input);
  bool State_Palm_Swipe (StateMachineInput input);
  // end of state handlers
};

}

#endif // __OutputPeripheralGestureOnly_h__
