#include "stdafx.h"
#include "Overlay/LPImageLinux.h"
#include "Utility/Value.h"

LPImageLinux::~LPImageLinux(void)
{
  if(m_colors)
    delete[] m_colors;
}

LPImage* LPImage::New(void) {
  return new LPImageLinux;
}

bool LPImageLinux::SetImage(const SIZE& size, const POINT* pHotspot) {
  return InitImage(static_cast<long>(size.cx), static_cast<long>(size.cy), pHotspot);
}

bool LPImageLinux::SetImage(const std::wstring& filename, const POINT* pHotspot) {
  return false; // TODO: write real code
}

bool LPImageLinux::InitImage(long width, long height, const POINT* pHotspot) {
  return false; // TODO: write real code
}