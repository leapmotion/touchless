/*==================================================================================================================

    Copyright (c) 2010 - 2013 Leap Motion. All rights reserved.

  The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
  protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
  strictly forbidden unless prior written permission is obtained from Leap Motion.

===================================================================================================================*/
#include "stdafx.h"
#include "LPImage.h"
#include "LPIcon.h"
#include "DataStructures/SSEfoo.h"
#include <memory>
#include <algorithm>

#if _WIN32
using namespace Gdiplus;
#pragma comment (lib,"Gdiplus.lib")
#elif __APPLE__
#include "DataStructures/Value.h"
#include <NSData.h>
#include <NSString.h>
#include <NSURL.h>
#endif

LPImage::LPImage()
#if _WIN32
: m_hDC(0),
  m_colors(nullptr)
#elif __APPLE__
: m_colors(nullptr)
#endif
{
  m_hotspot.x = 0;
  m_hotspot.y = 0;
  m_size.cx = 0;
  m_size.cy = 0;
}

LPImage::~LPImage()
{
#if _WIN32
  // Set the DC to null.  This will release any resources assocaited with
  // the current DC.
  SetDC(nullptr);
#elif __APPLE__
  delete [] m_colors;
#endif
}

void LPImage::RasterCircle(const Vector2& centerOffset, const Vector2& direction, double velocity, double outerRadius, double borderRadius, double glow, float r, float g, float b, float a)
{
#if _WIN32 || __APPLE__
  if (m_colors == nullptr) {
    return;
  }
#endif
  //Create a velocity based scale warping
  const double radiusSq = outerRadius*outerRadius;
  const double borderSq = (outerRadius-borderRadius)*(outerRadius-borderRadius);
  const double minSize = std::min(m_size.cx/2.0, m_size.cy/2.0);
  const double glowSq = std::min(radiusSq*glow*glow, minSize*minSize); // clamp so that glow doesn't exceed icon
  velocity = std::max(velocity - 50.0, 0.0);
  const double yScale = 1.0 + std::min(2.0, velocity / 900.0);
  const double xScale = std::sqrt(1.0 / yScale);

  //Find axes and center
  Vector2 aunit = velocity == 0 ? Vector2::UnitX() : direction.normalized();
  Vector2 bunit = Vector2(aunit.y(), -aunit.x());
  const Vector2 center(m_size.cx/2.0 - 1.0 + centerOffset.x(), m_size.cy/2.0 - 1.0 + centerOffset.y());

  //Calculate the drawing bounds
  const double xx = Vector2(aunit.x()/xScale, bunit.x()/yScale).norm()*outerRadius*std::abs(glow);
  const double yy = Vector2(aunit.y()/xScale, bunit.y()/yScale).norm()*outerRadius*std::abs(glow);
  const long minX = std::min(static_cast<long>(m_size.cx), std::max(0L, static_cast<long>(center.x() - xx)));
  const long minY = std::min(static_cast<long>(m_size.cy), std::max(0L, static_cast<long>(center.y() - yy)));
  const long maxX = std::min(static_cast<long>(m_size.cx), std::max(0L, static_cast<long>(center.x() + xx + 1)));
  const long maxY = std::min(static_cast<long>(m_size.cy), std::max(0L, static_cast<long>(center.y() + yy + 1)));

  //Pre-scale axes
  aunit *= xScale;
  bunit *= yScale;

  //Setup colors
  SSEf dark;
  if (glow > 0.0) {
    float mult = (glow > 1.0 ? 1.0f : 0.2f);
    dark = SSEf(mult*255, mult*255, mult*255, 200);
  } else {
    dark = SSEf(b*0.2f, g*0.2f, r*0.2f, 200);
  }
  SSEf color(b, g, r, 255);
  SSEf gray = SSEAvg(color, SSEf(255));

  //Fill dynamic image with solid transparent color
#if _WIN32
  memset(m_colors, 0, sizeof(RGBQUAD)*m_size.cx*m_size.cy);
#elif __APPLE__
  memset(m_colors, 0, sizeof(uint32_t)*m_size.cx*m_size.cy);
#else
  //TODO: Linux fixme
#endif

  //Draw the oriented ellipse
  for (long y = minY; y < maxY; y++) {
    for (long x = minX; x < maxX; x++) {
      //Calculate index of destination
#if _WIN32 || __APPLE__
      const long ix = m_size.cx*y + x;
#endif

      //Calculate normalized distances for points
      Vector2 pt = Vector2(x+0.5, y+0.5) - center;
      double fx = pt.dot(aunit);
      double fy = pt.dot(bunit);
      double distSq = fx*fx + fy*fy;

      //Check if point is inside of glow radius
      if (distSq < glowSq) {
        //Drawing parameters
        float alpha = a;
        float blend = 1.0f;

        //Check if point is inside of ellipse
        SSEf orig = color;
        if (distSq < radiusSq) {
          blend = static_cast<float>(std::max(0.0, std::min(1.0, (borderSq - distSq)/(2*outerRadius))));
          alpha *= static_cast<float>(std::min(1.0, (glowSq - distSq)/(2*outerRadius)));
          alpha *= 1.0f - blend*0.4f;
          orig = gray;
        } else {
          blend = static_cast<float>(std::max(0.0, std::min(1.0, (distSq - radiusSq)/(2*outerRadius))));
          const float temp = static_cast<float>(1.0 - (distSq - radiusSq)/(glowSq - radiusSq));
          alpha *= temp*temp*temp*0.9f;
        }

#if _WIN32 || __APPLE__
        //Apply blending to pixel
        SSEf blended = (orig*blend + dark*(1.0f - blend))*alpha;
        uint32_t finalColor = SSEPackChars((SSEi)blended);
#endif

        //Write pixel to memory
#if _WIN32
        reinterpret_cast<uint32_t*>(m_colors)[ix] = finalColor;
#elif __APPLE__
        m_colors[ix] = finalColor;
#else
        //TODO: Linux fixme
#endif
      }
    }
  }
}

bool LPImage::SetImage(const SIZE& size, const POINT* pHotspot)
{
  // this version creates the image from a given width and height
#if _WIN32
  Bitmap bmp(static_cast<INT>(size.cx), static_cast<INT>(size.cy), PixelFormat32bppARGB);

  if (bmp.GetLastStatus() != Ok) {
    return false;
  }

  return SetImageFromBitmap(bmp, pHotspot, false);
#elif __APPLE__
  return InitImage(static_cast<long>(size.cx), static_cast<long>(size.cy), pHotspot);
#else
  return false; // Linux -- FIXME
#endif
}

bool LPImage::SetImage(const std::wstring& filename, const POINT* pHotspot)
{
  // this version creates the image from a file
#if _WIN32
  Bitmap bmp(filename.c_str());

  if (bmp.GetLastStatus() != Ok) {
    return false;
  }

  return SetImageFromBitmap(bmp, pHotspot, true);
#elif __APPLE__
  delete [] m_colors;
  m_colors = nullptr;

  std::string utf8String = Value::convertWideStringToUTF8String(filename);
  NSString* fn = [NSString stringWithUTF8String:utf8String.c_str()];
  NSURL *url = [NSURL fileURLWithPath:fn];
#if 0
  // Disable for now... FIXME
  CGImage* image = ...; // Init with URL .. capture bytes -- FIXME
  if (image) {
    CGRect extent = [image extent];
    m_size.cx = extent.size.width;
    m_size.cy = extent.size.height;
    if (pHotspot) {
      // User is assigning a hotspot for this image.  Copy it over.
      m_hotspot = *pHotspot;
    } else {
      // User has not specified an explicit hotspot location.  The hotspot
      // will therefore default to the center of the image.
      m_hotspot.x = m_size.cx / 2;
      m_hotspot.y = m_size.cy / 2;
    }
  } else {
    m_hotspot.x = 0;
    m_hotspot.y = 0;
    m_size.cx = 0;
    m_size.cy = 0;
  }
#endif
  [url release];
  [fn release];

  return (m_colors != nullptr);
#else
  return false; // Linux -- FIXME
#endif
}

bool LPImage::InitImage(long width, long height, const POINT* pHotspot)
{
#if _WIN32
  // Construct an information header based on the locked bits:
  BITMAPINFO info;
  BITMAPINFOHEADER& hdr = info.bmiHeader;
  hdr.biSize = sizeof(hdr);
  hdr.biWidth = width;
  hdr.biHeight = height;
  hdr.biPlanes = 1;
  hdr.biBitCount = 32;
  hdr.biCompression = BI_RGB;
  hdr.biSizeImage = 0;
  hdr.biXPelsPerMeter = 0;
  hdr.biYPelsPerMeter = 0;
  hdr.biClrUsed = 0;
  hdr.biClrImportant = 0;

  // Create the objects to be returned:
  HDC hDC = CreateCompatibleDC(nullptr);

  // Attempt to perform the copy operation
  HBITMAP hBitmap = CreateDIBSection(
    hDC,
    &info,
    DIB_RGB_COLORS,
    (void**)&m_colors,
    nullptr,
    0
  );

  if (!hBitmap) {
    DeleteDC(hDC);
    return false;
  }

  // Attach the constructed DC to the bitmap:
  SelectObject(hDC, hBitmap);

  // Attach the DC:
  SetDC(hDC, pHotspot);

  // Done, return.
  return true;
#elif __APPLE__
  bool status = false;

  m_size.cx = static_cast<int32_t>(width);
  m_size.cy = static_cast<int32_t>(height);
  if (pHotspot) {
    // User is assigning a hotspot for this image.  Copy it over.
    m_hotspot = *pHotspot;
  } else {
    // User has not specified an explicit hotspot location.  The hotspot
    // will therefore default to the center of the image.
    m_hotspot.x = m_size.cx / 2;
    m_hotspot.y = m_size.cy / 2;
  }

  delete [] m_colors;
  m_colors = new uint32_t[m_size.cx*m_size.cy];

  return status;
#else
  (void)width;
  (void)height;
  (void)pHotspot;
  return false;
#endif
}

#if _WIN32
bool LPImage::SetImageFromBitmap(Gdiplus::Bitmap& src, const POINT* pHotspot, bool write)
{
  BitmapData bitmapData;

  // Lock bits in the passed source image:
  if (src.LockBits(&Rect(0, 0, src.GetWidth(), src.GetHeight()),
                   ImageLockModeRead, PixelFormat32bppARGB, &bitmapData) != Ok) {
    return false;
  }

  if (bitmapData.PixelFormat != PixelFormat32bppARGB) {
    return false;
  }

  // Set our image internally based on this array of pixels:
  bool rs = InitImage(bitmapData.Width, bitmapData.Height, pHotspot);

  if (write) {
    // Set up the destination pointer
    RGBQUAD* pvQuadDest = m_colors;

    // Set up the source pointers
    const RGBQUAD* pCurQuadSrc = (const RGBQUAD*)bitmapData.Scan0;
    const char*& pCurLineSrc = (const char*&)pCurQuadSrc;

    // Alpha channel must be premultiplied alpha in order to be blended correctly.
    // The premultiplication is done here.
    for (UINT y = bitmapData.Height; y--;) {
      for(UINT x = bitmapData.Width; x--;) {
        int alpha = pCurQuadSrc[x].rgbReserved;
        pvQuadDest[x].rgbReserved = alpha;
        pvQuadDest[x].rgbRed = pCurQuadSrc[x].rgbRed * alpha >> 8;
        pvQuadDest[x].rgbGreen = pCurQuadSrc[x].rgbGreen * alpha >> 8;
        pvQuadDest[x].rgbBlue = pCurQuadSrc[x].rgbBlue * alpha >> 8;
      }

      // Increment by width and by pitch:
      pvQuadDest += bitmapData.Width;
      pCurLineSrc += bitmapData.Stride;
    }
  }

  // Done with processing, unlock and return the response value.
  src.UnlockBits(&bitmapData);

  return rs;
}

void LPImage::SetDC(HDC hDC, const POINT* pHotspot)
{
  if (m_hDC) {
    // A DC currently exists.  We must release any resources associated with
    // this DC and then destroy it.

    // Get the bitmap first and delete it
    HBITMAP hBitmap = (HBITMAP)GetCurrentObject(m_hDC, OBJ_BITMAP);

    // Delete the DC
    DeleteDC(m_hDC);

    // Delete the unbounded bitmap
    DeleteObject(hBitmap);
  }

  // Get a handle to the bitmap:
  HBITMAP hBitmap = (HBITMAP)GetCurrentObject(hDC, OBJ_BITMAP);
  BITMAP bmpInfo;

  // Get extended information about the bitmap
  if (GetObject(hBitmap, sizeof(bmpInfo), &bmpInfo)) {
    // Record the width and height of the bitmap:
    m_size.cx = bmpInfo.bmWidth;
    m_size.cy = bmpInfo.bmHeight;
  } else {
    m_size.cx = m_size.cy = 0;
  }

  // Update hot spot information:
  if (hDC) {
    // User is assigning a non-null DC.
    if (pHotspot) {
      // User is assigning a hotspot for this DC.  Copy it over.
      m_hotspot = *pHotspot;
    } else {
      // User has not specified an explicit hotspot location.  The hotspot
      // will therefore default to the center of the image.
      m_hotspot.x = m_size.cx / 2;
      m_hotspot.y = m_size.cy / 2;
    }
  } else {
    // Everything is all zeroes
    m_hotspot.x = m_hotspot.y = 0;
    m_size.cx = m_size.cy = 0;
  }
  // Select in the new DC:
  m_hDC = hDC;
}
#endif

void LPImage::Clear()
{
#if _WIN32
  if (m_colors) {
    memset(m_colors, 0, sizeof(RGBQUAD)*m_size.cx*m_size.cy);
  }
#elif __APPLE__
  if (m_colors) {
    memset(m_colors, 0, sizeof(uint32_t)*m_size.cx*m_size.cy);
  }
#endif
}
