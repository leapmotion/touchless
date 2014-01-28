/*==================================================================================================================

 Copyright (c) 2010 - 2013 Leap Motion. All rights reserved.

 The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
 protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
 strictly forbidden unless prior written permission is obtained from Leap Motion.

 ===================================================================================================================*/
#include "OutputPeripheralImplementation.h"
#include "OutputPeripheralMode.h"

#if __APPLE__
#include <sys/sysctl.h>
#endif

namespace Leap {

// for self-documentation of time-based constants
const int64_t OutputPeripheralMode::SECONDS = 1000000;
const int64_t OutputPeripheralMode::MILLISECONDS = 1000;
const int64_t OutputPeripheralMode::MICROSECONDS = 1;

OutputPeripheralMode::OutputPeripheralMode(OutputPeripheralImplementation& outputPeripheral)
  :
  m_outputPeripheral(outputPeripheral),
  m_numOverlayImages(32),
  m_timedFrameHistory(500*MILLISECONDS),
  m_foremostPointableId(-1),
  m_favoritePointableId(-1),
  m_flushOverlay(true)
{
  m_FPS.SetWindow(5);

  // Initalize our categorical filters

  // pointable count filter
  {
    // generate the category vector
    std::vector<size_t> pointableCountCategoryVector;
    for (int i = 0; i < MAX_POINTABLES; ++i){
      pointableCountCategoryVector.push_back(i);
    }
    // create the pointable count filter, with a newly created rolling mean filter.
    // m_filteredPointableCount takes care of deleting the rolling mean filter object.
    m_filteredPointableCount = new PointableCountFilter(pointableCountCategoryVector, new PointableCountFilterRollingMean(), 0.8);
    // this must (should) be done after the creation of the pointable count filter.
    m_filteredPointableCount->getFilterAs<PointableCountFilterRollingMean>().SetWindow(16);
  }

  // RTS filter
  {
    // generate the category vector
    std::vector<RTS> rtsCategoryVector;
    rtsCategoryVector.push_back(ROTATING);
    rtsCategoryVector.push_back(TRANSLATING);
    rtsCategoryVector.push_back(SCALING);
    //rtsCategoryVector.push_back(NONE);
    // create the RTS filter, with a newly created rolling mean filter.
    // m_filteredRTS takes care of deleting the rolling mean filter object.
    m_filteredRTS = new RTSFilter(rtsCategoryVector, new RTSFilterRollingMean(), 0.9);
    // this must (should) be done after the creation of the RTS filter.
    m_filteredRTS->getFilterAs<RTSFilterRollingMean>().SetWindow(2);
  }
}

OutputPeripheralMode::~OutputPeripheralMode() {
#if __APPLE__
  m_outputPeripheral.flushOverlay();
#endif
  delete m_filteredPointableCount;
  delete m_filteredRTS;
}

void OutputPeripheralMode::processFrame (const Frame& frame, const Frame& sinceFrame) {
  m_timedFrameHistory.addFrame(frame.timestamp(), frame);
  m_interactionBox = frame.interactionBox();
  m_currentFrame = frame;
  m_sinceFrame = sinceFrame;

  identifyRelevantPointables(frame.pointables(), m_relevantPointables);
  m_collectiveZone = identifyCollectivePointableZone(m_relevantPointables);

  m_filteredPointableCount->updateWithCategory(m_relevantPointables.size() < MAX_POINTABLES ? m_relevantPointables.size() : MAX_POINTABLES - 1);
  m_filteredRTS->updateWithProbabilityVector(rtsProbabilityVector());
  if (frame.isValid() && sinceFrame.isValid() && frame.timestamp() > sinceFrame.timestamp()) {
    m_FPS.Update(1,
                 Eigen::Matrix<double, 1, 1>::Constant(static_cast<double>(1*SECONDS/(frame.timestamp() - sinceFrame.timestamp()))),
                 Eigen::Matrix<double, 1, 1>::Zero(),
                 1.0f);
  }
  m_filteredRTS->getFilterAs<RTSFilterRollingMean>().SetWindow(m_FPS.Predict(0)(0,0)/20);
  m_filteredPointableCount->getFilterAs<PointableCountFilterRollingMean>().SetWindow(m_FPS.Predict(0)(0,0)/15);

  setForemostPointable(m_relevantPointables, m_foremostPointableId);

  processFrameInternal();
}

void OutputPeripheralMode::identifyRelevantPointables (const PointableList &pointables, std::vector<Pointable> &relevantPointables) const {
  // Remove all ZONE_NONE pointables, backwards pointables,
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
                                  || 160 * (1 - pointable.direction().dot(best.direction())) + sqrt((pointable.tipPosition().z - best.tipPosition().z) * (pointable.tipPosition().z - best.tipPosition().z) + (pointable.tipPosition().y - best.tipPosition().y) * (pointable.tipPosition().y - best.tipPosition().y)) > 50;
                                });
  relevantPointables.erase(new_end, relevantPointables.end());
}

Pointable::Zone OutputPeripheralMode::identifyCollectivePointableZone (const std::vector<Pointable> &pointables) const {
  // NOTE: this is the implementation from finger mouse, which will be used until something different is needed.

  // if there are no pointables, return ZONE_NONE.
  if (pointables.empty()) {
    return Pointable::ZONE_NONE;
  }

  // for now, use the "minimum" of the pointables' zones, where the order is given by the enum values
  // ZONE_NONE = 0, ZONE_HOVERING = 1, ZONE_TOUCHING = 2.

  // first, ensure that this ordering assumption is actually the case.
  //   assert(Pointable::ZONE_NONE < Pointable::ZONE_HOVERING &&
  //          Pointable::ZONE_NONE < Pointable::ZONE_TOUCHING &&
  //          Pointable::ZONE_HOVERING < Pointable::ZONE_TOUCHING);

  // by using the minimum of all the touchZone values, it follows that all pointables must be ZONE_TOUCHING
  // for the collective zone to be ZONE_TOUCHING.  Similar for hovering.  this is a conservative
  // method which may need to be refined.

  size_t number_touching = 0;
  for (size_t i = 0; i < pointables.size(); ++i) {
    if (pointables[i].touchZone() == Pointable::ZONE_TOUCHING){
      ++number_touching;
    }
  }
  if ((m_filteredPointableCount->filteredCategoryIsUnambiguous() && number_touching == m_filteredPointableCount->filteredCategory())
      || number_touching >= 3) {
    return Pointable::ZONE_TOUCHING;
  }
  return Pointable::ZONE_HOVERING;
}

void OutputPeripheralMode::DrawOverlays() {
  // NOTE: this is the implementation from finger mouse, which will be used until something different is needed.

  Vector screenPosition, clampVec;
  for (int i = 0; i < m_numOverlayImages; ++i) {
    if (i < static_cast<int>(m_relevantPointables.size()) &&
        m_outputPeripheral.normalizedToScreen(m_interactionBox.normalizePoint(m_relevantPointables[i].stabilizedTipPosition()),
                                              screenPosition,
                                              clampVec)) {
      float clampDist = clampVec.magnitude();
      if (m_outputPeripheral.useProceduralOverlay()) {
        float touchDistance = m_relevantPointables[i].touchDistance();
        double radius = m_outputPeripheral.touchDistanceToRadius(touchDistance);
        m_outputPeripheral.drawRasterIcon(i,
                                          screenPosition.x,
                                          screenPosition.y,
                                          true,
                                          m_relevantPointables[i].tipVelocity().toVector3<Vector3>(),
                                          touchDistance,
                                          radius,
                                          clampDist,
                                          alphaFromTimeVisible(m_relevantPointables[i].timeVisible()));
      } else {
        int imageIndex = m_outputPeripheral.findImageIndex(m_relevantPointables[i].touchDistance(), 0, 1);
        m_outputPeripheral.drawImageIcon(i, imageIndex, screenPosition.x, screenPosition.y, true);
      }
    } else {
      m_outputPeripheral.setIconVisibility(i, false);
    }
  }
#if __APPLE__
  m_outputPeripheral.flushOverlay();
#endif
}

const InteractionBox &OutputPeripheralMode::interactionBox() const {
  return m_interactionBox;
}

int32_t OutputPeripheralMode::foremostPointableId () const {
  return m_foremostPointableId;
}

int32_t OutputPeripheralMode::foremostPointableIdOfHand(const Hand &hand) const {
  int32_t id;
  std::vector<Pointable> pointableVector;
  PointableList pointables = hand.pointables();
  for (int i=0; i<pointables.count(); i++) {
    pointableVector.push_back(pointables[i]);
  }
  identifyForemostPointable(pointableVector, id);
  return id;
}

void OutputPeripheralMode::resetFavoritePointableId() {
  m_favoritePointableId = foremostPointableId();
}

int32_t OutputPeripheralMode::favoritePointableId() {
  if (m_favoritePointableId < 0 || !m_currentFrame.pointable(m_favoritePointableId).isValid())
  {
    resetFavoritePointableId();
  }
  return m_favoritePointableId;
}

double OutputPeripheralMode::fps() const {
  return m_FPS.Predict(0)(0,0); // Predict returns a 1x1 matrix, so we have to get the single component out of it
}

bool OutputPeripheralMode::rtsIsUnambiguous() const {
  return m_filteredRTS->filteredCategoryIsUnambiguous();
}

OutputPeripheralMode::RTS OutputPeripheralMode::rts() const {
  return m_filteredRTS->filteredCategory();
}

OutputPeripheralMode::RTSFilter::ProbabilityVector OutputPeripheralMode::rtsProbabilityVector () const {
  double probability = 0.0;
  RTSFilter::ProbabilityVector v;
  auto it = m_timedFrameHistory.getFrameHavingAgeAtLeast(20*MILLISECONDS);

  if (it != m_timedFrameHistory.end()) {
    const Frame &sinceFrame = it->second;

    // NOTE: in the future, there will be "instantaneous" probability accessors for RTS which we should use.
    v = RTSFilter::ProbabilityVector(m_currentFrame.rotationProbability(sinceFrame),
                                     m_currentFrame.translationProbability(sinceFrame),
                                     m_currentFrame.scaleProbability(sinceFrame));
    for (unsigned int i = 0; i < RTS__CATEGORY_COUNT; ++i) {
      probability += v[i];
    }
  }
  assert(probability == 0.0 || abs(probability - 1.0) < 0.00001);
  if (probability == 0.0) {
    return RTSFilter::ambiguousProbabilityVector();
  } else {
    return v;
  }
}

bool OutputPeripheralMode::pointableCountIsUnambiguous() const{
  return m_filteredPointableCount->filteredCategoryIsUnambiguous();
}

int OutputPeripheralMode::pointableCount() const {
  return static_cast<int>(m_filteredPointableCount->filteredCategory());
}

Pointable::Zone OutputPeripheralMode::collectiveZone() const {
  return m_collectiveZone;
}

bool OutputPeripheralMode::deviceToScreen(const Vector& position, Vector& output, Vector& clampVec, float scale, bool clamp) {
  return m_outputPeripheral.deviceToScreen(position, output, clampVec, scale, clamp);
}

bool OutputPeripheralMode::normalizedToScreen(const Vector& position, Vector& output, Vector& clampVec, float scale, bool clamp) {
  return m_outputPeripheral.normalizedToScreen(position, output, clampVec, scale, clamp);
}

bool OutputPeripheralMode::normalizedToAspect(const Vector& position, Vector& output, Vector& clampVec, float scale, bool clamp) {
  return m_outputPeripheral.normalizedToAspect(position, output, clampVec, scale, clamp);
}

void OutputPeripheralMode::setCursorPosition(float fx, float fy, bool absolute) {
  m_outputPeripheral.setCursorPosition(fx, fy, absolute);
}

void OutputPeripheralMode::clickDown(int button, int number) {
  m_outputPeripheral.clickDown(button, number);
}

void OutputPeripheralMode::clickUp(int button, int number) {
  m_outputPeripheral.clickUp(button, number);
}

bool OutputPeripheralMode::beginGesture(uint32_t gestureType) {
  return m_outputPeripheral.beginGesture(gestureType);
}

bool OutputPeripheralMode::endGesture() {
  return m_outputPeripheral.endGesture();
}

bool OutputPeripheralMode::applyZoom(float zoom) {
  return m_outputPeripheral.applyZoom(zoom);
}

bool OutputPeripheralMode::applyRotation(float rotation) {
  return m_outputPeripheral.applyRotation(rotation);
}

bool OutputPeripheralMode::applyScroll(float dx, float dy, int64_t timeDiff) {
  return m_outputPeripheral.applyScroll(dx, dy, timeDiff);
}

bool OutputPeripheralMode::applyDesktopSwipe(float dx, float dy) {
  return m_outputPeripheral.applyDesktopSwipe(dx, dy);
}

void OutputPeripheralMode::applyCharms(const Vector& aspectNormalized, int numPointablesActive, int& charmsMode) {
#if _WIN32
  m_outputPeripheral.applyCharms(aspectNormalized, numPointablesActive, charmsMode);
#endif
}

float OutputPeripheralMode::acceptableClampDistance() const {
  return m_outputPeripheral.acceptableClampDistance();
}

void OutputPeripheralMode::setIconVisibility(int index, bool visible) {
  m_outputPeripheral.setIconVisibility(index, visible);
}

int OutputPeripheralMode::findImageIndex(float z, float touchThreshold, float touchRange) const {
  return m_outputPeripheral.findImageIndex(z, touchThreshold, touchRange);
}

void OutputPeripheralMode::drawImageIcon(int iconIndex, int imageIndex, float x, float y, bool visible) {
  m_outputPeripheral.drawImageIcon(iconIndex, imageIndex, x, y, visible);
}


void OutputPeripheralMode::drawOverlayForPointable (const Pointable &pointable, int32_t iconIndex, float alphaMult, bool deltaTracked) {
  if (!pointable.isValid()) {
    return; // can't do anything in this case
  }

  Vector screenPosition, clampVec, position;
  if (deltaTracked) {
    m_positionalDeltaTracker.setPositionToStabilizedPositionOf(pointable);
    position = m_positionalDeltaTracker.getTrackedPosition();
  } else {
    position = pointable.stabilizedTipPosition();
  }

  normalizedToScreen(interactionBox().normalizePoint(position, false), screenPosition, clampVec);
  if (useProceduralOverlay()) {
    float touchDistance = pointable.touchDistance();
    double radius = touchDistanceToRadius(touchDistance);
    Vector3 vel = pointable.tipVelocity().toVector3<Vector3>();
    float clampDist = clampVec.magnitude();
    if (clampDist > 0) {
      if (fabs(clampVec.x) > 0) {
        vel.x() *= 0;
        vel.y() += 10000 * clampDist;
      } else if (fabs(clampVec.y) > 0) {
        vel.x() += 10000 * clampDist;
        vel.y() *= 0;
      }
    }
    if (touchDistance < 0.0) {
      radius = 10.0;
    }
    drawRasterIcon(iconIndex,
                   screenPosition.x,
                   screenPosition.y,
                   true,
                   vel,
                   touchDistance,
                   radius,
                   clampDist,
                   alphaMult*alphaFromTimeVisible(pointable.timeVisible()));
  } else {
    int imageIndex = findImageIndex(pointable.touchDistance(), 0, 1);
    drawImageIcon(pointable.id(), imageIndex, screenPosition.x, screenPosition.y, true);
  }
}

void OutputPeripheralMode::drawOverlayForHand (const Hand &hand, int32_t iconIndex, float alphaMult) {
  if (!hand.isValid()) {
    return; // can't do anything in this case
  }
  
  Vector screenPosition, clampVec;
  normalizedToScreen(interactionBox().normalizePoint(m_positionalDeltaTracker.getTrackedPosition(), false), screenPosition, clampVec);
  Vector3 vel = hand.palmVelocity().toVector3<Vector3>();
  //Use the most forward touch distance
  float touchDistance = 1.0;
  Leap::PointableList pointables = hand.pointables();
  for (int i=0; i<pointables.count(); i++) {
    touchDistance = std::min(touchDistance, pointables[i].touchDistance());
  }
  double radius = touchDistanceToRadius(touchDistance);

  float clampDist = clampVec.magnitude();
  if (clampDist > 0) {
    if (fabs(clampVec.x) > 0) {
      vel.x() *= 0;
      vel.y() += 10000 * clampDist;
    } else if (fabs(clampVec.y) > 0) {
      vel.x() += 10000 * clampDist;
      vel.y() *= 0;
    }
  }
  if (touchDistance < 0.0) {
    radius = 10.0;
  }
  drawRasterIcon(iconIndex,
                 screenPosition.x,
                 screenPosition.y,
                 true,
                 vel,
                 touchDistance,
                 radius,
                 clampDist,
                 alphaMult*alphaFromTimeVisible(hand.timeVisible()));
}


void OutputPeripheralMode::drawGestureOverlayForHand (const Hand& hand, float rotationAngle, float scaleFactor, float alphaMult, const Vector *positionOverride, bool doubleHorizontalDots, bool doubleVerticalDots, bool verticalMovementGlow, bool horizontalMovementGlow) {
  assert(scaleFactor >= 0.0f);

  Vector screenPosition, clampVec, tipVelocity;
  tipVelocity = hand.palmVelocity();

  Vector position;
  // if positionOverride is specified, use that
  if (positionOverride != nullptr) {
    position = *positionOverride;
  } else {
    // otherwise, if the pointable's hand is valid, use its stabilized palm position
    position = m_positionalDeltaTracker.getTrackedPosition();
  }
  normalizedToScreen(interactionBox().normalizePoint(position), screenPosition, clampVec);

  //Use the most forward touch distance
  float touchDistance = 1.0;
  Leap::PointableList pointables = hand.pointables();
  if (!hand.isValid() || pointables.count() == 0) {
    touchDistance = 0.2f;
  } else {
    for (int i=0; i<pointables.count(); i++) {
      touchDistance = std::min(touchDistance, pointables[i].touchDistance());
    }
  }
  
  
  double radius = touchDistanceToRadius(touchDistance);
  float alpha = alphaMult * alphaFromTimeVisible(hand.timeVisible());

  drawGestureOverlayCore(screenPosition, clampVec, tipVelocity, radius, touchDistance, alpha, rotationAngle, scaleFactor, alphaMult, doubleHorizontalDots, doubleVerticalDots, verticalMovementGlow, horizontalMovementGlow);
}

void OutputPeripheralMode::drawGestureOverlayForPointable (const Pointable& pointable, float rotationAngle, float scaleFactor, float alphaMult, const Vector *positionOverride, bool doubleHorizontalDots, bool doubleVerticalDots, bool verticalMovementGlow, bool horizontalMovementGlow) {
  assert(scaleFactor >= 0.0f);

  Vector screenPosition, clampVec, tipVelocity;
  tipVelocity = pointable.tipVelocity();

  Vector position;
  // if positionOverride is specified, use that
  if (positionOverride != nullptr) {
    position = *positionOverride;
  } else {
    // otherwise, if the pointable's hand is valid, use its stabilized palm position
    position = m_positionalDeltaTracker.getTrackedPosition();
  }
  normalizedToScreen(interactionBox().normalizePoint(position), screenPosition, clampVec);

  //Use the most forward touch distance
  float touchDistance = pointable.touchDistance();
  double radius = touchDistanceToRadius(touchDistance);
  float alpha = alphaMult * alphaFromTimeVisible(pointable.timeVisible());

  drawGestureOverlayCore(screenPosition, clampVec, tipVelocity, radius, touchDistance, alpha, rotationAngle, scaleFactor, alphaMult, doubleHorizontalDots, doubleVerticalDots, verticalMovementGlow, horizontalMovementGlow);
}

void OutputPeripheralMode::drawGestureOverlayCore(Vector screenPosition, Vector clampVec, Vector tipVelocity, double radius, float touchDistance, float alpha, float rotationAngle, float scaleFactor, float alphaMult, bool doubleHorizontalDots, bool doubleVerticalDots, bool verticalMovementGlow, bool horizontalMovementGlow) {
  const double velocityLevel = 30;
  if (useProceduralOverlay()) {
    float xdistance1 = std::abs(touchDistance);
    float xdistance2 = std::abs(touchDistance);
    float ydistance1 = std::abs(touchDistance);
    float ydistance2 = std::abs(touchDistance);
    if (touchDistance < 0.0) {
      if (verticalMovementGlow) {
        if (tipVelocity.y > velocityLevel ) { ydistance1 = -0.5f; }
        if (tipVelocity.y < -velocityLevel) { ydistance2 = -0.5f; }
      }
      if (horizontalMovementGlow)
      {
        if (tipVelocity.x > velocityLevel ) { xdistance1 = -0.5f; }
        if (tipVelocity.x < -velocityLevel) { xdistance2 = -0.5f; }

      }
      radius = 10.0;
    }

    drawRasterIcon(0,
                   screenPosition.x,
                   screenPosition.y,
                   true,
                   Vector3::Zero(),
                   touchDistance,
                   radius,
                   clampVec.magnitude(),
                   alpha);

    float offset = static_cast<float>(std::max((1.0f + scaleFactor) * radius, 25.0));
    assert(!doubleHorizontalDots || !doubleVerticalDots); // can't have both
    if (!doubleHorizontalDots && !doubleVerticalDots) {
      drawRasterIcon(1,
                     screenPosition.x + cos(rotationAngle) * offset, // rotate from positive X axis
                     screenPosition.y + sin(rotationAngle) * offset,
                     true,
                     Vector3::Zero(),
                     xdistance1,
                     5.0,
                     clampVec.magnitude(),
                     alpha);
      drawRasterIcon(2,
                     screenPosition.x - cos(rotationAngle) * offset, // rotate from negative X axis
                     screenPosition.y - sin(rotationAngle) * offset,
                     true,
                     Vector3::Zero(),
                     xdistance2,
                     5.0,
                     clampVec.magnitude(),
                     alpha);
      drawRasterIcon(3,
                     screenPosition.x - sin(rotationAngle) * offset, // rotate from positive Y axis
                     screenPosition.y + cos(rotationAngle) * offset,
                     true,
                     Vector3::Zero(),
                     ydistance2,
                     5.0,
                     clampVec.magnitude(),
                     alpha);
      drawRasterIcon(4,
                     screenPosition.x + sin(rotationAngle) * offset, // rotate from negative Y axis
                     screenPosition.y - cos(rotationAngle) * offset,
                     true,
                     Vector3::Zero(),
                     ydistance1,
                     5.0,
                     clampVec.magnitude(),
                     alpha);
    } else if (doubleHorizontalDots) {
      drawRasterIcon(1,
                     screenPosition.x + cos(rotationAngle) * offset, // rotate from positive X axis
                     screenPosition.y + sin(rotationAngle) * offset,
                     true,
                     Vector3::Zero(),
                     xdistance1,
                     5.0,
                     clampVec.magnitude(),
                     alpha);
      drawRasterIcon(2,
                     screenPosition.x - cos(rotationAngle) * offset, // rotate from negative X axis
                     screenPosition.y - sin(rotationAngle) * offset,
                     true,
                     Vector3::Zero(),
                     xdistance2,
                     5.0,
                     clampVec.magnitude(),
                     alpha);
      drawRasterIcon(3,
                     screenPosition.x + 2.0f * cos(rotationAngle) * offset, // rotate from positive X axis
                     screenPosition.y + 2.0f * sin(rotationAngle) * offset,
                     true,
                     Vector3::Zero(),
                     xdistance1,
                     5.0,
                     clampVec.magnitude(),
                     alpha);
      drawRasterIcon(4,
                     screenPosition.x - 2.0f * cos(rotationAngle) * offset, // rotate from negative X axis
                     screenPosition.y - 2.0f * sin(rotationAngle) * offset,
                     true,
                     Vector3::Zero(),
                     xdistance2,
                     5.0,
                     clampVec.magnitude(),
                     alpha);
    } else {
      drawRasterIcon(1,
                     screenPosition.x - sin(rotationAngle) * offset, // rotate from positive Y axis
                     screenPosition.y + cos(rotationAngle) * offset,
                     true,
                     Vector3::Zero(),
                     ydistance2,
                     5.0,
                     clampVec.magnitude(),
                     alpha);
      drawRasterIcon(2,
                     screenPosition.x + sin(rotationAngle) * offset, // rotate from negative Y axis
                     screenPosition.y - cos(rotationAngle) * offset,
                     true,
                     Vector3::Zero(),
                     ydistance1,
                     5.0,
                     clampVec.magnitude(),
                     alpha);
      drawRasterIcon(3,
                     screenPosition.x - 2.0f * sin(rotationAngle) * offset, // rotate from positive Y axis
                     screenPosition.y + 2.0f * cos(rotationAngle) * offset,
                     true,
                     Vector3::Zero(),
                     ydistance2,
                     5.0,
                     clampVec.magnitude(),
                     alpha);
      drawRasterIcon(4,
                     screenPosition.x + 2.0f * sin(rotationAngle) * offset, // rotate from negative Y axis
                     screenPosition.y - 2.0f * cos(rotationAngle) * offset,
                     true,
                     Vector3::Zero(),
                     ydistance1,
                     5.0,
                     clampVec.magnitude(),
                     alpha);
    }
  } else {
    int imageIndex = findImageIndex(touchDistance, 0, 1);
    drawImageIcon(0, imageIndex, screenPosition.x, screenPosition.y, true);
  }
}

void OutputPeripheralMode::drawScrollOverlayForHand (const Hand& hand, float alphaMult, const Vector *positionOverride, bool doubleHorizontalDots, bool doubleVerticalDots, bool verticalMovementGlow, bool horizontalMovementGlow) {
  // no rotation, unit scale
  drawGestureOverlayForHand(hand, 0.0f, 1.0f, alphaMult, positionOverride, doubleHorizontalDots, doubleVerticalDots, verticalMovementGlow, horizontalMovementGlow);
}

void OutputPeripheralMode::drawScrollOverlayForPointable (const Pointable& pointable, float alphaMult, const Vector *positionOverride, bool doubleHorizontalDots, bool doubleVerticalDots, bool verticalMovementGlow, bool horizontalMovementGlow) {
  // no rotation, unit scale
  drawGestureOverlayForPointable(pointable, 0.0f, 1.0f, alphaMult, positionOverride, doubleHorizontalDots, doubleVerticalDots, verticalMovementGlow, horizontalMovementGlow);
}

void OutputPeripheralMode::drawRotateOverlayForHand (const Hand& hand, float alphaMult, const Vector *positionOverride) {
  float rotationRate = 0.0f;
  auto it = m_timedFrameHistory.getFrameHavingAgeAtLeast(500*MILLISECONDS);
  if (it != m_timedFrameHistory.end()) {
    const Frame &frame = it->second;
    float timeDelta = float(m_currentFrame.timestamp() - frame.timestamp()) / SECONDS;
    rotationRate = m_currentFrame.rotationAngle(frame, Vector::zAxis()) / timeDelta;
  }
  // unit scale
  drawGestureOverlayForHand(hand, rotationRate, 1.0f, alphaMult, positionOverride, false, false, false, false);
}

void OutputPeripheralMode::drawRotateOverlayForPointable (const Pointable& pointable, float alphaMult, const Vector *positionOverride) {
  float rotationRate = 0.0f;
  auto it = m_timedFrameHistory.getFrameHavingAgeAtLeast(500*MILLISECONDS);
  if (it != m_timedFrameHistory.end()) {
    const Frame &frame = it->second;
    float timeDelta = float(m_currentFrame.timestamp() - frame.timestamp()) / SECONDS;
    rotationRate = m_currentFrame.rotationAngle(frame, Vector::zAxis()) / timeDelta;
  }
  // unit scale
  drawGestureOverlayForPointable(pointable, rotationRate, 1.0f, alphaMult, positionOverride, false, false, false, false);
}

void OutputPeripheralMode::drawZoomOverlayForHand (const Hand& hand, float alphaMult, const Vector *positionOverride) {
  float scaleFactor = 1.0f;
  auto it = m_timedFrameHistory.getFrameHavingAgeAtLeast(500*MILLISECONDS);
  if (it != m_timedFrameHistory.end()) {
    const Frame &frame = it->second;
    scaleFactor = m_currentFrame.scaleFactor(frame);
  }
  // no rotation
  drawGestureOverlayForHand(hand, 0.0f, scaleFactor, alphaMult, positionOverride, false, false, false, false);
}

void OutputPeripheralMode::drawZoomOverlayForPointable (const Pointable& pointable, float alphaMult, const Vector *positionOverride) {
  float scaleFactor = 1.0f;
  auto it = m_timedFrameHistory.getFrameHavingAgeAtLeast(500*MILLISECONDS);
  if (it != m_timedFrameHistory.end()) {
    const Frame &frame = it->second;
    scaleFactor = m_currentFrame.scaleFactor(frame);
  }
  // no rotation
  drawGestureOverlayForPointable(pointable, 0.0f, scaleFactor, alphaMult, positionOverride, false, false, false, false);
}

void OutputPeripheralMode::addTouchPointForHand (const Hand &hand, bool touching) {
  if (!hand.isValid()) {
    return;
  }
  Vector screenPosition, clampVec;
  m_positionalDeltaTracker.setPositionToStabilizedPositionOf(hand);
  Vector position = m_positionalDeltaTracker.getTrackedPosition();
  normalizedToScreen(interactionBox().normalizePoint(position), screenPosition, clampVec);
  int32_t touchId = hand.id();
  // untouch at the borders
  if (clampVec.magnitudeSquared() == 0.0f) {
    addTouchPoint(touchId, screenPosition.x, screenPosition.y, touching);
  }
}

void OutputPeripheralMode::addTouchPointForPointable (int touchId, const Pointable &pointable, bool touching, bool deltaTracked) {
  if (!pointable.isValid()) {
    return;
  }
  Vector screenPosition, clampVec, position;
  if (deltaTracked) {
    m_positionalDeltaTracker.setPositionToStabilizedPositionOf(pointable);
    position = m_positionalDeltaTracker.getTrackedPosition();
  } else {
    position = pointable.stabilizedTipPosition();
  }
  normalizedToScreen(interactionBox().normalizePoint(position), screenPosition, clampVec);
  // untouch at the borders
  if (clampVec.magnitudeSquared() == 0.0f) {
    addTouchPoint(touchId, screenPosition.x, screenPosition.y, touching);
  }
}

void OutputPeripheralMode::setAbsoluteCursorPosition (const Vector &deviceCoordinatePosition, Vector *calculatedScreenPosition) {
  Vector screenPosition, clampVec;
  if (normalizedToScreen(interactionBox().normalizePoint(deviceCoordinatePosition),
                         screenPosition,
                         clampVec)) {
    setCursorPosition(screenPosition.x, screenPosition.y, true); // true indicates use of absolute positioning.
    if (calculatedScreenPosition != nullptr) {
      *calculatedScreenPosition = screenPosition;
    }
  }
}

void OutputPeripheralMode::setAbsoluteCursorPositionHand (const Hand &hand, Vector *calculatedScreenPosition) {
  if (hand.isValid()) {
    m_positionalDeltaTracker.setPositionToStabilizedPositionOf(hand);
    setAbsoluteCursorPosition(m_positionalDeltaTracker.getTrackedPosition(), calculatedScreenPosition);
  }
}

void OutputPeripheralMode::setAbsoluteCursorPositionPointable (const Pointable &pointable, Vector *calculatedScreenPosition) {
  if (pointable.isValid()) {
    m_positionalDeltaTracker.setPositionToStabilizedPositionOf(pointable);
    setAbsoluteCursorPosition(m_positionalDeltaTracker.getTrackedPosition(), calculatedScreenPosition);
  }
}

float OutputPeripheralMode::alphaFromTimeVisible(float timeVisible) const {
  static const float VISIBILITY_WARMUP_TIME = 0.4f;
  return std::min(1.0f, timeVisible/VISIBILITY_WARMUP_TIME);
}

float OutputPeripheralMode::scrollDampingFactor (const Vector &scrollVelocity) {
  static const float SPEED_THRESHOLD = 60.0f; // TODO: tune this
  float speed = scrollVelocity.magnitude();
  // use a cubic polynomial to damp out small motions
  float s = speed / SPEED_THRESHOLD;
  float r = 1.0f - s;
  float scaleFactor;
  if (s < 1.0f) {
    scaleFactor = (2.0f*r + s)*s*s;
  } else {
    scaleFactor = 1.0f;
  }
  //std::cerr << "speed = " << speed << " --> scaleFactor = " << scaleFactor << '\n';
  return scaleFactor;
}

double OutputPeripheralMode::touchDistanceToRadius (float touchDistance) {
  return OutputPeripheralImplementation::touchDistanceToRadius(touchDistance);
}

void OutputPeripheralMode::drawRasterIcon(int iconIndex, float x, float y, bool visible, const Vector3& velocity, float touchDistance, double radius, float clampDistance, float alphaMult, int numFingers) {
  m_outputPeripheral.drawRasterIcon(iconIndex, x, y, visible, velocity, touchDistance, radius, clampDistance, alphaMult, numFingers);
}

// bool OutputPeripheralMode::checkTouching(const Vector& position, float noTouchBorder) const {
//   return m_outputPeripheral.checkTouching(position, noTouchBorder);
// }

void OutputPeripheralMode::clearTouchPoints() {
  m_outputPeripheral.clearTouchPoints();
}

void OutputPeripheralMode::removeTouchPoint(int touchId) {
  m_outputPeripheral.removeTouchPoint(touchId);
}

void OutputPeripheralMode::addTouchPoint(int touchId, float x, float y, bool touching) {
  m_outputPeripheral.addTouchPoint(touchId, x, y, touching);
}

void OutputPeripheralMode::emitTouchEvent() {
  m_outputPeripheral.emitTouchEvent();
}

bool OutputPeripheralMode::touchAvailable() const {
  return m_outputPeripheral.touchAvailable();
}

int OutputPeripheralMode::numTouchScreens() const {
  return m_outputPeripheral.numTouchScreens();
}

int OutputPeripheralMode::touchVersion() const {
  return m_outputPeripheral.touchVersion();
}

bool OutputPeripheralMode::useProceduralOverlay() const {
  return m_outputPeripheral.useProceduralOverlay();
}

bool OutputPeripheralMode::useCharmHelper() const {
  return m_outputPeripheral.useCharmHelper();
}

#if __APPLE__
void OutputPeripheralMode::flushOverlay() {
  if (m_flushOverlay) {
    m_outputPeripheral.flushOverlay();
  }
}
#endif

void OutputPeripheralMode::setForemostPointable (const std::vector<Pointable> &relevantPointables, int32_t &foremostPointableId) const {
  if ((m_filteredPointableCount->filteredCategoryIsUnambiguous() && m_filteredPointableCount->filteredCategory() == m_relevantPointables.size())
      || !m_filteredPointableCount->filteredCategoryIsUnambiguous()) {
    identifyForemostPointable(relevantPointables, foremostPointableId);
  } else {
    foremostPointableId = -1;
  }
}

void OutputPeripheralMode::stopActiveEvents()
{
  return;
}

void OutputPeripheralMode::identifyForemostPointable (const std::vector<Pointable> &relevantPointables, int32_t &foremostPointableId) const {
  if (relevantPointables.empty()) {
    foremostPointableId = -1;
    return;
  }

  auto best = relevantPointables.begin();
  for (auto it = relevantPointables.begin(); it < relevantPointables.end(); ++it) {
    if (it->stabilizedTipPosition().z < best->stabilizedTipPosition().z) {
      best = it;
    }
  }

  if (best == relevantPointables.end()) {
    foremostPointableId = -1;
  } else {
    foremostPointableId = best->id();
  }
}

}
