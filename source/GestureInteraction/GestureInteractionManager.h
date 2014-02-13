#if !defined(__GestureInteractionManager_h__)
#define __GestureInteractionManager_h__

#include "common.h"

#include "Leap.h"

#include "OSInteraction/OSInteraction.h"
#include "Overlay/Overlay.h"

#include "Utility/TimedHistory.h"
#include "PositionalDeltaTracker.h"
#include "Utility/CategoricalFilter.h"
#include "Utility/RollingMean.h"
#include "Utility/StateMachine.h"
#include "OSInteraction/Touch.h"

#include <vector>

#define MAX_POINTABLES 10

namespace Touchless {
using Leap::Frame;
using Leap::Pointable;
using Leap::PointableList;
using Leap::Hand;
using Leap::Vector;
using Leap::InteractionBox;

class OSInteractionDriver;
class OverlayDriver;

enum GestureInteractionMode { OUTPUT_MODE_DISABLED = 0, OUTPUT_MODE_INTRO, OUTPUT_MODE_BASIC, OUTPUT_MODE_ADVANCED };

class GestureInteractionManager {
public:

  GestureInteractionManager(OSInteractionDriver& osInteractionDriver, OverlayDriver& overlayDriver);
  virtual ~GestureInteractionManager();

  static GestureInteractionManager* New(Touchless::GestureInteractionMode desiredMode, OSInteractionDriver& osInteractionDriver, OverlayDriver& overlayDriver);

  void processFrame (const Frame& frame, const Frame& sinceFrame);

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
  void clearTouchPoints();
  void addTouchPoint(int touchId, float x, float y, bool touching);
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

protected:
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
  OSInteractionDriver                        &m_osInteractionDriver;
  OverlayDriver                              &m_overlayDriver;
  Leap::PositionalDeltaTracker                m_positionalDeltaTracker;
  TouchEvent                                  m_touchEvent;

public:
  // Accessor methods:
  Leap::PositionalDeltaTracker& positionalDeltaTracker(void) {return m_positionalDeltaTracker;}
  const InteractionBox& interactionBox() const {return m_interactionBox;}

  /// <summary>
  /// Converts the passed normalized coordinates to pixel coordinates on the virtual screen
  /// </summary>
  /// <param name="position">The normalized coordinate to be converted</param>
  /// <param name="output">A vector that will receive the normalized coordinates</param>
  bool normalizedToScreen(const Vector& position, Vector& output, Vector& clampVec, float scale = 1, bool clamp = true);

  bool normalizedToAspect(const Vector& position, Vector& output, Vector& clampVec, float scale = 1, bool clamp = true);

  // API implementation
  void emitTouchEvent();
};

}

#endif // __GestureInteractionManager_h__
