// Copyright (c) 2010 - 2013 Leap Motion. All rights reserved. Proprietary and confidential.
#ifndef _STDAFX_H
#define _STDAFX_H

#include "common.h"

#if _WIN32

#include "DataStructures/BoundedQueue.h"
#include "Utility/Heartbeat.h"

#include <math.h>

#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <Psapi.h>
#include <SetupAPI.h>
#include <Shlwapi.h>
#include <algorithm>
using std::min;
using std::max;
#include <objidl.h>
#include <GdiPlus.h>

#include "FocusAppInfo.h"

#include <algorithm>
#include <stdexcept>
#include <stdint.h>
#include <string>
#include <vector>

HMODULE GetHmodUser32(void);

#endif // _WIN32


#endif
