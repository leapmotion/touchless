#include "stdafx.h"
#include <algorithm>
using std::min;
using std::max;
#include <windows.h>
#include <objidl.h>
#include "GdiPlusInitializer.h"
#include <GdiPlus.h>

GdiPlusInitializer::GdiPlusInitializer(void):
  m_gdiplusToken(0)
{
  Gdiplus::GdiplusStartupInput startupInput;
  Gdiplus::GdiplusStartup(&m_gdiplusToken, &startupInput, nullptr);
}

GdiPlusInitializer::~GdiPlusInitializer(void) {
  Gdiplus::GdiplusShutdown(m_gdiplusToken);
}
