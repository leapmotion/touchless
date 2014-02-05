#include "stdafx.h"
#include "Overlay/LPImageMac.h"
#include "Utility/Value.h"
#include <NSData.h>
#include <NSString.h>
#include <NSURL.h>

LPImageMac::~LPImageMac(void)
{
  if(m_colors)
    delete[] m_colors;
}

LPImage* LPImage::New(void) {
  return new LPImageMac;
}

bool LPImageMac::SetImage(const SIZE& size, const POINT* pHotspot) {
  return InitImage(static_cast<long>(size.cx), static_cast<long>(size.cy), pHotspot);
}

bool LPImageMac::SetImage(const std::wstring& filename, const POINT* pHotspot) {
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
}

bool LPImageMac::InitImage(long width, long height, const POINT* pHotspot) {
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
  m_colors = new RGBQUAD[m_size.cx*m_size.cy];

  return status;
}