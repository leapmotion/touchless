/*==================================================================================================================

    Copyright (c) 2010 - 2013 Leap Motion. All rights reserved.

  The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
  protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
  strictly forbidden unless prior written permission is obtained from Leap Motion.

===================================================================================================================*/
#include "stdafx.h"
#include "TouchManager.h"

#define TOUCH_DEBUGGING_TO_FILE 0

#if TOUCH_DEBUGGING_TO_FILE
#include <fstream>
#endif

TouchManager::TouchManager() :
#if _WIN32
  m_hidInstance(0),
#endif
  m_touchVersion(0)
{
#if _WIN32
  // Check for Windows 8 Touch support
  HMODULE handle = GetModuleHandle("user32.dll");
  BOOL (WINAPI *initializeTouchInjection)(UINT32, DWORD) =
    reinterpret_cast<BOOL (WINAPI *)(UINT32, DWORD)>(GetProcAddress(handle, "InitializeTouchInjection"));
  if (initializeTouchInjection) {
    m_injectTouchInput =
      reinterpret_cast<BOOL (WINAPI *)(UINT32, const POINTER_TOUCH_INFO*)>(GetProcAddress(handle, "InjectTouchInput"));
    if (m_injectTouchInput) {
      initializeTouchInjection(MAX_TOUCH_COUNT, TOUCH_FEEDBACK_DEFAULT);
      m_touchVersion = 8; // Supports Windows 8 Touch Interface
    }
  }
  if (!m_touchVersion) { // Check for Windows 7 Touch support
    m_hidInstance = COcuInterface().GetFirstCompliantInterface();
    if (m_hidInstance) {
      m_touchVersion = 7; // Supports Windows 7 Touch Interface
    }
  }
#endif
}

TouchManager::~TouchManager()
{
  clearTouches();
#if _WIN32
  if (m_touchVersion == 7) { // Windows 7 Touch
    delete m_hidInstance;
  }
#endif
}

void TouchManager::clearTouches()
{
  if (!m_touches.empty()) {
    setTouches(std::set<Touch>());
  }
}

#if TOUCH_DEBUGGING_TO_FILE
#define LOG_TO_FILE(x) fileOutput << x
#else
#define LOG_TO_FILE(x)
#endif

void TouchManager::setTouches(const std::set<Touch>& touches)
{
#if TOUCH_DEBUGGING_TO_FILE
  std::ofstream fileOutput("touch-log.txt", std::ios::app);
#endif

  std::set<Touch> sendTouches;

  // Handle new and existing touches
  for (std::set<Touch>::iterator iter = touches.begin(); iter != touches.end(); ++iter) {
    Touch touch = *iter;
    std::set<Touch>::iterator result = m_touches.find(*iter);
    if (result == m_touches.end()) {
      touch.setState(Touch::STATE_HOVER);
      LOG_TO_FILE("A");
    } else {
      if (touch.touching()) {
        if (result->state() == Touch::STATE_BEGIN) {
          touch.setPos(result->x(), result->y());
          touch.setState(Touch::STATE_UPDATE);
          LOG_TO_FILE("B");
        } else if (result->state() == Touch::STATE_UPDATE) {
          touch.setState(Touch::STATE_UPDATE);
          LOG_TO_FILE("C");
        } else {
          touch.setPos(result->x(), result->y());
          touch.setState(Touch::STATE_BEGIN);
          LOG_TO_FILE("D");
        }
      } else {
        if (result->state() == Touch::STATE_BEGIN || result->state() == Touch::STATE_UPDATE) {
          // when ending a touch, the position must be set to the same position as the previous frame
          touch.setPos(result->x(), result->y());
          touch.setState(Touch::STATE_END);
          LOG_TO_FILE("E");
        } else if (result->state() == Touch::STATE_END) {
          touch.setPos(result->x(), result->y());
          touch.setState(Touch::STATE_HOVER);
          LOG_TO_FILE("F");
        } else {
          touch.setState(Touch::STATE_HOVER);
          LOG_TO_FILE("G");
        }
      }
    }
    sendTouches.insert(touch);
  }
  // Remove touches that no longer exist
  for (std::set<Touch>::iterator iter = m_touches.begin(); iter != m_touches.end(); ++iter) {
    if (sendTouches.find(*iter) == sendTouches.end()
      && iter->state() != Touch::STATE_HOVER_END
      && iter->state() != Touch::STATE_UNDEFINED) {
      Touch touch = *iter;
      if (iter->state() == Touch::STATE_HOVER) {
        touch.setState(Touch::STATE_HOVER_END);
        LOG_TO_FILE("H");
      } else if (iter->state() == Touch::STATE_END) {
        touch.setState(Touch::STATE_HOVER);
        LOG_TO_FILE("I");
      } else {
        touch.setState(Touch::STATE_END);
        LOG_TO_FILE("J");
      }
      sendTouches.insert(touch);
    }
  }
  if (!sendTouches.empty()) {
#if _WIN32
    if (m_touchVersion == 8) {
      const size_t numTouches = sendTouches.size();
      memset(m_contacts, 0, numTouches*sizeof(POINTER_TOUCH_INFO));
      UINT32 index = 0;

      // Build touches
      for (std::set<Touch>::iterator iter = sendTouches.begin(); iter != sendTouches.end(); ++iter) {
        const LONG x = static_cast<LONG>(iter->x() - 0.5);
        const LONG y = static_cast<LONG>(iter->y() + 0.5);
        Touch::State state = iter->state();

        POINTER_TOUCH_INFO& contact = m_contacts[index];
        contact.touchFlags = TOUCH_FLAG_NONE;
        contact.touchMask = TOUCH_MASK_ORIENTATION | TOUCH_MASK_CONTACTAREA;
        contact.orientation = iter->orientation();
        contact.rcContact.top = y - 2;
        contact.rcContact.bottom = y + 2;
        contact.rcContact.left = x - 2;
        contact.rcContact.right = x + 2;

        POINTER_INFO& ptrInfo = contact.pointerInfo;
        ptrInfo.pointerType = PT_TOUCH;
        ptrInfo.pointerId = iter->id();
        ptrInfo.ptPixelLocation.x = x;
        ptrInfo.ptPixelLocation.y = y;

        LOG_TO_FILE(" | touch id " << ptrInfo.pointerId << " (" << x << ", " << y << "): ");

        // Injecting the touch down on screen
        switch (iter->state()) {
          case Touch::STATE_BEGIN:
            ptrInfo.pointerFlags = POINTER_FLAG_INRANGE | POINTER_FLAG_INCONTACT | POINTER_FLAG_DOWN;
            LOG_TO_FILE("STATE_BEGIN");
            break;
          case Touch::STATE_HOVER:
            ptrInfo.pointerFlags = POINTER_FLAG_INRANGE | POINTER_FLAG_UPDATE;
            LOG_TO_FILE("STATE_HOVER");
            break;
          case Touch::STATE_UPDATE:
            ptrInfo.pointerFlags = POINTER_FLAG_INRANGE | POINTER_FLAG_INCONTACT | POINTER_FLAG_UPDATE;
            LOG_TO_FILE("STATE_UPDATE");
            break;
          case Touch::STATE_HOVER_END:
            ptrInfo.pointerFlags = POINTER_FLAG_UPDATE;
            LOG_TO_FILE("STATE_HOVER_END");
            break;
          case Touch::STATE_END:
            ptrInfo.pointerFlags = POINTER_FLAG_INRANGE | POINTER_FLAG_UP;
            LOG_TO_FILE("STATE_END");
            break;
          default:
            ptrInfo.pointerFlags = POINTER_FLAG_NONE;
            LOG_TO_FILE("STATE_NONE");
            break;
        }
        if (ptrInfo.pointerFlags != POINTER_FLAG_NONE) {
          index++;
        }
      }

      // Send touches to OS
      m_injectTouchInput(index, m_contacts);
      DWORD error = GetLastError();
      if (error == ERROR_INVALID_PARAMETER) {
        LOG_TO_FILE(" ERROR_INVALID_PARAMETER");
      } else if (error == ERROR_TIMEOUT) {
        LOG_TO_FILE(" ERROR_TIMEOUT");
      } else if (error == ERROR_NOT_READY) {
        LOG_TO_FILE(" ERROR_NOT_READY");
      }

    } else if (m_touchVersion == 7) {
      for (std::set<Touch>::iterator iter = sendTouches.begin(); iter != sendTouches.end(); ++iter) {
        bool touching = iter->state() == Touch::STATE_BEGIN || iter->state() == Touch::STATE_UPDATE;
        LOG_TO_FILE(" | touch id " << iter->id() << " (" << iter->x() << ", " << iter->y() << "): " << touching);
        m_hidInstance->SendReport((BYTE)iter->id(), touching, iter->x(), iter->y(), 0.5f, 0.5f);
      }
    }
#endif
    // Otherwise, do what?
  }

#if TOUCH_DEBUGGING_TO_FILE
  fileOutput << std::endl;
  fileOutput.close();
#endif

  // The new set of touches
  m_touches = sendTouches;
}
