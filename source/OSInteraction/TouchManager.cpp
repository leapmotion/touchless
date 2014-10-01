#include "stdafx.h"
#include "TouchManager.h"
#include "Utility/LPVirtualScreen.h"
#if _WIN32
#include "TouchManagerWin7Driver.h"
#include "TouchManagerWin7Undocumented.h"
#include "TouchManagerWin8.h"
#endif
#include <fstream>

#define TOUCH_DEBUGGING_TO_FILE 0

#if TOUCH_DEBUGGING_TO_FILE
  #define LOG_TO_FILE(x) fileOutput << x
#else
  #define LOG_TO_FILE(x)
#endif

TouchManager::TouchManager(void) {
}

TouchManager::~TouchManager(void) {
  clearTouches();
}

TouchManager* TouchManager::New(void) {
#if _WIN32
  if(TouchManagerWin8::s_supported)
    return new TouchManagerWin8;

  if(TouchManagerWin7Undocumented::s_supported)
    return new TouchManagerWin7Undocumented;

  if(TouchManagerWin7Driver::s_supported)
    return new TouchManagerWin7Driver;
#endif
  return nullptr;
}

void TouchManager::clearTouches() {
  if (!m_touches.empty()) {
    setTouches(std::set<Touch>());
  }
}

size_t TouchManager::numTouchScreens(void) const {
#if __APPLE__
  return 0;
#else
  return m_virtualScreen.NumScreens();
#endif
}

void TouchManager::setTouches(std::set<Touch>&& touches) {
#if TOUCH_DEBUGGING_TO_FILE
  std::ofstream fileOutput("touch-log.txt", std::ios::app);
#endif

  // Initially, we consider all current touches to be removed:
  std::set<Touch> removedTouches;
  std::swap(m_touches, removedTouches);

  // Then we consider the passed touches to be the new touches:
#if __APPLE__
  m_touches = touches;
#else
  m_touches = std::move(touches);
#endif

  // Handle new and existing touches
  for (auto iter = m_touches.begin(); iter != m_touches.end(); ++iter) {
    // Determine whether this touch is new:
    auto priorTouch = removedTouches.find(*iter);
    if (priorTouch == removedTouches.end()) {
      // Couldn't find this touch in our collection.  It must be new.
      AddTouch(*iter);
    } else {
      // Touch already exists, just update it.
      UpdateTouch(*priorTouch, *iter);

      // In any case we found this touch--we don't want to consider it "removed"
      removedTouches.erase(priorTouch);
    }
  }

  // Process each removed touch:
  for (auto q = removedTouches.begin(); q != removedTouches.end(); ++q) {
    RemoveTouch(*q);
  }

  // If the collection is now empty, we notify derived types of this new fact:
  if (m_touches.empty()) {
    OnRemoveAllTouches();
  }

#if TOUCH_DEBUGGING_TO_FILE
  fileOutput << std::endl;
  fileOutput.close();
#endif
}
