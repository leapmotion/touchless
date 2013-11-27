/*==================================================================================================================

 Copyright (c) 2010 - 2013 Leap Motion. All rights reserved.

 The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
 protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
 strictly forbidden unless prior written permission is obtained from Leap Motion.

 ===================================================================================================================*/

#if !defined(__OutputPeripheralMode_h__)
#define __OutputPeripheralMode_h__

#include "ocuType.h"

#if LEAP_API_INTERNAL
#include "LeapInternal.h"
#else
#include "Leap.h"
#endif

#include "DataTypes.h"
#include "StateMachine.h"
#include "FilterMethods/CategoricalFilter.h"
#include "FilterMethods/RollingMean.h"
#include "TimedHistory.h"

#include <vector>

#define MAX_POINTABLES 10

namespace Leap{

class OutputPeripheralImplementation;

class OutputPeripheralMode {
public:

  OutputPeripheralMode(OutputPeripheralImplementation& outputPeripheral);
  virtual ~OutputPeripheralMode();

  void processFrame (const Frame& frame, const Frame& sinceFrame);
  virtual void stopActiveEvents();

protected:

  // this must be implemented in a subclass -- it provides the mode-specific peripheral behavior.
  virtual void processFrameInternal() = 0;
  // a default implementation, currently taken from the finger mouse.
  virtual void identifyRelevantPointables (const PointableList &pointables, std::vector<Pointable> &relevantPointables) const;
  // a default implementation, currently taken from the finger mouse.
  virtual Pointable::Zone identifyCollectivePointableZone (const std::vector<Pointable> &pointables) const;
  // a default implementation, currently taken from the finger mouse.
  virtual void DrawOverlays();
  // a default implementation, sets to -1 if the number of actual fingers doesn't match the finger category
  virtual void setForemostPointable (const std::vector<Pointable> &relevantPointables, int32_t &foremostPointableId) const;

  enum RTS {
    ROTATING = 0,
    TRANSLATING,
    SCALING,

    RTS__CATEGORY_COUNT // that this gives the category count relies on the fact that the first value is 0.
  };

  typedef CategoricalFilter<RTS,RTS__CATEGORY_COUNT>  RTSFilter;

  const InteractionBox &interactionBox() const;
  int32_t foremostPointableId () const;
  int32_t foremostPointableIdOfHand(const Hand &hand) const;
  void resetFavoritePointableId();
  int32_t favoritePointableId();
  double fps() const;
  // TODO: should probably name these filteredRTS*
  bool rtsIsUnambiguous() const;
  RTS rts() const;
  RTSFilter::ProbabilityVector rtsProbabilityVector () const;
  // TODO: should probably name these filteredPointableCount*
  bool pointableCountIsUnambiguous() const;
  int pointableCount() const;
  Pointable::Zone collectiveZone() const;

  bool deviceToScreen(const Vector& position, Vector& output, Vector& clampVec, float scale = 1, bool clamp = true);
  bool normalizedToScreen(const Vector& position, Vector& output, Vector& clampVec, float scale = 1, bool clamp = true);
  bool normalizedToAspect(const Vector& position, Vector& output, Vector& clampVec, float scale = 1, bool clamp = true);
  void setCursorPosition(float fx, float fy, bool absolute = true);
  void clickDown(int button, int number = 1);
  void clickUp(int button, int number = 1);
  bool beginGesture(uint32_t gestureType);
  bool endGesture();
  bool applyZoom(float zoom);
  bool applyRotation(float rotation);
  bool applyScroll(float dx, float dy, int64_t timeDiff);
  bool applyDesktopSwipe(float dx, float dy);
  void applyCharms(const Vector& aspectNormalized, int numPointablesActive, int& charmsMode);
  float acceptableClampDistance() const;

  void setIconVisibility(int index, bool visible);
  int findImageIndex(float z, float touchThreshold, float touchRange) const;
  void drawImageIcon(int iconIndex, int imageIndex, float x, float y, bool visible);
  void drawOverlayForPointable (const Pointable &pointable, int32_t iconIndex = 0, float alphaMult = 1.0f, bool deltaTracked = true);
  void drawOverlayForHand (const Hand &hand, int32_t iconIndex = 0, float alphaMult = 1.0f);
  void drawGestureOverlayForHand (const Hand& hand, float rotationAngle, float scaleFactor, float alphaMult = 1.0f, const Vector *positionOverride = nullptr, bool doubleHorizontalDots = false, bool doubleVerticalDots = false, bool verticalMovementGlow = true, bool horizontalMovementGlow = true);
  void drawGestureOverlayForPointable (const Pointable& pointable, float rotationAngle, float scaleFactor, float alphaMult = 1.0f, const Vector *positionOverride = nullptr, bool doubleHorizontalDots = false, bool doubleVerticalDots = false, bool verticalMovementGlow = true, bool horizontalMovementGlow = true);
  void drawGestureOverlayCore(Vector screenPosition, Vector clampVec, Vector tipVelocity, double radius, float touchDistance, float alpha, float rotationAngle, float scaleFactor, float alphaMult, bool doubleHorizontalDots, bool doubleVerticalDots, bool verticalMovementGlow = true, bool horizontalMovementGlow = true);
  void drawScrollOverlayForHand (const Hand& hand, float alphaMult = 1.0f, const Vector *positionOverride = nullptr, bool doubleHorizontalDots = false, bool doubleVerticalDots = false, bool verticalMovementGlow = true, bool horizontalMovementGlow = true);
  void drawScrollOverlayForPointable (const Pointable& pointable, float alphaMult = 1.0f, const Vector *positionOverride = nullptr, bool doubleHorizontalDots = false, bool doubleVerticalDots = false, bool verticalMovementGlow = true, bool horizontalMovementGlow = true);
  void drawRotateOverlayForHand (const Hand& hand, float alphaMult = 1.0f, const Vector *positionOverride = nullptr);
  void drawRotateOverlayForPointable (const Pointable& pointable, float alphaMult = 1.0f, const Vector *positionOverride = nullptr);
  void drawZoomOverlayForHand (const Hand& hand, float alphaMult = 1.0f, const Vector *positionOverride = nullptr);
  void drawZoomOverlayForPointable (const Pointable& pointable, float alphaMult = 1.0f, const Vector *positionOverride = nullptr);
  void addTouchPointForPointable (int touchId, const Pointable &pointable, bool touching, bool deltaTracked = true);
  void addTouchPointForHand (const Hand &position, bool touching);
  void setAbsoluteCursorPosition (const Vector &deviceCoordinatePosition, Vector *calculatedScreenPosition = nullptr);
  virtual void setAbsoluteCursorPositionHand (const Hand &hand, Vector *calculatedScreenPosition = nullptr);
  virtual void setAbsoluteCursorPositionPointable (const Pointable &pointable, Vector *calculatedScreenPosition = nullptr);

  float alphaFromTimeVisible(float timeVisible) const;

  static float scrollDampingFactor (const Vector &scrollVelocity);
  static double touchDistanceToRadius (float touchDistance);
  void drawRasterIcon(int iconIndex, float x, float y, bool visible, const Vector3& velocity, float touchDistance, double radius, float clampDistance, float alphaMult, int numFingers = 1);
//   bool checkTouching(const Vector& position, float noTouchBorder) const; // NOTE: this is apparently undefined in OutputPeripheral
  void clearTouchPoints();
  void removeTouchPoint(int touchId);
  void addTouchPoint(int touchId, float x, float y, bool touching);
  void emitTouchEvent();
  bool touchAvailable() const;
  int numTouchScreens() const;
  int touchVersion() const;
  bool useProceduralOverlay() const;
  bool useCharmHelper() const;
#if __APPLE__
  void flushOverlay();
#endif
  void identifyForemostPointable (const std::vector<Pointable> &relevantPointables, int32_t &foremostPointableId) const;

  static const int64_t SECONDS;
  static const int64_t MILLISECONDS;
  static const int64_t MICROSECONDS;

  typedef TimedHistory<Frame,int64_t> TimedFrameHistory;

  int                                         m_numOverlayImages;
  Frame                                       m_currentFrame;
  Frame                                       m_sinceFrame;
  TimedFrameHistory                           m_timedFrameHistory;
  std::vector<Pointable>                      m_relevantPointables;
  
  bool                                        m_flushOverlay;

private:

  typedef RollingMean<MAX_POINTABLES>                 PointableCountFilterRollingMean;
  typedef CategoricalFilter<size_t,MAX_POINTABLES>    PointableCountFilter;
  typedef RollingMean<RTS__CATEGORY_COUNT>            RTSFilterRollingMean;

  Pointable::Zone                             m_collectiveZone;
  PointableCountFilter                       *m_filteredPointableCount;
  RTSFilter                                  *m_filteredRTS;
  RollingMean<1>                              m_FPS;
  InteractionBox                              m_interactionBox;
  int32_t                                     m_foremostPointableId;
  int32_t                                     m_favoritePointableId;
  OutputPeripheralImplementation             &m_outputPeripheral;

protected:

  // This is for removing discontinuities in cursor/overlay position resulting
  // from switching between pointable and palm positions for setting the cursor/
  // overlay position.  Positional deltas are accumulated so that the output
  // position is continuous.  There should be one of these objects per tracked
  // hand, and tracking 2+ hands will require additional logic to determine which
  // one of these objects corresponds to which hand.
  class PositionalDeltaTracker {
  public:

    PositionalDeltaTracker () {
      clear();
    }

    Vector getTrackedPosition () const { return m_position + m_delta; }

    void setPositionToStabilizedPositionOf (const Hand &hand) {
      assert(hand.isValid());
      setPosition(hand.id(), -1, hand.stabilizedPalmPosition());
    }
    void setPositionToStabilizedPositionOf (const Pointable &pointable) {
      assert(pointable.isValid());
      setPosition(pointable.hand().id(), pointable.id(), pointable.stabilizedTipPosition());
    }

    void clear () {
      m_handId = -1;
      m_pointableId = -1;
      m_delta = Vector::zero();
      m_position = Vector::zero();
    }

  private:

    // A hand id of -1 is considered to be equal to any hand id.  Otherwise, different
    // ids are considered to belong to different hands/pointables.  If pointableId is
    // specified, then position is understood to be the pointable position, otherwise
    // it is the hand position.  Specifying -1 for both handId and pointableId is not
    // allowed.
    //
    // Different possible transitions:
    //
    // - If the hand id is different, then the delta is cleared, and the new position
    //   is used directly.
    // - Otherwise the hand id is the same.
    //   * If the pointable id is the same, no delta is accumulated.
    //   * Otherwise, the pointable id changed, so a delta is accumulated.
    void setPosition (int32_t handId, int32_t pointableId, const Vector &position) {
      // if this object is uninitialized, set everything and return.
      if (m_handId == -1 && m_pointableId == -1) {
        m_handId = handId;
        m_pointableId = pointableId;
        m_delta = Vector::zero();
        m_position = position;
        return;
      }

      // determine if a delta should be accumulated.
      bool accumulateDelta = false;
      if (handId == -1 || m_handId == -1) {
        // -1 for hand id matches everything, so don't accumulate delta.
      } else if (handId != m_handId) {
        accumulateDelta = true;
      } else {
        if (pointableId != m_pointableId) {
          accumulateDelta = true;
        } else {
          // the pointable ids match, so don't accumulate delta.
        }
      }
      // if a delta accumulation was indicated, do so.
      if (accumulateDelta) {
        m_delta = m_position + m_delta - position;
      }

      // update state variables
      m_handId = handId;
      m_pointableId = pointableId;
      m_position = position;
    }

    int32_t                   m_handId;
    int32_t                   m_pointableId;
    Vector                    m_delta;
    Vector                    m_position;
  };

  PositionalDeltaTracker m_positionalDeltaTracker;
};

}

#endif // __OutputPeripheralMode_h__
