// Copyright (c) 2010 - 2013 Leap Motion. All rights reserved. Proprietary and confidential.
#ifndef _STDAFX_H
#define _STDAFX_H

#define __STDC_LIMIT_MACROS

#include "ocuConfig.h"

#ifndef _MSC_VER
#ifdef __APPLE__
// workaround for clang+libc++ bug 'call to isnan() is ambiguous'
#include <math.h>
#else
#include <cmath>
#endif

#ifdef isnan
  #define _isnan isnan
#else
  namespace std {
    template<typename _Tp>
    int isnan(_Tp val);
  }

  template<typename _Tp>
  int _isnan(_Tp val) {return std::isnan(val);}
#endif

#endif

#endif
