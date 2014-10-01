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
  memset(&contact, 0, sizeof(POINTER_TOUCH_INFO));
  contact.touchFlags = TOUCH_FLAG_NONE;
  contact.touchMask = TOUCH_MASK_CONTACTAREA | TOUCH_MASK_ORIENTATION | TOUCH_MASK_PRESSURE;
  contact.orientation = 90;
  contact.pressure = 32000;
  contact.rcContact.top = y - 2;
  contact.rcContact.bottom = y + 2;
  contact.rcContact.left = x - 2;
  contact.rcContact.right = x + 2;

  POINTER_INFO& ptrInfo = contact.pointerInfo;
  ptrInfo.pointerType = PT_TOUCH;
  ptrInfo.pointerFlags = pointerFlags;
  ptrInfo.pointerId = touch.id() % MAX_TOUCH_COUNT; //NOTE: the touch ids cannot exceed the touch count
  ptrInfo.ptPixelLocation.x = x;
  ptrInfo.ptPixelLocation.y = y;
  NtUserInjectTouchInput(1, &contact);
}

TouchManagerWin8::TouchManagerWin8(LPVirtualScreen* virtualScreen) :
  TouchManager(virtualScreen)
{
  // Support exists for InitializeTouchInjection
  NtUserInitializeTouchInjection(MAX_TOUCH_COUNT, TOUCH_FEEDBACK_DEFAULT);
}

void TouchManagerWin8::AddTouch(const Touch& touch) {
  if (touch.touching()) {
    TranslateAndSend(touch, POINTER_FLAG_INRANGE | POINTER_FLAG_INCONTACT | POINTER_FLAG_DOWN);
  } else if (!touch.touching()) {
    TranslateAndSend(touch, POINTER_FLAG_INRANGE | POINTER_FLAG_UPDATE);
  }
}

void TouchManagerWin8::UpdateTouch(const Touch& oldTouch, const Touch& newTouch) {
  if (newTouch.touching() && oldTouch.touching()) {
    TranslateAndSend(newTouch, POINTER_FLAG_INRANGE | POINTER_FLAG_INCONTACT | POINTER_FLAG_UPDATE);
  } else if (newTouch.touching() && !oldTouch.touching()) {
    TranslateAndSend(newTouch, POINTER_FLAG_INRANGE | POINTER_FLAG_INCONTACT | POINTER_FLAG_DOWN);
  } else if (!newTouch.touching() && oldTouch.touching()) {
    TranslateAndSend(oldTouch, POINTER_FLAG_INRANGE | POINTER_FLAG_UP);
    TranslateAndSend(newTouch, POINTER_FLAG_INRANGE | POINTER_FLAG_UPDATE);
  } else if (!newTouch.touching() && !oldTouch.touching()) {
    TranslateAndSend(newTouch, POINTER_FLAG_INRANGE | POINTER_FLAG_UPDATE);
  }
}

void TouchManagerWin8::RemoveTouch(const Touch& oldTouch) {
    TranslateAndSend(oldTouch, POINTER_FLAG_UP);
  } else {
    TranslateAndSend(oldTouch, POINTER_FLAG_UPDATE);
  }
}
