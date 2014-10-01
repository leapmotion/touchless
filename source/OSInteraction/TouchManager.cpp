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

TouchManager::TouchManager(LPVirtualScreen* virtualScreen) :
  m_virtualScreen(virtualScreen) {
}

TouchManager::~TouchManager(void) {
  clearTouches();
}

TouchManager* TouchManager::New(LPVirtualScreen* virtualScreen) {
#if _WIN32
  if(TouchManagerWin8::s_supported)
    return new TouchManagerWin8(virtualScreen);

  if(TouchManagerWin7Driver::s_supported)
    return new TouchManagerWin7Driver(virtualScreen);

  if(TouchManagerWin7Undocumented::s_supported)
    return new TouchManagerWin7Undocumented(virtualScreen);

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
  return m_virtualScreen->NumScreens();
#endif
}

void TouchManager::setTouches(const std::set<Touch>& newTouches) {
  // Process each removed touch:
  for (auto priorTouch = m_touches.begin(); priorTouch != m_touches.end();) {
    const auto newTouch = newTouches.find(*priorTouch);
    if (newTouch == newTouches.end()) {
      RemoveTouch(*priorTouch);
      priorTouch = m_touches.erase(priorTouch);
    } else {
      ++priorTouch;
    }
  }

  // Handle new and existing touches
  for (auto newTouch = newTouches.begin(); newTouch != newTouches.end(); ++newTouch) {
    // Determine whether this touch is new:
    auto priorTouch = m_touches.find(*newTouch);
    if (priorTouch == m_touches.end()) {
      // Couldn't find this touch in our collection.  It must be new.
      AddTouch(*newTouch);
      m_touches.insert(*newTouch);
    } else {
      // Touch already exists, just update it.
      UpdateTouch(*priorTouch, *newTouch);
      m_touches.erase(priorTouch);
      m_touches.insert(*newTouch);
    }
  }

  //Notify that the frame is finished with touch events
  FinishFrame();

  // If the collection is now empty, we notify derived types of this new fact:
  if (m_touches.empty()) {
    OnRemoveAllTouches();
  }
}
