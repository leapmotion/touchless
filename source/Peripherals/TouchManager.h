/*==================================================================================================================

    Copyright (c) 2010 - 2013 Leap Motion. All rights reserved.

  The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
  protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
  strictly forbidden unless prior written permission is obtained from Leap Motion.

===================================================================================================================*/

#if !defined(_TouchManager_h_)
#define _TouchManager_h_

#include "Touch.h"

#if _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include "OcuHidInstance.h"

// If building with pre-"Windows SDK 8.0" include headers, define the touch interface ourselves
#if !defined(MAX_TOUCH_COUNT)
enum tagPOINTER_INPUT_TYPE {
      PT_POINTER = 0x00000001,    // Generic pointer
      PT_TOUCH   = 0x00000002,    // Touch
      PT_PEN     = 0x00000003,    // Pen
      PT_MOUSE   = 0x00000004,    // Mouse
};
typedef DWORD POINTER_INPUT_TYPE;

typedef UINT32 POINTER_FLAGS;

#define POINTER_FLAG_NONE               0x00000000 // Default
#define POINTER_FLAG_INRANGE            0x00000002 // Pointer has not departed
#define POINTER_FLAG_INCONTACT          0x00000004 // Pointer is in contact
#define POINTER_FLAG_DOWN               0x00010000 // Pointer transitioned to down state (made contact)
#define POINTER_FLAG_UPDATE             0x00020000 // Pointer update
#define POINTER_FLAG_UP                 0x00040000 // Pointer transitioned from down state (broke contact)

typedef enum tagPOINTER_BUTTON_CHANGE_TYPE {
    POINTER_CHANGE_NONE,
    POINTER_CHANGE_FIRSTBUTTON_DOWN,
    POINTER_CHANGE_FIRSTBUTTON_UP,
    POINTER_CHANGE_SECONDBUTTON_DOWN,
    POINTER_CHANGE_SECONDBUTTON_UP,
    POINTER_CHANGE_THIRDBUTTON_DOWN,
    POINTER_CHANGE_THIRDBUTTON_UP,
    POINTER_CHANGE_FOURTHBUTTON_DOWN,
    POINTER_CHANGE_FOURTHBUTTON_UP,
    POINTER_CHANGE_FIFTHBUTTON_DOWN,
    POINTER_CHANGE_FIFTHBUTTON_UP,
} POINTER_BUTTON_CHANGE_TYPE;

typedef struct tagPOINTER_INFO {
    POINTER_INPUT_TYPE         pointerType;
    UINT32                     pointerId;
    UINT32                     frameId;
    POINTER_FLAGS              pointerFlags;
    HANDLE                     sourceDevice;
    HWND                       hwndTarget;
    POINT                      ptPixelLocation;
    POINT                      ptHimetricLocation;
    POINT                      ptPixelLocationRaw;
    POINT                      ptHimetricLocationRaw;
    DWORD                      dwTime;
    UINT32                     historyCount;
    INT32                      InputData;
    DWORD                      dwKeyStates;
    UINT64                     PerformanceCount;
    POINTER_BUTTON_CHANGE_TYPE ButtonChangeType;
} POINTER_INFO;

typedef UINT32 TOUCH_FLAGS;
#define TOUCH_FLAG_NONE                 0x0000000 // Default

typedef UINT32 TOUCH_MASK;
#define TOUCH_MASK_NONE                 0x0000000 // Default - none of the optional fields are valid
#define TOUCH_MASK_CONTACTAREA          0x0000001 // The rcContact field is valid
#define TOUCH_MASK_ORIENTATION          0x0000002 // The orientation field is valid
#define TOUCH_MASK_PRESSURE             0x0000004 // The pressure field is valid

typedef struct tagPOINTER_TOUCH_INFO {
    POINTER_INFO    pointerInfo;
    TOUCH_FLAGS     touchFlags;
    TOUCH_MASK      touchMask;
    RECT            rcContact;
    RECT            rcContactRaw;
    UINT32          orientation;
    UINT32          pressure;
} POINTER_TOUCH_INFO;

#define MAX_TOUCH_COUNT 256

#define TOUCH_FEEDBACK_DEFAULT 0x1
#define TOUCH_FEEDBACK_INDIRECT 0x2
#define TOUCH_FEEDBACK_NONE 0x3
#endif
#endif

class TouchManager {
  public:
    TouchManager();
    ~TouchManager();

    void clearTouches();
    void setTouches(const std::set<Touch>& touches);
    std::set<Touch> getTouches() const { return m_touches; }
    int getVersion() const { return m_touchVersion; }

    bool supportsTouch() const { return (m_touchVersion > 0); }
    bool expectsNormalizedTouchCoordinates() const { return (m_touchVersion == 7); }

  private:
    std::set<Touch> m_touches;
#if _WIN32
    BOOL (WINAPI *m_injectTouchInput)(UINT32, const POINTER_TOUCH_INFO*);
    COcuHidInstance* m_hidInstance;
    POINTER_TOUCH_INFO m_contacts[MAX_TOUCH_COUNT];
#endif
    int m_touchVersion;
};

#endif // _TouchManager_h_
