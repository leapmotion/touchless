/*==================================================================================================================

 Copyright (c) 2010 - 2013 Leap Motion. All rights reserved.

 The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
 protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
 strictly forbidden unless prior written permission is obtained from Leap Motion.

 ===================================================================================================================*/

#if !defined(__OutputPeripheralBasic_h__)
#define __OutputPeripheralBasic_h__

#include "GestureInteractionManager.h"

namespace Touchless {

class OutputPeripheralBasic : public GestureInteractionManager {
// This class is used for both Intro and Basic modes
public:
  OutputPeripheralBasic(OSInteractionDriver& osInteractionDriver, OverlayDriver& overlayDriver);
  virtual ~OutputPeripheralBasic();
  virtual void stopActiveEvents();

  static void SetIntroMode();
  static void SetBasicMode();

protected:

  virtual void processFrameInternal();

private:

  enum {
    OMB__PROCESS_FRAME = 0, // for updating movements, scrolls, rotations, etc.
    OMB__LOST_FOCUS
  };

  // Configuration Parameters
  static const bool UseMouseOutput;
  static bool DisableClicking;
  static bool DisablePalmSwipe;
  static bool DisableAllOverlays;
  static bool DisableHorizontalScrolling;
  static float TranslationScaleFactor;
  static float ZoomScaleFactor;

  bool                                  m_drawOverlays;
  bool                                  m_noTouching;
  int64_t                               m_clickDuration;
  int                                   m_clickNumber;
  bool                                  m_clickDown;
  int64_t                               m_lastClickTime;
  Vector                                m_lastClickLocation;
  Frame                                 m_gestureStart;
  Vector                                m_clickDownScreenPosition;
  Vector                                m_scrollStartPosition;
  int                                   m_scrollDrawCount;
  StateMachine<OutputPeripheralBasic>   m_basicModeStateMachine;
  int                                   m_lastNumIcons;
  int                                   m_charmsMode;
  int64_t                               m_lastStateChangeTime;
  int32_t                               m_favoriteHandId;
  Pointable                             m_clickPointable;
  bool                                  m_updateCursor;

  virtual void setAbsoluteCursorPositionHand (Vector *calculatedScreenPosition = nullptr);
  virtual void setAbsoluteCursorPositionPointable (Vector *calculatedScreenPosition = nullptr);
  void generateScrollBetweenFrames (const Frame &currentFrame, const Frame &sinceFrame);
  void generateDesktopSwipeBetweenFrames (const Frame &currentFrame, const Frame &sinceFrame);

  bool shouldBeInMultiHandMode(int32_t *pointableId1 = nullptr, int32_t *pointableId2 = nullptr);
  bool shouldBeInPalmSwipeMode () const;
  bool hasFingersTouching(const Hand& hand) const;

  void BasicMode_TransitionTo_NPointables_NoInteraction ();

  // beginning of OUTPUT_MODE_BASIC state handlers
  bool State_BasicMode_0Pointables_NoInteraction (StateMachineInput input);

  bool State_BasicMode_1Pointable_Hovering (StateMachineInput input);
  bool State_BasicMode_1Pointable_ClickCooldown (StateMachineInput input);

  bool State_BasicMode_2PlusPointables_Hovering (StateMachineInput input);
  bool State_BasicMode_2PlusPointables_Scrolling (StateMachineInput input);

  bool State_BasicMode_2Hands_Hovering (StateMachineInput input);
  bool State_BasicMode_2Hands_GestureRecognition (StateMachineInput input);
  bool State_BasicMode_2Hands_Rotating (StateMachineInput input);
  bool State_BasicMode_2Hands_Zooming (StateMachineInput input);
  bool State_BasicMode_2Hands_Touch (StateMachineInput input);

  bool State_BasicMode_Palm_GestureRecognition (StateMachineInput input);
  bool State_BasicMode_Palm_Swipe (StateMachineInput input);
  // end of OUTPUT_MODE_BASIC state handlers
};

}

#endif // __OutputPeripheralBasic_h__
