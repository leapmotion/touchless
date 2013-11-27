/*==================================================================================================================

    Copyright (c) 2010 - 2013 Leap Motion. All rights reserved.

  The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
  protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
  strictly forbidden unless prior written permission is obtained from Leap Motion.

===================================================================================================================*/

#if !defined(__LPGesture_h__)
#define __LPGesture_h__

#include "LPGeometry.h"
#include "ocuConfig.h"
#include "Heartbeat.h"
#include "BoundedQueue.h"

#include <boost/thread.hpp>
#include <stdint.h>

#if __APPLE__
#include <ApplicationServices/ApplicationServices.h>

enum {
  kIOHIDDigitizerEventRange      = 0x00000001,
  kIOHIDDigitizerEventTouch      = 0x00000002,
  kIOHIDDigitizerEventPosition   = 0x00000004,
  kIOHIDDigitizerEventStop       = 0x00000008,
  kIOHIDDigitizerEventPeak       = 0x00000010,
  kIOHIDDigitizerEventIdentity   = 0x00000020,
  kIOHIDDigitizerEventAttribute  = 0x00000040,
  kIOHIDDigitizerEventCancel     = 0x00000080,
  kIOHIDDigitizerEventStart      = 0x00000100,
  kIOHIDDigitizerEventResting    = 0x00000200,
  kIOHIDDigitizerEventSwipeUp    = 0x01000000,
  kIOHIDDigitizerEventSwipeDown  = 0x02000000,
  kIOHIDDigitizerEventSwipeLeft  = 0x04000000,
  kIOHIDDigitizerEventSwipeRight = 0x08000000,
  kIOHIDDigitizerEventSwipeMask  = 0xFF000000
};
typedef uint32_t IOHIDDigitizerEventMask;
#endif

enum {
  kIOHIDGestureMotionNone,
  kIOHIDGestureMotionHorizontalX,
  kIOHIDGestureMotionVerticalY,
  kIOHIDGestureMotionScale,
  kIOHIDGestureMotionRotate,
  kIOHIDGestureMotionTap,
  kIOHIDGestureMotionDoubleTap,
  kIOHIDGestureMotionFromLeftEdge,
  kIOHIDGestureMotionOffLeftEdge,
  kIOHIDGestureMotionFromRightEdge,
  kIOHIDGestureMotionOffRightEdge,
  kIOHIDGestureMotionFromTopEdge,
  kIOHIDGestureMotionOffTopEdge,
  kIOHIDGestureMotionFromBottomEdge,
  kIOHIDGestureMotionOffBottomEdge,
};
typedef uint16_t IOHIDGestureMotion;

enum {
  kIOHIDSwipeNone     = 0x00000000,
  kIOHIDSwipeUp       = 0x00000001,
  kIOHIDSwipeDown     = 0x00000002,
  kIOHIDSwipeLeft     = 0x00000004,
  kIOHIDSwipeRight    = 0x00000008,
  kIOHIDScaleExpand   = 0x00000010,
  kIOHIDScaleContract = 0x00000020,
  kIOHIDRotateCW      = 0x00000040,
  kIOHIDRotateCCW     = 0x00000080,
};
typedef uint32_t IOHIDSwipeMask;

enum {
  kIOHIDEventTypeNULL = 0,
  kIOHIDEventTypeVendorDefined = 1,
  kIOHIDEventTypeRotation = 5,
  kIOHIDEventTypeScroll = 6,
  kIOHIDEventTypeZoom = 8,
  kIOHIDEventTypeVelocity = 9,
  kIOHIDEventTypeDigitizer = 11,
  kIOHIDEventTypeMouse = 18,
  kIOHIDEventTypeDockSwipe = 23,
  kIOHIDEventTypeFluidTouchGesture = 27,
  kIOHIDEventTypeGestureBegan = 61,
  kIOHIDEventTypeGestureEnded = 62
};
typedef uint32_t IOHIDEventType;

enum {
  kIOHIDEventPhaseUndefined        = 0x00,
  kIOHIDEventPhaseBegan            = 0x01,
  kIOHIDEventPhaseChanged          = 0x02,
  kIOHIDEventPhaseEnded            = 0x04,
  kIOHIDEventPhaseCancelled        = 0x08,
  kIOHIDEventPhaseMayBegin         = 0x80,
  kIOHIDEventEventPhaseMask        = 0xFF,
  kIOHIDEventEventOptionPhaseShift = 24
};
typedef uint16_t IOHIDEventPhaseBits;

enum {
  kIOHIDEventMomentumPhaseUndefined = 0,
  kIOHIDEventMomentumPhaseBegan     = 1,
  kIOHIDEventMomentumPhaseChanged   = 2,
  kIOHIDEventMomentumPhaseEnded     = 3
};
typedef uint32_t IOHIDEventMomentumPhase;

class LPGesture {
  public:
    LPGesture(float fx = 0, float fy = 0);
    ~LPGesture();

    static const uint32_t GestureNone                   = 0U;
    static const uint32_t GestureScroll                 = 1U;
    static const uint32_t GestureZoom                   = 2U;
    static const uint32_t GestureRotate                 = 3U;
    static const uint32_t GestureDesktopSwipeHorizontal = 4U;
    static const uint32_t GestureDesktopSwipeVertical   = 5U;

    uint32_t type() const { return m_type; }

    void setPosition(float fx, float fy) {
      m_position.x = static_cast<LPFloat>(fx);
      m_position.y = static_cast<LPFloat>(fy);
    }

    bool begin(uint32_t gestureType);
    bool end();
    bool applyZoom(float zoom);
    bool applyRotation(float rotation);
    bool applyDesktopSwipe(float dx, float dy);
    bool applyScroll(float dx, float dy, int64_t timeDiff = 0);

  private:
    LPGesture(const LPGesture&);
    LPGesture& operator=(const LPGesture&);
    void applyMomentum();

    static IOHIDEventType getEventTypeFromGestureType(uint32_t type);

#if __APPLE__
    CGEventRef createEvent(IOHIDEventType type) const;
    IOHIDDigitizerEventMask phaseToEventMask(IOHIDEventPhaseBits phase) const;

    IOHIDDigitizerEventMask m_eventMask;
#elif _WIN32
    int64_t m_windowsEventTimer;
#endif
    uint32_t m_type;
    LPPoint m_position;
    Heartbeat m_momentumTimer;
    float m_momentumFPS;
    BoundedQueue<LPPoint, 11> m_momentumHistory;
    LPPoint m_momentum;
    IOHIDEventPhaseBits m_phase;
    IOHIDEventMomentumPhase m_momentumPhase;
    boost::mutex m_scrollMutex;
    LPPoint m_scrollPartialLine;
    LPPoint m_scrollPartialPixel;
    float m_horizontalPixelsPerLine;
    float m_verticalPixelsPerLine;
    float m_desktopSwipeDistance;
    float m_desktopSwipeDistanceDelta;
    float m_desktopSwipeDistanceThreshold;
    IOHIDSwipeMask m_desktopSwipeDirection;
};

#endif // __LPGesture_h__
