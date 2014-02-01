#include "stdafx.h"
#include "TouchManagerWin8.h"
#include "Win8TouchStructures.h"

typedef BOOL (__stdcall *t_NtUserInitializeTouchInjection)(UINT32 maxCount, DWORD dwMode);
static auto NtUserInitializeTouchInjection = (t_NtUserInitializeTouchInjection)GetProcAddress(GetHmodUser32(), "InitializeTouchInjection");

typedef BOOL (__stdcall *t_NtUserInjectTouchInput)(UINT32 nTouches, const POINTER_TOUCH_INFO* touches);
static auto NtUserInjectTouchInput = (t_NtUserInjectTouchInput)GetProcAddress(GetHmodUser32(), "InjectTouchInput");

const bool TouchManagerWin8::s_supported = NtUserInitializeTouchInjection && NtUserInjectTouchInput;

/// <summary>
/// Stub structure creator, used by TouchManager overrides
/// </summary>
static void TranslateAndSend(const Touch& touch, ULONG pointerFlags) {
  // Translate:
  const LONG x = static_cast<LONG>(touch.x() - 0.5);
  const LONG y = static_cast<LONG>(touch.y() + 0.5);

  POINTER_TOUCH_INFO contact;
  contact.touchFlags = TOUCH_FLAG_NONE;
  contact.touchMask = TOUCH_MASK_ORIENTATION | TOUCH_MASK_CONTACTAREA;
  contact.orientation = touch.orientation();
  contact.rcContact.top = y - 2;
  contact.rcContact.bottom = y + 2;
  contact.rcContact.left = x - 2;
  contact.rcContact.right = x + 2;

  POINTER_INFO& ptrInfo = contact.pointerInfo;
  ptrInfo.pointerType = PT_TOUCH;
  ptrInfo.pointerFlags = pointerFlags;
  ptrInfo.pointerId = touch.id();
  ptrInfo.ptPixelLocation.x = x;
  ptrInfo.ptPixelLocation.y = y;
  NtUserInjectTouchInput(1, &contact);
}

TouchManagerWin8::TouchManagerWin8(void) {
  // Support exists for InitializeTouchInjection
  NtUserInitializeTouchInjection(MAX_TOUCH_COUNT, TOUCH_FEEDBACK_DEFAULT);
}

void TouchManagerWin8::AddTouch(const Touch& touch) {
  TranslateAndSend(
    touch,
    POINTER_FLAG_INRANGE |
    (
      touch.touching() ?
      POINTER_FLAG_INCONTACT :
      0
    ) |
    POINTER_FLAG_UPDATE
  );
}

void TouchManagerWin8::UpdateTouch(const Touch& oldTouch, const Touch& newTouch) {
  TranslateAndSend(
    newTouch,

    // As long as we're updating, we're in range
    POINTER_FLAG_INRANGE |

    // Touching in the new state?
    (
      newTouch.touching() ?
      (
        POINTER_FLAG_INCONTACT |

        // Transition detection:
        (
          oldTouch.touching() ?
          POINTER_FLAG_UPDATE :
          POINTER_FLAG_DOWN
        )
      ) :

      // Transition detection:
      (
        oldTouch.touching() ?
        POINTER_FLAG_UPDATE :
        POINTER_FLAG_UP
      )
    )
  );
}

void TouchManagerWin8::RemoveTouch(const Touch& oldTouch) {
  TranslateAndSend(
    oldTouch,
    POINTER_FLAG_UPDATE
  );
}