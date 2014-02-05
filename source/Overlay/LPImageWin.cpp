#include "stdafx.h"
#include "Overlay/LPImageWin.h"

using namespace Gdiplus;

LPImageWin::LPImageWin(void):
  m_hDC(nullptr)
{
}

LPImageWin::~LPImageWin(void) {
  // Set the DC to null.  This will release any resources associated with
  // the current DC.
  SetDC(nullptr);
}

LPImage* LPImage::New(void) {
  return new LPImageWin;
}

bool LPImageWin::SetImage(const SIZE& size, const POINT* pHotspot) {
  Bitmap bmp(static_cast<INT>(size.cx), static_cast<INT>(size.cy), PixelFormat32bppARGB);

  if (bmp.GetLastStatus() != Ok) {
    return false;
  }

  return SetImageFromBitmap(bmp, pHotspot, false);
}

bool LPImageWin::SetImageFromBitmap(Gdiplus::Bitmap& src, const POINT* pHotspot, bool write) {
  BitmapData bitmapData;

  // Lock bits in the passed source image:
  if (src.LockBits(&Rect(0, 0, src.GetWidth(), src.GetHeight()), ImageLockModeRead, PixelFormat32bppARGB, &bitmapData) != Ok)
    return false;
  if (bitmapData.PixelFormat != PixelFormat32bppARGB)
    return false;

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

void LPImageWin::SetDC(HDC hDC, const POINT* pHotspot) {
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

bool LPImageWin::SetImage(const std::wstring& filename, const POINT* pHotspot) {
  Bitmap bmp(filename.c_str());
  if (bmp.GetLastStatus() != Ok)
    return false;

  return SetImageFromBitmap(bmp, pHotspot, true);
}

bool LPImageWin::InitImage(long width, long height, const POINT* pHotspot) {
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
}