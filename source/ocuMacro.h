/*==================================================================================================================

    Copyright (c) 2010 - 2012 Leap Motion. All rights reserved.

  The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
  protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
  strictly forbidden unless prior written permission is obtained from Leap Motion.

===================================================================================================================*/

#ifndef __ocu_Macro_h__
#define __ocu_Macro_h__

#include "ocuConfig.h"
#include <iostream>
#include <cassert>

/// static (compile-time) assert macro.
/// Cond is any expression that can be fully evaluated at compile time.
/// Msg must be a valid C identifier suffix (allowed characters are alphanumeric and underscore)
/// e.g. LEAP_STATIC_ASSERT(sizeof(void*) == 4 || sizeof(void*) == 8, 32Or64BitPointersRequired);
/// an attempt to compile on a 16 bit platform would result in sizeof(void*) returning 2.
/// a compile-time error would occur and the text
/// '_LeapStaticAssert_32Or64BitPointersRequired': declared as an array with negative size
/// (or a similar message) would appear in the error output.
/// this macro will function in C as well as C++ and is cross-platform
#define LEAP_STATIC_ASSERT(Cond, Msg) \
  typedef int _LeapStaticAssert_##Msg[(Cond) ? 1 : -1]

extern bool __DEBUG_OUTPUT_FLAG;
extern bool __ERROR_OUTPUT_FLAG;

#define ocuDebugMacro(message) \
  if (__DEBUG_OUTPUT_FLAG) \
  { \
    std::cout << message << std::endl; \
  }

#define ocuErrorMacro(message) \
  if (__ERROR_OUTPUT_FLAG) \
  { \
    std::cout << message << std::endl; \
  }

#define LEAP_ASSERT(cond, message) \
  if (__ERROR_OUTPUT_FLAG && !(cond)) \
  { \
    std::cout << message << std::endl; \
  } \
  assert((cond));

#define ocuSetMacro(name,type) \
  virtual void Set##name (const type _arg) \
  { \
    if (this->m_##name != _arg) \
      { \
      this->m_##name = _arg; \
      } \
  }

#define ocuGetMacro(name,type) \
  virtual const type Get##name () const \
  { \
    return this->m_##name; \
  }

#define ocuGetSetMacro(name,type) \
  ocuGetMacro(name,type)  \
  ocuSetMacro(name,type)

#define ocuNew1DArrayMacro(name,type,size) \
  this->name = new type [size]();

#define ocuDelete1DArrayMacro(name) \
  if (this->name != NULL) \
  { \
    delete [] this->name; \
    this->name = NULL; \
  }

#define ocuNew2DArrayMacro(name,type,row,col) \
  ocuNew1DArrayMacro(name, type*, row); \
  for (int i=0; i < row; i++) \
  { \
    ocuNew1DArrayMacro(name[i], type, col); \
  }

#define ocuDelete2DArrayMacro(name,row) \
  for (int i=0; i < row; i++) \
  { \
    ocuDelete1DArrayMacro(name[i]); \
  } \
  ocuDelete1DArrayMacro(name);

#define ocuNewStatic1DArrayMacro(name,type,size) \
  if (name != NULL) \
  { \
    delete [] name; \
  } \
  name = new type [size]();

#define ocuDeleteStatic1DArrayMacro(name) \
  if (name != NULL) \
  { \
    delete [] name; \
    name = NULL; \
  }

#define ocuNewStatic2DArrayMacro(name,type,row,col) \
  if (name != NULL) \
  { \
    ocuDeleteStatic2DArrayMacro(name, row); \
  } \
  ocuNewStatic1DArrayMacro(name, type*, row); \
  for (int i=0; i < row; i++) \
  { \
    ocuNewStatic1DArrayMacro(name[i], type, col); \
  }

#define ocuDeleteStatic2DArrayMacro(name,row) \
  if (name != NULL) \
  { \
    for (int i=0; i < row; i++) \
    { \
      ocuDeleteStatic1DArrayMacro(name[i]); \
    } \
    ocuDeleteStatic1DArrayMacro(name); \
  }

#ifndef MATIAS_DEBUG
#define MATIAS_DEBUG 0
#endif

#ifndef MATIAS_COLLECT
#define MATIAS_COLLECT 0
#endif

//#define LOCALHOST "localhost"
#define LOCALHOST "127.0.0.1"

#endif //__ocu_Macro_h__
