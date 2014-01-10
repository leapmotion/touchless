/*==================================================================================================================

    Copyright (c) 2010 - 2013 Leap Motion. All rights reserved.

  The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
  protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
  strictly forbidden unless prior written permission is obtained from Leap Motion.

===================================================================================================================*/

#if !defined(__LPImage_h__)
#define __LPImage_h__

#if _WIN32
#include <windows.h>
#include <algorithm>
using std::min;
using std::max;
#include <objidl.h>
#include <gdiplus.h>
#else
#include "LPGeometry.h"
#endif

#include "common.h"
#include <set>

class LPIcon;

class LPImage {
  public:
    LPImage();
    ~LPImage();

    void RasterCircle(const Vector2& centerOffset, const Vector2& direction, double velocity, double outerRadius, double borderRadius, double glow, float r, float g, float b, float a);
    bool SetImage(const SIZE& size, const POINT* pHotspot = 0);
    bool SetImage(const std::wstring& filename, const POINT* pHotspot = 0);
    void Clear();

    int32_t GetWidth() const { return static_cast<int32_t>(m_size.cx); }
    int32_t GetHeight() const { return static_cast<int32_t>(m_size.cy); }

    const POINT& GetHotspot() const { return m_hotspot; }

#if __APPLE__
    uint32_t* GetInternalImage() const { return m_colors; }
#endif

  private:
#if _WIN32
    // The DC representing this image
    HDC m_hDC;
    RGBQUAD* m_colors;
#elif __APPLE__
    uint32_t* m_colors;
#endif
    POINT m_hotspot;
    SIZE m_size;

    /// <summary>
    /// Constructs an image
    /// </summary>
    /// <param name="width">The width of the passed image</param>
    /// <param name="height">The height of the passed image</param>
    /// <param name="pHotspot">The hot spot for the image.  If left null, it will default to the geometric center.</param>
    bool InitImage(long width, long height, const POINT* pHotspot = 0);

#if _WIN32
    /// <summary>
    /// Constructs a DC from the passed Gdi+ bitmap
    /// </summary>
    /// <param name="bitmap">The Gdi+ bitmap containing the desired image data</param>
    /// <param name="pHotspot">The hot spot for the image.  If left null, it will default to the geometric center.</param>
    bool SetImageFromBitmap(Gdiplus::Bitmap& src, const POINT* pHotspot = 0, bool write = false);

    /// <summary>
    /// Sets the DC used as a source for this image
    /// </summary>
    /// <param name="pHotspot">The hot spot for the image.  If left null, it will default to the geometric center.</param>
    /// <remarks>
    /// The passed DC must have been created with CreateCompatibleDC.  Once assigned, this class
    /// takes responsibility for the cleanup of the HDC and the HBITMAP attached to it
    /// </remarks>
    void SetDC(HDC hDc, const POINT* pHotspot = 0);

    HDC GetDC() const { return m_hDC; }
#endif

    friend class LPIcon;
};

#endif // __LPImage_h__
