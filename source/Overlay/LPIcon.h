/*==================================================================================================================

    Copyright (c) 2010 - 2013 Leap Motion. All rights reserved.

  The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
  protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
  strictly forbidden unless prior written permission is obtained from Leap Motion.

===================================================================================================================*/

#if !defined(__LPIcon_h__)
#define __LPIcon_h__
#include "C++11/cpp11.h"
#include "Utility/LPGeometry.h"
#include SHARED_PTR_HEADER

class LPImage;

class LPIcon {
  public:
    LPIcon();
    ~LPIcon();

    void SetImage(const std::shared_ptr<LPImage>& image = std::shared_ptr<LPImage>(), bool update = true);
    void SetPosition(const LPPoint& position);
    void SetVisibility(bool isVisible);
    bool Update();

    const std::shared_ptr<LPImage>& GetImage() const { return m_image; }
    const LPPoint& GetPosition() const { return m_position; }
    bool GetVisibility() const { return m_isVisible; }

  private:
    std::shared_ptr<LPImage> m_image;
    LPPoint m_position;
    bool m_isVisible;
#if _WIN32
    HWND m_hWnd;

    LRESULT WindowProc(UINT uMsg, WPARAM wParam, LPARAM lParam);

    friend class LPIconWindowClass;
#endif
};

#endif // __LPIcon_h__
