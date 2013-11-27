/*==================================================================================================================

    Copyright (c) 2010 - 2013 Leap Motion. All rights reserved.

  The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
  protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
  strictly forbidden unless prior written permission is obtained from Leap Motion.

===================================================================================================================*/

#import "stdafx.h"
#import "LPMac.h"
#import "LPGeometry.h"
#import "LPIcon.h"
#import "LPImage.h"

#import <NSArray.h>
#import <NSBundle.h>
#import <NSData.h>
#import <NSDictionary.h>
#import <NSEvent.h>
#import <NSProcessInfo.h>
#import <NSRunningApplication.h>
#import <NSURL.h>
#import <NSScreen.h>
#import <TextInputSources.h>
#import <cmath>

//
// LPMac
//

CGEventTimestamp LPMac::systemUptime()
{
  return [[NSProcessInfo processInfo] systemUptime]*1000000000;
}

void LPMac::cursorPosition(float &x, float &y)
{
  NSPoint mouseLoc;
  mouseLoc = [NSEvent mouseLocation];
  
  NSRect screenRect;
  NSArray *screenArray = [NSScreen screens];
//   unsigned screenCount = [screenArray count];
  unsigned index  = 0;

  NSScreen *screen = [screenArray objectAtIndex: index];
  screenRect = [screen visibleFrame];

//   NSRect screenRect = [[NSScreen mainScreen] frame];
  
  x = mouseLoc.x;
  y = screenRect.size.height - mouseLoc.y;
}

CGEventType LPMac::EventTypeGesture = NSEventTypeGesture;

//
// CFocusAppInfo
//

CFocusAppInfo::CFocusAppInfo()
{
  m_rcClient.left = m_rcClient.right = m_rcClient.top = m_rcClient.bottom = 0;
  @autoreleasepool {
    m_hasFrontmostApplication = [[NSWorkspace sharedWorkspace] respondsToSelector:@selector(frontmostApplication)];
  }
}

void CFocusAppInfo::Update()
{
  @autoreleasepool {
    NSWorkspace* workspace = [NSWorkspace sharedWorkspace];
    NSString* localizedName;
    NSString* executableName;

    if (m_hasFrontmostApplication) { // Available since OS X v10.7
      NSRunningApplication* runningApplication = [workspace frontmostApplication];

      localizedName = [runningApplication localizedName];
      executableName = [[[runningApplication executableURL] filePathURL] path];
    } else { // Deprecated in OS X v10.7
      NSDictionary* dict = [workspace activeApplication];

      localizedName = [dict objectForKey:@"NSApplicationName"];
      executableName = [[NSBundle bundleWithPath:[dict objectForKey:@"NSApplicationPath"]] executablePath];
    }

    NSData* title = [localizedName dataUsingEncoding:NSUTF32LittleEndianStringEncoding];
    NSData* name = [executableName dataUsingEncoding:NSUTF32LittleEndianStringEncoding];

    m_windowTitle =  std::wstring(reinterpret_cast<const wchar_t*>([title bytes]), [title length]/sizeof(wchar_t));
    m_ownerExeName = std::wstring(reinterpret_cast<const wchar_t*>([name bytes]), [name length]/sizeof(wchar_t));
  }
}

//
// LPKeyboard
//

CGKeyCode LPKeyboard::GetKeyCode(int key)
{
  CGKeyCode code;

  switch (key) {
    case VK_BACK: code = kVK_Delete; break;
    case VK_TAB: code = kVK_Tab; break;
    case VK_RETURN: code = kVK_Return; break;
    case VK_SHIFT: code = kVK_Shift; break;
    case VK_CONTROL: code = kVK_Control; break;
    case VK_MENU: code = kVK_Command; break;
    case VK_CAPITAL: code = kVK_CapsLock; break;
    case VK_ESCAPE: code = kVK_Escape; break;
    case VK_SPACE: code = kVK_Space; break;
    case VK_PRIOR: code = kVK_PageUp; break;
    case VK_NEXT: code = kVK_PageDown; break;
    case VK_END: code = kVK_End; break;
    case VK_HOME: code = kVK_Home; break;
    case VK_LEFT: code = kVK_LeftArrow; break;
    case VK_UP: code = kVK_UpArrow; break;
    case VK_RIGHT: code = kVK_RightArrow; break;
    case VK_DOWN: code = kVK_DownArrow; break;
    case VK_DELETE: code = kVK_ForwardDelete; break;
    case VK_HELP: code = kVK_Help; break;
    case VK_F1: code = kVK_F1; break;
    case VK_F2: code = kVK_F2; break;
    case VK_F3: code = kVK_F3; break;
    case VK_F4: code = kVK_F4; break;
    case VK_F5: code = kVK_F5; break;
    case VK_F6: code = kVK_F6; break;
    case VK_F7: code = kVK_F7; break;
    case VK_F8: code = kVK_F8; break;
    case VK_F9: code = kVK_F9; break;
    case VK_F10: code = kVK_F10; break;
    case VK_F11: code = kVK_F11; break;
    case VK_F12: code = kVK_F12; break;
    case VK_F13: code = kVK_F13; break;
    case VK_F14: code = kVK_F14; break;
    case VK_F15: code = kVK_F15; break;
    case VK_F16: code = kVK_F16; break;
    case VK_F17: code = kVK_F17; break;
    case VK_F18: code = kVK_F18; break;
    case VK_F19: code = kVK_F19; break;
    case VK_F20: code = kVK_F20; break;
    case VK_LSHIFT: code = kVK_Shift; break;
    case VK_RSHIFT: code = kVK_RightShift; break;
    case VK_LCONTROL: code = kVK_Control; break;
    case VK_RCONTROL: code = kVK_RightControl; break;
    case VK_LMENU: code = kVK_Command; break;
    default: code = kVK_Invalid; break;
  }

  if (code == kVK_Invalid) {
    TISInputSourceRef currentKeyboard = TISCopyCurrentKeyboardInputSource();
    CFDataRef uchr = (CFDataRef)TISGetInputSourceProperty(currentKeyboard, kTISPropertyUnicodeKeyLayoutData);
    const UCKeyboardLayout *keyboardLayout = (const UCKeyboardLayout*)CFDataGetBytePtr(uchr);

    if (keyboardLayout) {
      const UCKeyboardTypeHeader* keyboardList = keyboardLayout->keyboardTypeList;

      for (ItemCount i = 0; i < keyboardLayout->keyboardTypeCount; i++) {
        UCKeyToCharTableIndex* tableIndex =
          (UCKeyToCharTableIndex*)(((uint8_t*)keyboardLayout) + (keyboardList[i].keyToCharTableIndexOffset));

        if (tableIndex->keyToCharTableIndexFormat == kUCKeyToCharTableIndexFormat) {
          UCKeyStateRecordsIndex* recordsIndex = 0;

          if (keyboardList[i].keyStateRecordsIndexOffset != 0) {
            recordsIndex = (UCKeyStateRecordsIndex*)(((uint8_t*)keyboardLayout) + keyboardList[i].keyStateRecordsIndexOffset);
            if (recordsIndex->keyStateRecordsIndexFormat != kUCKeyStateRecordsIndexFormat) {
              recordsIndex = 0;
            }
          }
          for (ItemCount j = 0; j < tableIndex->keyToCharTableCount; j++) {
            UCKeyOutput* charData =
              (UCKeyOutput*)(((uint8_t*)keyboardLayout) + (tableIndex->keyToCharTableOffsets[j]));

            for (uint16_t k = 0; k < tableIndex->keyToCharTableSize; k++) {
              if ((charData[k] & kUCKeyOutputTestForIndexMask) == kUCKeyOutputStateIndexMask) {
                long keyIndex = (charData[k] & kUCKeyOutputGetIndexMask);

                if (recordsIndex && keyIndex <= recordsIndex->keyStateRecordCount) {
                  UCKeyStateRecord* stateRecord =
                    (UCKeyStateRecord*)(((uint8_t*)keyboardLayout) + recordsIndex->keyStateRecordOffsets[keyIndex]);

                  if (stateRecord->stateZeroCharData == key) {
                    return (CGKeyCode)k;
                  }
                } else if (charData[k] == key) {
                  return (CGKeyCode)k;
                }
              } else if ((charData[k] & kUCKeyOutputTestForIndexMask) != kUCKeyOutputSequenceIndexMask &&
                          charData[k] != 0xFFFE && charData[k] != 0xFFFF && charData[k] == key) {
                return (CGKeyCode)k;
              }
            }
          }
        }
      }
    }
  }
  return code;
}
