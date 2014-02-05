// Copyright (c) 2010 - 2013 Leap Motion. All rights reserved. Proprietary and confidential.
#include "stdafx.h"


HMODULE GetHmodUser32(void) {
  static HMODULE hmodUser32 = LoadLibraryW(L"user32");
  return hmodUser32;
}
