/*==================================================================================================================

    Copyright (c) 2010 - 2013 Leap Motion. All rights reserved.

  The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
  protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
  strictly forbidden unless prior written permission is obtained from Leap Motion.

===================================================================================================================*/
#include "stdafx.h"
#include "LPGesture.h"

#if __APPLE__
#include "LPMac.h"
#else
#if _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#else
#include "LPLinux.h"
#endif
#include <math.h>
#endif

//
// LPGesture
//

LPGesture::LPGesture(float fx, float fy) :
#if __APPLE__
  m_eventMask(0),
#elif _WIN32
  m_windowsEventTimer(0),
#endif
  m_position(LPPointMake(fx, fy)), m_type(GestureNone),
  m_momentumTimer(8), m_momentumFPS(1000.0f/8.0f),
  m_momentum(LPPointZero), m_phase(kIOHIDEventPhaseUndefined),
  m_momentumPhase(kIOHIDEventMomentumPhaseUndefined),
  m_scrollPartialLine(LPPointZero), m_scrollPartialPixel(LPPointZero),
  m_verticalPixelsPerLine(1),m_horizontalPixelsPerLine(1),
  m_desktopSwipeDistance(0), m_desktopSwipeDistanceDelta(0),
  m_desktopSwipeDistanceThreshold(0), m_desktopSwipeDirection(kIOHIDSwipeNone)
{
}

LPGesture::~LPGesture()
{
}

IOHIDEventType LPGesture::getEventTypeFromGestureType(uint32_t type)
{
  switch (type) {
    case LPGesture::GestureZoom:
      return kIOHIDEventTypeZoom;
    case LPGesture::GestureRotate:
      return kIOHIDEventTypeRotation;
    case LPGesture::GestureScroll:
      return kIOHIDEventTypeScroll;
    case LPGesture::GestureDesktopSwipeHorizontal:
    case LPGesture::GestureDesktopSwipeVertical:
      return kIOHIDEventTypeDockSwipe;
    default:
      return kIOHIDEventTypeNULL;
  }
}

bool LPGesture::begin(uint32_t type)
{
  if (m_type == GestureNone && getEventTypeFromGestureType(type) != kIOHIDEventTypeNULL) {
    if (m_momentumPhase != kIOHIDEventMomentumPhaseUndefined) {
      m_momentumTimer.Stop();
      applyScroll(0, 0); // Stop any existing momentum scroll
    }
    m_type = type;
    m_momentumPhase = kIOHIDEventMomentumPhaseUndefined;
    m_phase = kIOHIDEventPhaseBegan;
    m_momentumHistory.Reset();
    m_momentum = LPPointZero;
    m_scrollPartialLine = LPPointZero;
    m_scrollPartialPixel = LPPointZero;
#if __APPLE__
    m_horizontalPixelsPerLine = m_verticalPixelsPerLine = 10;
#elif _WIN32
    // For Windows, this is actually lines per scroll wheel notch
    UINT param = 0;
    m_horizontalPixelsPerLine =
      (SystemParametersInfo(SPI_GETWHEELSCROLLCHARS, 0, &param, 0) && (param != 0)) ? static_cast<float>(param) : 3;
    m_verticalPixelsPerLine =
      (SystemParametersInfo(SPI_GETWHEELSCROLLLINES, 0, &param, 0) && (param != 0)) ? static_cast<float>(param) : 3;
#else
    m_horizontalPixelsPerLine = m_verticalPixelsPerLine = 12;
#endif
    m_desktopSwipeDistance = 0;
    m_desktopSwipeDistanceDelta = 0;
    m_desktopSwipeDistanceThreshold = 0;
    m_desktopSwipeDirection = kIOHIDSwipeNone;

#if __APPLE__
    if (m_type != LPGesture::GestureDesktopSwipeHorizontal &&
        m_type != LPGesture::GestureDesktopSwipeVertical) {
      CGEventRef event = createEvent(kIOHIDEventTypeGestureBegan);
      CGEventSetIntegerValueField(event, 115, getEventTypeFromGestureType(m_type)); // 115: begin gesture subtype
      CGEventPost(kCGHIDEventTap, event);
      CFRelease(event);
    }
#endif
    if (m_type == GestureZoom) {
      applyZoom(0);
    } else if (m_type == GestureRotate) {
      applyRotation(0);
    }
    return true;
  }
  return false;
}

bool LPGesture::end()
{
  if (m_type != GestureNone) {
    if (m_phase == kIOHIDEventPhaseBegan || m_phase == kIOHIDEventPhaseChanged) {
      m_phase = kIOHIDEventPhaseEnded;
      if (m_type == GestureZoom) {
        applyZoom(0);
      } else if (m_type == GestureRotate) {
        applyRotation(0);
      }
#if __APPLE__
      IOHIDGestureMotion gestureMotion;
      IOHIDEventType gestureType;

      if (m_type == LPGesture::GestureDesktopSwipeHorizontal) {
        gestureMotion = kIOHIDGestureMotionHorizontalX;
        gestureType = kIOHIDEventTypeDockSwipe;
      } else if (m_type == LPGesture::GestureDesktopSwipeVertical) {
        gestureMotion = kIOHIDGestureMotionVerticalY;
        gestureType = kIOHIDEventTypeDockSwipe;
      } else {
        gestureMotion = kIOHIDGestureMotionNone;
        gestureType = kIOHIDEventTypeGestureEnded;
      }
      CGEventRef event = createEvent(gestureType);
      if (gestureType == kIOHIDEventTypeGestureEnded) {
        CGEventSetIntegerValueField(event, 115, getEventTypeFromGestureType(m_type)); // 115: ended gesture subtype
      } else if (gestureType == kIOHIDEventTypeDockSwipe) {
        // Cancel the swipe if ...
        if (std::fabs(m_desktopSwipeDistance) < 0.5f &&      // ... we didn't move enough and
           (std::fabs(m_desktopSwipeDistanceDelta) < 0.01 || // the last movement was too small or
            m_desktopSwipeDistance*m_desktopSwipeDistanceDelta < 0)) { // it was is in the opposite direction
          m_phase = kIOHIDEventPhaseCancelled;
        }
        CGEventSetDoubleValueField(event, 124, m_desktopSwipeDistance);
        CGEventSetIntegerValueField(event, 132, m_phase);
        CGEventSetIntegerValueField(event, 134, m_phase);
        CGEventSetIntegerValueField(event, 136, 1); // Unknown
        CGEventSetIntegerValueField(event, 138, 3); // Unknown
      }
      CGEventSetIntegerValueField(event, 123, gestureMotion);
      CGEventPost(kCGHIDEventTap, event);
      CFRelease(event);
#endif
      if (m_type == GestureScroll) {
        m_momentumTimer.Stop(); // Shouldn't be running, but just to be safe
        applyScroll(0, 0); // End the gesture
        m_momentum = LPPointZero;
        float maxAbsValue = 0;
        // Average the history of deltas to determine starting momentum value
        int numHistory = 0;
        while (!m_momentumHistory.IsEmpty()) {
          const LPPoint& momentum = m_momentumHistory.Dequeue();
          float curAbsValue = std::fabs(momentum.x);

          if (curAbsValue > maxAbsValue) {
            m_momentum.x = momentum.x;
            m_momentum.y = momentum.y;
            maxAbsValue = curAbsValue;
          }
          curAbsValue = std::fabs(momentum.y);
          if (curAbsValue > maxAbsValue) {
            m_momentum.x = momentum.x;
            m_momentum.y = momentum.y;
            maxAbsValue = curAbsValue;
          }
          numHistory++;
        }
        if (numHistory) {
          m_momentumPhase = kIOHIDEventMomentumPhaseBegan;
          m_momentumTimer.Start(boost::bind(&LPGesture::applyMomentum, this));
        }
      }

      m_type = GestureNone;
      m_phase = kIOHIDEventPhaseUndefined;
      return true;
    }
  }
  return false;
}

bool LPGesture::applyZoom(float zoom)
{
  if (m_type == GestureZoom) {
#if __APPLE__
    CGEventRef event = createEvent(kIOHIDEventTypeZoom);
    CGEventSetDoubleValueField(event, 113, zoom);
    CGEventPost(kCGHIDEventTap, event);
    CFRelease(event);
#elif _WIN32
    // Windows -- FIXME
#endif
    if (m_phase == kIOHIDEventPhaseBegan) {
      m_phase = kIOHIDEventPhaseChanged;
    }
    return true;
  }
  return false;
}

bool LPGesture::applyRotation(float rotation)
{
  if (m_type == GestureRotate) {
#if __APPLE__
    CGEventRef event = createEvent(kIOHIDEventTypeRotation);
    CGEventSetDoubleValueField(event, 113, rotation);
    CGEventPost(kCGHIDEventTap, event);
    CFRelease(event);
#elif _WIN32
    // Windows -- FIXME
#endif
    if (m_phase == kIOHIDEventPhaseBegan) {
      m_phase = kIOHIDEventPhaseChanged;
    }
    return true;
  }
  return false;
}

bool LPGesture::applyDesktopSwipe(float dx, float dy)
{
  boost::unique_lock<boost::mutex> lock(m_scrollMutex);

  if (m_type == GestureDesktopSwipeHorizontal ||
      m_type == GestureDesktopSwipeVertical) {
#if __APPLE__
    CGEventRef event = createEvent(kIOHIDEventTypeDockSwipe);

    static const float scaleUnits = 1.0f/300.0f; // Scale units so they are more in line with those of the trackpad
    dx *= -scaleUnits; // Also negate the direction of the x component to match
    dy *=  scaleUnits;

    IOHIDSwipeMask swipePositive, swipeNegative;
    float swipeTrigger = 0.05f;

    if (m_type == GestureDesktopSwipeVertical) {
      swipePositive = kIOHIDSwipeDown;
      swipeNegative = kIOHIDSwipeUp;
      m_desktopSwipeDistanceDelta = dy;
      CGEventSetIntegerValueField(event, 123, kIOHIDGestureMotionVerticalY);
    } else {
      swipePositive = kIOHIDSwipeLeft;
      swipeNegative = kIOHIDSwipeRight;
      m_desktopSwipeDistanceDelta = dx;
      CGEventSetIntegerValueField(event, 123, kIOHIDGestureMotionHorizontalX);
    }
    m_desktopSwipeDistance += m_desktopSwipeDistanceDelta;

    IOHIDSwipeMask swipe = kIOHIDSwipeNone;

    // Send a swipe trigger if we cross the threshold. The new threshold is
    // adjusted as the swipe continues in the same direction past the old
    // threshold.
    if (m_desktopSwipeDirection == swipeNegative) {
      if (m_desktopSwipeDistance < m_desktopSwipeDistanceThreshold) {
        m_desktopSwipeDistanceThreshold = m_desktopSwipeDistance;
      } else if (m_desktopSwipeDistance >= m_desktopSwipeDistanceThreshold + swipeTrigger) {
        swipe = swipePositive;
      }
    } else if (m_desktopSwipeDirection == swipePositive) {
      if (m_desktopSwipeDistance > m_desktopSwipeDistanceThreshold) {
        m_desktopSwipeDistanceThreshold = m_desktopSwipeDistance;
      } else if (m_desktopSwipeDistance <= m_desktopSwipeDistanceThreshold - swipeTrigger) {
        swipe = swipeNegative;
      }
    } else {
      if (m_desktopSwipeDistance >= swipeTrigger) {
        swipe = swipePositive;
      } else if (m_desktopSwipeDistance <= -swipeTrigger) {
        swipe = swipeNegative;
      }
    }
    if (swipe != kIOHIDSwipeNone) {
      m_desktopSwipeDistanceThreshold = m_desktopSwipeDistance;
      m_desktopSwipeDirection = swipe;
      CGEventSetIntegerValueField(event, 115, swipe);
    }
    CGEventSetDoubleValueField(event, 124, m_desktopSwipeDistance);
    CGEventSetDoubleValueField(event, 125, dx);
    CGEventSetDoubleValueField(event, 126, dy);
    CGEventSetIntegerValueField(event, 132, m_phase);
    CGEventSetIntegerValueField(event, 133, kIOHIDDigitizerEventPosition);
    CGEventSetIntegerValueField(event, 134, m_phase);
    CGEventSetIntegerValueField(event, 136, 1); // Unknown
    CGEventSetIntegerValueField(event, 138, 3); // Unknown
    CGEventPost(kCGHIDEventTap, event);
    CFRelease(event);
#elif _WIN32
    // Windows -- FIXME
#endif
    if (m_phase == kIOHIDEventPhaseBegan) {
      m_phase = kIOHIDEventPhaseChanged;
    }
    return true;
  }
  return false;
}

bool LPGesture::applyScroll(float dx, float dy, int64_t timeDiff)
{
  boost::unique_lock<boost::mutex> lock(m_scrollMutex);

  if (m_type == GestureScroll || m_momentumPhase != kIOHIDEventMomentumPhaseUndefined) {
    const float ppi = 120.0f; // Pixels per inch (base this on the DPI of the monitors -- FIXME)
    const float ppmm = ppi/25.4f; // Convert pixels per inch to pixels per millimeter
    float px = dx*ppmm, py = dy*ppmm; // Convert to pixels
    float lx = px/m_horizontalPixelsPerLine, ly = py/m_verticalPixelsPerLine; // Convert to lines

    if (m_momentumPhase != kIOHIDEventMomentumPhaseUndefined) {
      if (std::fabs(px) < 1.5f && std::fabs(py) < 1.5f) {
        m_momentumHistory.Reset();
        m_momentum = LPPointZero;
        if (m_momentumPhase == kIOHIDEventMomentumPhaseBegan) {
          m_momentumPhase = kIOHIDEventMomentumPhaseUndefined;
          if (m_phase != kIOHIDEventPhaseEnded) {
            return false;
          }
        } else {
          m_momentumPhase = kIOHIDEventMomentumPhaseEnded;
          m_momentumTimer.Stop();
        }
      } else {
        float scale;

        if (m_momentum.x > 0.4f || m_momentum.y > 0.4f) {
          scale = 0.98f;
        } else {
          scale = 0.96f;
        }
        m_momentum.x *= scale;
        m_momentum.y *= scale;
      }
    } else if (m_phase != kIOHIDEventPhaseEnded) {
      if (std::fabs(dx) <= 0.0000001f && std::fabs(dy) <= 0.0000001f) {
        m_momentumHistory.Reset();
        m_momentum = LPPointZero;
        return false;
      }
      if (timeDiff > 0 && timeDiff < 1000000) {
        const float dt_inv = 1000000.0f/static_cast<float>(timeDiff)/m_momentumFPS;
        const LPPoint momentum = LPPointMake(dx*dt_inv, dy*dt_inv);

        if (m_momentumHistory.IsFull()) {
          m_momentumHistory.Dequeue();
        }
        m_momentumHistory.Enqueue(momentum);
      } else {
        m_momentumHistory.Reset();
        m_momentum = LPPointZero;
      }
    }

    // Adjust partial pixels
    m_scrollPartialPixel.x += px;
    m_scrollPartialPixel.y += py;
    px = round(m_scrollPartialPixel.x);
    py = round(m_scrollPartialPixel.y);
    m_scrollPartialPixel.x -= px;
    m_scrollPartialPixel.y -= py;

    // Adjust partial lines
    m_scrollPartialLine.x += lx;
    m_scrollPartialLine.y += ly;

#if _WIN32
    static const int64_t windowsUpdateRate = 100000 / 60; // 60 updates per second
    m_windowsEventTimer += timeDiff;
    if (m_windowsEventTimer >= windowsUpdateRate) {
#endif
      lx = floor(m_scrollPartialLine.x);
      ly = floor(m_scrollPartialLine.y);
      m_scrollPartialLine.x -= lx;
      m_scrollPartialLine.y -= ly;
#if _WIN32
      m_windowsEventTimer = 0;
    } else {
      lx = ly = 0;
    }
#endif
    int ilx = static_cast<int>(lx);
    int ily = static_cast<int>(ly);

#if __APPLE__
    if (m_phase != kIOHIDEventPhaseUndefined) {
      // Scroll Gesture Event (only when gesturing)
      CGEventRef event = createEvent(kIOHIDEventTypeScroll);
      CGEventSetDoubleValueField(event, 113, px);
      CGEventSetDoubleValueField(event, 119, py);
      CGEventSetIntegerValueField(event, 123, 0x80000000); // Swipe direction
      CGEventSetIntegerValueField(event, 132, m_phase);
      CGEventSetIntegerValueField(event, 135, 1); // Unsure what this does
      CGEventPost(kCGHIDEventTap, event);
      CFRelease(event);
    }

    // Scroll Wheel Event
    CGEventRef event = CGEventCreateScrollWheelEvent(0, kCGScrollEventUnitPixel, 2, 0, 0);
    if (ily != 0) {
      CGEventSetIntegerValueField(event, kCGScrollWheelEventDeltaAxis1, ily);
      CGEventSetIntegerValueField(event, kCGScrollWheelEventFixedPtDeltaAxis1, ily*65536);
    }
    if (ilx != 0) {
      CGEventSetIntegerValueField(event, kCGScrollWheelEventDeltaAxis2, ilx);
      CGEventSetIntegerValueField(event, kCGScrollWheelEventFixedPtDeltaAxis2, ilx*65536);
    }
    CGEventSetIntegerValueField(event, kCGScrollWheelEventPointDeltaAxis1, py);
    CGEventSetIntegerValueField(event, kCGScrollWheelEventPointDeltaAxis2, px);

    CGEventSetIntegerValueField(event, 99, m_phase); // phase
    CGEventSetIntegerValueField(event, 123, m_momentumPhase); // momentum phase
    CGEventSetIntegerValueField(event, 137, 1); // Unsure what this does
    CGEventPost(kCGHIDEventTap, event);
    CFRelease(event);
#elif _WIN32
    if (ily != 0) {
      INPUT input = { 0 };
      input.type = INPUT_MOUSE;
      input.mi.dwFlags = MOUSEEVENTF_WHEEL;
      input.mi.mouseData = static_cast<DWORD>(ily*2); // Scale the step (Windows often drops values of -1 and 1)
      SendInput(1, &input, sizeof(input));
    }
    if (ilx != 0) {
      INPUT input = { 0 };
      input.type = INPUT_MOUSE;
      input.mi.dwFlags = MOUSEEVENTF_HWHEEL;
      input.mi.mouseData = static_cast<DWORD>(-ilx*2); // Scale the step (see above); Also reverse direction of scroll
      SendInput(1, &input, sizeof(input));
    }
#else
    // Linux -- FIXME
    (void)ilx; // Unused
    (void)ily; // Unused
#endif
    // Change phase or momentum phase as needed
    if (m_phase == kIOHIDEventPhaseBegan) {
      m_phase = kIOHIDEventPhaseChanged;
    } else if (m_momentumPhase == kIOHIDEventMomentumPhaseBegan) {
      m_momentumPhase = kIOHIDEventMomentumPhaseChanged;
    } else if (m_momentumPhase == kIOHIDEventMomentumPhaseEnded) {
      m_momentumPhase = kIOHIDEventMomentumPhaseUndefined;
    }
    return true;
  }
  return false;
}

void LPGesture::applyMomentum()
{
  if (m_momentumPhase != kIOHIDEventMomentumPhaseUndefined) {
    const int64_t timeDiff = static_cast<int64_t>(1000000.0f/m_momentumFPS);
    applyScroll(m_momentum.x, m_momentum.y, timeDiff);
  }
}

#if __APPLE__

CGEventRef LPGesture::createEvent(IOHIDEventType type) const
{
  CGEventRef event = CGEventCreate(0);
  CGEventSetLocation(event, m_position);
  CGEventTimestamp time = LPMac::systemUptime();
  CGEventSetTimestamp(event, time);
  CGEventSetFlags(event, 0x100);
  CGEventSetType(event, type == kIOHIDEventTypeDockSwipe ? 30 : LPMac::EventTypeGesture); // FIXME
  CGEventSetIntegerValueField(event, 110, type); // 110: event subtype
  if (type != kIOHIDEventTypeGestureBegan &&
      type != kIOHIDEventTypeGestureEnded) {
    CGEventSetIntegerValueField(event, 133, phaseToEventMask(m_phase)); // 133: digitizer event mask
  }
  return event;
}

IOHIDDigitizerEventMask LPGesture::phaseToEventMask(IOHIDEventPhaseBits phase) const
{
  IOHIDDigitizerEventMask mask = 0;
  if (phase & kIOHIDEventPhaseBegan) {
    mask |= kIOHIDDigitizerEventStart;
  }
  if (phase & kIOHIDEventPhaseEnded) {
    mask |= kIOHIDDigitizerEventStop;
  } else {
    if (mask) {
      mask |= kIOHIDDigitizerEventPosition;
    } else {
      mask = m_eventMask;
    }
  }
  return mask;
}

#endif
