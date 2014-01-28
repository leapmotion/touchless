// Copyright (c) 2010 - 2013 Leap Motion. All rights reserved. Proprietary and confidential.
#ifndef _STDAFX_H
#define _STDAFX_H

#include "common.h"

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

#if _WIN32
// Some hard-to-compile boost math modules:
#include <boost/chrono/chrono.hpp>
#include <boost/math/distributions/fisher_f.hpp>
#include <boost/thread.hpp>
#include <Eigen/SVD>

#endif // _WIN32

#endif
