/*==================================================================================================================

    Copyright (c) 2010 - 2014 Leap Motion. All rights reserved.

  The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
  protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
  strictly forbidden unless prior written permission is obtained from Leap Motion.

===================================================================================================================*/

#ifndef __common_h__
#define __common_h__

#ifdef _WIN32
  #ifndef WIN32_LEAN_AND_MEAN
    #define WIN32_LEAN_AND_MEAN
  #endif

  #pragma warning(disable : 4482)  // enum used in namespace

  // Using deprecated POSIX names like stricmp, but that's OK
  #define _CRT_NONSTDC_NO_DEPRECATE

#endif // _WIN32

#if defined(_MSC_VER) && (_MSC_VER < 1600)
// Visual Studio 2008
typedef __int32 int32_t;
typedef __int64 int64_t;
typedef unsigned __int32 uint32_t;
#else
#include <stdint.h>
#endif

// installation constants
#if _WIN32 || __APPLE__
const char APPLICATION_DIRECTORY[] = "Leap Motion"; // use this directory located under [user]/AppData/Roaming/
#else
const char APPLICATION_DIRECTORY[] = ".Leap Motion"; // use this directory located under [user]/AppData/Roaming/
#endif

/// static (compile-time) assert macro.
/// Cond is any expression that can be fully evaluated at compile time.
/// Msg must be a valid C identifier suffix (allowed characters are alphanumeric and underscore)
/// e.g. LEAP_STATIC_ASSERT(sizeof(void*) == 4 || sizeof(void*) == 8, 32Or64BitPointersRequired);
/// an attempt to compile on a 16 bit platform would result in sizeof(void*) returning 2.
/// a compile-time error would occur and the text
/// '_LeapStaticAssert_32Or64BitPointersRequired': declared as an array with negative size
/// (or a similar message) would appear in the error output.
/// this macro will function in C as well as C++ and is cross-platform
#define STATIC_ASSERT(Cond, Msg) \
  typedef int _StaticAssert_##Msg[(Cond) ? 1 : -1]

//#define LOCALHOST "localhost"
#define LOCALHOST "127.0.0.1"

#ifdef __APPLE__
#include "C++11/cpp11.h"
#include RVALUE_HEADER
#endif

// Define integer types for Visual Studio 2008 and earlier
#if defined(_MSC_VER) && (_MSC_VER < 1600)
typedef __int32 int32_t;
typedef __int64 int64_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
#else
#include <stdint.h>
#endif

#ifdef _WIN64
  // warning C4244: 'argument' : conversion from '__int64' to 'int', possible loss of data
  #pragma warning(push)
  #pragma warning(disable: 4244)
#endif

#if !_WIN32
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wunused-variable"
#endif

#include <Eigen/Dense>

#if !_WIN32
  #pragma GCC diagnostic pop
#endif

//Standard Library
#include <vector>
#include <Eigen/StdVector>
#ifdef _WIN64
  #pragma warning(pop)
#endif

typedef double MATH_TYPE;

typedef Eigen::Matrix<MATH_TYPE, 2, 1> Vector2;
typedef Eigen::Matrix<MATH_TYPE, 3, 1> Vector3;

#endif // __common_h__