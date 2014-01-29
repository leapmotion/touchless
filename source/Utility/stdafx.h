// Copyright (c) 2010 - 2013 Leap Motion. All rights reserved. Proprietary and confidential.
#ifndef _STDAFX_H
#define _STDAFX_H

#define _CRT_NONSTDC_NO_DEPRECATE

// Standard internal libraries:
#include "common.h"

#if _WIN32
#define _CRT_NONSTDC_NO_DEPRECATE

#include <math.h>
#include <float.h>
#include <time.h>

#include <Windows.h>
#include <Shellapi.h>
#include <TlHelp32.h>
#include <direct.h>
#include <psapi.h>

#include <vector>
#include <algorithm>
#include <iostream>
#include <istream>
#include <ostream>
#include <sstream>
#include <fstream>
#include <map>
#include <set>
#include <string>
#include <fstream>
#include <utility>
#include <cassert>
#include <climits>
#include <iomanip>

#endif // _WIN32

#endif
