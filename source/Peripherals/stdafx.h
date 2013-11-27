// Copyright (c) 2010 - 2013 Leap Motion. All rights reserved. Proprietary and confidential.
#ifndef _STDAFX_H
#define _STDAFX_H

#include "ocuConfig.h"

#if _WIN32

#include "AlgorithmUtil.h"
#include "BoundedQueue.h"
#include "DataTypes.h"
#include "Heartbeat.h"
#include "MathUtil.h"
#include "TouchManager.h"
#include "DataStructures/Value.h"
#include "ocuType.h"

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

#include "Globals/Interface.h"
#include "OcuInterfaceCodes.h"
#include "PreprocFlags.h"
#include "FocusAppInfo.h"
#include "OcuHidInstance.h"
#include "OcuIcon.h"
#include "OcuImage.h"
#include "OcuInterface.h"
#include "hidsdi.h"
#include "targetver.h"

#include "LPIcon.h"
#include "LPImage.h"
#include "LPGeometry.h"
#include "LPGesture.h"
#include "LPScreen.h"
#include "LPVirtualScreen.h"
#include "TouchManager.h"

#include <algorithm>
#include <stdexcept>
#include <stdint.h>
#include <string>
#include <vector>

#include <boost/thread.hpp>

#endif // _WIN32

#endif
