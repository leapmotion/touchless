/*==================================================================================================================

    Copyright (c) 2010 - 2013 Leap Motion. All rights reserved.

  The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
  protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
  strictly forbidden unless prior written permission is obtained from Leap Motion.

===================================================================================================================*/

#if !defined(__LPOverlay_h__)
#define __LPOverlay_h__

#if __APPLE__

#include "LPIcon.h"
#include <vector>
#include <set>
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>

struct LPOverlayCache {
  LPOverlayCache() : mainDisplayHeight(0), data(nullptr) {}
  ~LPOverlayCache() { delete data; }

  std::vector<CGRect> overlayImageBoundingBox;
  CGFloat mainDisplayHeight;
  uint32_t* data;
};

class LPOverlay {
  public:
    LPOverlay();
    ~LPOverlay();

    void AddIcon(const std::shared_ptr<LPIcon>& icon);
    void RemoveIcon(const std::shared_ptr<LPIcon>& icon);

    void Flush();

  private:
    std::vector<void*> m_displays;
    std::set<std::shared_ptr<LPIcon>> m_icons;
    CGFloat m_mainDisplayHeight;
    boost::thread m_renderThread;
    boost::mutex m_renderMutex;
    boost::condition_variable m_renderCondition;
    std::shared_ptr<LPOverlayCache> m_overlayCache;
    bool m_modifiedCache;
    bool m_keepThreadRunning;

    void Update();
    void Render();

    static void ConfigurationChangeCallback(CGDirectDisplayID display, CGDisplayChangeSummaryFlags flags, void *that);
};

class LPOverlayFlusher {
  public:
    LPOverlayFlusher(LPOverlay& overlay) : m_overlay(overlay), m_flushed(false) {}
    ~LPOverlayFlusher() { Flush(); }

    bool Flush() { if (!m_flushed) { m_overlay.Flush(); m_flushed = true; return true; } else { return false; } }

    bool WasFlushed() const { return m_flushed; }

  private:
    LPOverlay& m_overlay;
    bool m_flushed;
};

#endif

#endif // __LPOverlay_h__
