/*==================================================================================================================

 Copyright (c) 2010 - 2013 Leap Motion. All rights reserved.

 The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
 protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
 strictly forbidden unless prior written permission is obtained from Leap Motion.

 ===================================================================================================================*/

#if !defined(__OutputPeripheralFingerMouse_h__)
#define __OutputPeripheralFingerMouse_h__

#include "GestureInteractionManager.h"

namespace Touchless {

class OutputPeripheralFingerMouse : public GestureInteractionManager {
public:
  OutputPeripheralFingerMouse(OSInteractionDriver& osInteractionDriver, OverlayDriver& overlayDriver);
  virtual ~OutputPeripheralFingerMouse();
  virtual void stopActiveEvents();

protected:

  virtual void processFrameInternal();
  virtual Pointable::Zone identifyCollectivePointableZone (const std::vector<Pointable> &pointables) const;
  virtual void DrawOverlays();

private:

  enum {
    OMFM__PROCESS_FRAME = 0, // state machine input for each frame
    OMFM__LOST_FOCUS
  };

  enum CURSOR_MOVE_TYPE {
    ONE_FINGER_ONLY = 0,
    ANY_HOVER,
    ALWAYS
  };

  // Configuration Parameters
  static const bool UseAbsolutePositioning;
  static const bool UseAcceleratedScrolling;
  static const bool UseAcceleratedDesktopSwipe;
  static bool Use3PlusFingerGestures;
  static bool UseRotateandZoom;
  static CURSOR_MOVE_TYPE CursorMoveType;
  static bool DisableAllOverlays;
  static float ZoomScaleFactor;
  static float TranslationScaleFactor;
  static const int64_t HoverDurationToActivateDrag;

  void DrawPointingOverlay();
  void DrawClickingOrDraggingOverlay();
  void DrawScrollOverlay(bool doubleHorizontalDots = false, bool doubleVerticalDots = false);
  void DrawRotateOverlay();
  void DrawZoomOverlay();
  void DrawDesktopSwipeOverlay(bool movementGlow = true);
  void DrawVerticalPalmOverlay();

  bool                                        m_drawOverlays;
  bool                                        m_noTouching;
  int64_t                                     m_clickDuration;
  bool                                        m_clickEligibleForDrag;
  int                                         m_clickNumber;
  bool                                        m_deferClickUpForDrag;
  int64_t                                     m_lastClickTime;
  Vector                                      m_lastClickLocation;
  Frame                                       m_gestureStart;
  Vector                                      m_clickDownScreenPosition;
  StateMachine<OutputPeripheralFingerMouse>   m_fingerMouseStateMachine;
  bool                                        m_mountainLionOrNewer;


  void setDeltaTrackedAbsoluteCursorPositionHand (const Hand &hand, Vector *calculatedScreenPosition = nullptr);
  void setDeltaTrackedAbsoluteCursorPositionPointable (const Pointable &pointable, Vector *calculatedScreenPosition = nullptr);
  void generateScrollBetweenFrames (const Frame &currentFrame, const Frame &sinceFrame);
  void generateDesktopSwipeBetweenFrames (const Frame &currentFrame, const Frame &sinceFrame);
  double cdGain (double magnitude, double maxVelocity, double initialPower) const;
  //   double awesomeAccel (double velocity, double maxVelocity) const;
  bool shouldBeInPalmSwipeMode () const;

  void FingerMouse_TransitionTo_NFingers_NoInteraction ();

  // beginning of OUTPUT_MODE_FINGER_MOUSE state handlers
  bool State_FingerMouse_0Fingers_NoInteraction (StateMachineInput input);

  bool State_FingerMouse_1Finger_Hovering (StateMachineInput input);
  bool State_FingerMouse_1Finger_Clicking (StateMachineInput input);
  bool State_FingerMouse_1Finger_Dragging (StateMachineInput input);

  bool State_FingerMouse_2Fingers_Hovering (StateMachineInput input);
  bool State_FingerMouse_2Fingers_GestureRecognition (StateMachineInput input);
  bool State_FingerMouse_2Fingers_Rotating (StateMachineInput input);
  bool State_FingerMouse_2Fingers_Scrolling (StateMachineInput input);
  bool State_FingerMouse_2Fingers_Zooming (StateMachineInput input);

  bool State_FingerMouse_3PlusFingers_NoInteraction (StateMachineInput input);
  bool State_FingerMouse_3PlusFingers_Hovering (StateMachineInput input);
  bool State_FingerMouse_3PlusFingers_GestureRecognition (StateMachineInput input);
  bool State_FingerMouse_3PlusFingers_SwipeVertical (StateMachineInput input);
  bool State_FingerMouse_3PlusFingers_SwipeHorizontal (StateMachineInput input);

  bool State_FingerMouse_Palm_GestureRecognition (StateMachineInput input);
  bool State_FingerMouse_Palm_Swipe (StateMachineInput input);
  // end of OUTPUT_MODE_FINGER_MOUSE state handlers
};

}

#endif // __OutputPeripheralFingerMouse_h__
