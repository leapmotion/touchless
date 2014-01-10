/*==================================================================================================================

    Copyright (c) 2010 - 2012 Leap Motion. All rights reserved.

  The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
  protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
  strictly forbidden unless prior written permission is obtained from Leap Motion.

===================================================================================================================*/
#ifndef __SSEFoo_h__
#define __SSEFoo_h__

#include "common.h"

#if defined(__SSE2__) || defined(_M_IX86) || defined(_M_AMD64)

#define _X_SSE4_2 42
#define _X_SSE4_1 41
#define _X_SSSE3  31
#define _X_SSE3   30
#define _X_SSE2   20

//Determine minimum version of SSE to require (Windows defaults to SSE2)
#if defined(__SSE4_2__)
#define HAS_SSE _X_SSE4_2
#include <nmmintrin.h>
#elif defined(__SSE4_1__)
#define HAS_SSE _X_SSE4_1
#include <smmintrin.h>
#elif defined(__SSSE3__)
#define HAS_SSE _X_SSSE3
#include <tmmintrin.h>
#elif defined(__SSE3__)
#define HAS_SSE _X_SSE3
#include <pmmintrin.h>
#else
#define HAS_SSE _X_SSE2
#include <emmintrin.h>
#endif

#include <iostream>
#include <math.h>

//Static meta-operations
template<int I> struct StaticLog2 { inline operator int() const { return I<=1 ? 0 : StaticLog2<I/2>()+1; }};
template<int I> struct StaticExp2 { inline operator int() const { return I<=0 ? 1 : StaticExp2<I-1>()*2; }};

#if defined(_WIN32) || defined(__MACH__)
  #define ALIGN16 __declspec(align(16))
#else
  #define ALIGN16 __attribute__ ((aligned (16)))
#endif

//Compile time macros
#define FIRST_BIT_DIV8(A) (A%2?A:A%4?A/2:A%8?A/4:A%16?A/8:A%32?A/16:A%64?A/32:A%128?A/64:A/128)
#define FIRST_BIT_MASK8(A) (A%2?0xFF:A%4?0xFE:A%8?0xFC:A%16?0xF8:A%32?0xF0:A%64?0xE0:A%128?0xC0:0x80)
#define FIRST_BIT_POS8(A) (A%2?0:A%4?1:A%8?2:A%16?3:A%32?4:A%64?5:A%128?6:7)

//Automatic headers
#define AUTO_EXTEND_OPER(oper, sseType, type) \
  inline sseType operator oper(const sseType& a, const type& b) { return a oper sseType(b); } \
  inline sseType operator oper(const type& b, const sseType& a) { return sseType(b) oper a; } \
  inline sseType operator oper(const sseType& a, const type*& b) { return a oper sseType(b); } \
  inline sseType operator oper(const type*& b, const sseType& a) { return sseType(b) oper a; }
#define AUTO_EXTEND_FULL(oper, sseType, type) AUTO_EXTEND_OPER(oper, sseType, type) \
  inline sseType& operator oper ## =(sseType& a, const sseType& b) { return a = a oper b; } \
  inline sseType& operator oper ## =(sseType& a, const type& b) { return a = a oper sseType(b); } \
  inline sseType& operator oper ## =(sseType& a, const type*& b) { return a = a oper sseType(b); } \
  inline type* operator oper ## =(type* a, const sseType& b) { a << (sseType(a) oper b); return a; }

//Identifiers for static class comparisons
#define SSEerrID 0
#define SSEcID 0x00000001
#define SSEsID 0x00000002
#define SSEiID 0x00000004
#define SSEfID 0x00000008
#define SSEdID 0x00000010

//Struct definitions
struct SSEc; struct SSEs; struct SSEi; struct SSEf; struct SSEd;

//####################################################################
//#  SSE Unsigned Char Struct
//####################################################################
struct SSEc {
  __m128i m;
  inline SSEc() {}
  inline explicit SSEc(const __m128i& _m) : m(_m) {}
  inline explicit SSEc(const unsigned char* p) : m(_mm_loadu_si128((__m128i const*) p)) {}
  inline explicit SSEc(const unsigned char& a) : m(_mm_set1_epi8(a)) {}
  inline explicit SSEc(const unsigned char& a, const unsigned char& b, const unsigned char& c, const unsigned char& d,
                       const unsigned char& e, const unsigned char& f, const unsigned char& g, const unsigned char& h,
                       const unsigned char& i, const unsigned char& j, const unsigned char& k, const unsigned char& l,
                       const unsigned char& m, const unsigned char& n, const unsigned char& o, const unsigned char& p)
                       : m(_mm_set_epi8(p, o, n, m, l, k, j, i, h, g, f, e, d, c, b, a)) {}
  inline operator __m128i() const { return m; }
  inline void save(unsigned char* p) const { _mm_storeu_ps((float*)p, _mm_castsi128_ps(m)); }
  inline void saveAligned(unsigned char* p) const { _mm_store_ps((float*)p, _mm_castsi128_ps(m)); }
  inline void save(unsigned char& b1, unsigned char& b2, unsigned char& b3, unsigned char& b4,
                   unsigned char& b5, unsigned char& b6, unsigned char& b7, unsigned char& b8,
                   unsigned char& b9, unsigned char& b10, unsigned char& b11, unsigned char& b12,
                   unsigned char& b13, unsigned char& b14, unsigned char& b15, unsigned char& b16) const {
    ALIGN16 unsigned char i[16]; saveAligned(i);
    b1 = i[0]; b2 = i[1]; b3 = i[2]; b4 = i[3];
    b5 = i[4]; b6 = i[5]; b7 = i[6]; b8 = i[7];
    b9 = i[8]; b10 = i[9]; b11 = i[10]; b12 = i[11];
    b13 = i[12]; b14 = i[13]; b15 = i[14]; b16 = i[15];
  }
  inline bool isAnyTrue() const { return _mm_movemask_epi8(m) != 0x0000; }
  inline bool isAnyFalse() const { return _mm_movemask_epi8(m) != 0xFFFF; }
  inline bool isAllTrue() const { return _mm_movemask_epi8(m) == 0xFFFF; }
  inline bool isAllFalse() const { return _mm_movemask_epi8(m) == 0x0000; }
  inline int truthMask() const { return _mm_movemask_epi8(m); }
  template<int I>
  inline SSEc lshift() const {
    return (SSEc)_mm_slli_si128(m, I);
  }
  template<int I>
  inline SSEc rshift() const {
    return (SSEc)_mm_srli_si128(m, I);
  }
  inline SSEc lshiftkeep() const {
    __m128i a = _mm_slli_si128(m, 1);
    __m128i b = _mm_slli_si128(m, 15);
    return (SSEc)_mm_or_si128(a, _mm_srli_si128(b, 15));
  }
  inline SSEc rshiftkeep() const {
    __m128i a = _mm_srli_si128(m, 1);
    __m128i b = _mm_srli_si128(m, 15);
    return (SSEc)_mm_or_si128(a, _mm_slli_si128(b, 15));
  }
  template<int I> inline SSEc div() const {
    STATIC_ASSERT(!(I&(I-1)), SSE_ERROR_CannotDivideByNonPowersOf2);
    const int i = StaticLog2<I>();
    const int mask = ((0xFF >> i) << 8) | (0xFF >> i);
    return (SSEc)_mm_and_si128(_mm_srli_epi16(m, i), _mm_set1_epi16(mask));
  }
  template<int I> inline SSEc sdiv() const {
    STATIC_ASSERT(!(I&(I-1)), SSE_ERROR_CannotDivideByNonPowersOf2);
    const int i = StaticLog2<I>();
    const int mask = ((0xFF >> i) << 8) | (0xFF >> i);
    return (SSEc)_mm_and_si128(_mm_srai_epi16(m, i), _mm_set1_epi16(mask));
  }
  template<int I> inline SSEc mul() const {
    STATIC_ASSERT(!(I&(I-1)), SSE_ERROR_CannotMultiplyByNonPowersOf2);
    return (SSEc)_mm_and_si128(_mm_slli_epi16(m, FIRST_BIT_POS8(I)), _mm_set1_epi16((unsigned short)(FIRST_BIT_MASK8(I)*257)));
  }
};

//####################################################################
//#  SSE Unsigned Short Struct
//####################################################################
struct SSEs {
  __m128i m;
  inline SSEs() {}
  inline explicit SSEs(const __m128i& _m) : m(_m) {}
  inline explicit SSEs(const unsigned short* p) : m(_mm_loadu_si128((__m128i const*) p)) {}
  inline explicit SSEs(const unsigned short& a) : m(_mm_set1_epi16(a)) {}
  inline explicit SSEs(const unsigned short& a, const unsigned short& b, const unsigned short& c, const unsigned short& d,
                       const unsigned short& e, const unsigned short& f, const unsigned short& g, const unsigned short& h)
                       : m(_mm_set_epi16(h, g, f, e, d, c, b, a)) {}
  inline operator __m128i() const { return m; }
  inline void save(unsigned short* p) const { _mm_storeu_ps((float*)p, _mm_castsi128_ps(m)); }
  inline void saveAligned(unsigned short* p) const { _mm_store_ps((float*)p, _mm_castsi128_ps(m)); }
  inline void save(unsigned short& b1, unsigned short& b2, unsigned short& b3, unsigned short& b4,
                   unsigned short& b5, unsigned short& b6, unsigned short& b7, unsigned short& b8) const {
    ALIGN16 unsigned short i[8]; saveAligned(i);
    b1 = i[0]; b2 = i[1]; b3 = i[2]; b4 = i[3];
    b5 = i[4]; b6 = i[5]; b7 = i[6]; b8 = i[7];
  }
  inline bool isAnyTrue() const { return _mm_movemask_epi8(m) != 0x0000; }
  inline bool isAnyFalse() const { return _mm_movemask_epi8(m) != 0xFFFF; }
  inline bool isAllTrue() const { return _mm_movemask_epi8(m) == 0xFFFF; }
  inline bool isAllFalse() const { return _mm_movemask_epi8(m) == 0x0000; }
  template<int I>
  inline SSEs lshift() const {
    return (SSEs)_mm_slli_si128(m, I);
  }
  template<int I>
  inline SSEs rshift() const {
    return (SSEs)_mm_srli_si128(m, I);
  }
  inline SSEs lshiftkeep() const {
    __m128i a = _mm_slli_si128(m, 2);
    __m128i b = _mm_slli_si128(m, 14);
    return (SSEs)_mm_or_si128(a, _mm_srli_si128(b, 14));
  }
  inline SSEs rshiftkeep() const {
    __m128i a = _mm_srli_si128(m, 2);
    __m128i b = _mm_srli_si128(m, 14);
    return (SSEs)_mm_or_si128(a, _mm_slli_si128(b, 14));
  }
  template<int I> inline SSEs div() const {
    STATIC_ASSERT(!(I&(I-1)), SSE_ERROR_CannotDivideByNonPowersOf2);
    return (SSEs)_mm_srli_epi16(m, StaticLog2<I>());
  }
  template<int I> inline SSEs sdiv() const {
    STATIC_ASSERT(!(I&(I-1)), SSE_ERROR_CannotDivideByNonPowersOf2);
    return (SSEs)_mm_srai_epi16(m, StaticLog2<I>());
  }
  template<int I> inline SSEs mul() const {
    STATIC_ASSERT(!(I&(I-1)), SSE_ERROR_CannotMultiplyByNonPowersOf2);
    return (SSEs)_mm_slli_epi16(m, StaticLog2<I>());
  }
};

//####################################################################
//#  SSE Signed Integer Struct
//####################################################################
struct SSEi {
  __m128i m;
  inline SSEi() {}
  inline explicit SSEi(const __m128i& _m) : m(_m) {}
  inline explicit SSEi(const __m128& _m) : m(_mm_cvttps_epi32(_m)) {}
  inline explicit SSEi(const int* p) : m(_mm_loadu_si128((__m128i const*) p)) {}
  inline explicit SSEi(const int& a) : m(_mm_set1_epi32(a)) {}
  inline explicit SSEi(const int& a, const int& b, const int& c, const int& d) : m(_mm_set_epi32(d, c, b, a)) {}
  inline operator __m128i() const { return m; }
  inline SSEf castFloat() const;
  inline void save(int* p) const { _mm_storeu_ps((float*)p, _mm_castsi128_ps(m)); }
  inline void saveAligned(int* p) const { _mm_store_ps((float*)p, _mm_castsi128_ps(m)); }
  inline void save(int& b1, int& b2, int& b3, int& b4) const {
    ALIGN16 int i[4]; saveAligned(i);
    b1 = i[0]; b2 = i[1]; b3 = i[2]; b4 = i[3];
  }
  inline bool isAnyTrue() const { return _mm_movemask_epi8(m) != 0x0000; }
  inline bool isAnyFalse() const { return _mm_movemask_epi8(m) != 0xFFFF; }
  inline bool isAllTrue() const { return _mm_movemask_epi8(m) == 0xFFFF; }
  inline bool isAllFalse() const { return _mm_movemask_epi8(m) == 0x0000; }
  template<int I> inline SSEi div() const {
    STATIC_ASSERT(!(I&(I-1)), SSE_ERROR_CannotDivideByNonPowersOf2);
    return (SSEi)_mm_srai_epi32(m, StaticLog2<I>());
  }
  inline SSEi operator [](const unsigned char* const ptr) const;
  inline SSEf operator [](const float* const ptr) const;
};

//####################################################################
//#  SSE Float Struct
//####################################################################
struct SSEf {
  __m128 m;
  inline SSEf() {}
  inline explicit SSEf(const __m128& _m) : m(_m) {}
  inline explicit SSEf(const __m128i& _m) : m(_mm_cvtepi32_ps(_m)) {}
  inline explicit SSEf(const float* p) : m(_mm_loadu_ps(p)) {}
  inline explicit SSEf(const float& a) : m(_mm_set1_ps(a)) {}
  inline explicit SSEf(const float& a, const float& b, const float& c, const float& d) : m(_mm_set_ps(d, c, b, a)) {}
  inline operator __m128() const { return m; }
  inline SSEi castInt() const;
  inline void save(float* p) const { _mm_storeu_ps(p, m); }
  inline void saveAligned(float* p) const { _mm_store_ps(p, m); }
  inline void save(float& b1, float& b2, float& b3, float& b4) const {
    ALIGN16 float i[4]; saveAligned(i);
    b1 = i[0]; b2 = i[1]; b3 = i[2]; b4 = i[3];
  }
  inline bool isAnyTrue() const { return _mm_movemask_epi8(_mm_castps_si128(m)) != 0x0000; }
  inline bool isAnyFalse() const { return _mm_movemask_epi8(_mm_castps_si128(m)) != 0xFFFF; }
  inline bool isAllTrue() const { return _mm_movemask_epi8(_mm_castps_si128(m)) == 0xFFFF; }
  inline bool isAllFalse() const { return _mm_movemask_epi8(_mm_castps_si128(m)) == 0x0000; }
};

//####################################################################
//#  SSE Double Struct
//####################################################################
struct SSEd {
  __m128d m;
  inline SSEd() {}
  inline explicit SSEd(const __m128d& _m) : m(_m) {}
  inline explicit SSEd(const double* p) : m(_mm_loadu_pd(p)) {}
  inline explicit SSEd(const double& a) : m(_mm_set1_pd(a)) {}
  inline explicit SSEd(const double& a, const double& b) : m(_mm_set_pd(b, a)) {}
  inline operator __m128d() const { return m; }
  inline void save(double* p) const { _mm_storeu_pd(p, m); }
  inline void saveAligned(double* p) const { _mm_store_pd(p, m); }
  inline void save(double& b1, double& b2) const {
    ALIGN16 double i[2]; saveAligned(i);
    b1 = i[0]; b2 = i[1];
  }
  inline bool isAnyTrue() const { return _mm_movemask_epi8(_mm_castpd_si128(m)) != 0x0000; }
  inline bool isAnyFalse() const { return _mm_movemask_epi8(_mm_castpd_si128(m)) != 0xFFFF; }
  inline bool isAllTrue() const { return _mm_movemask_epi8(_mm_castpd_si128(m)) == 0xFFFF; }
  inline bool isAllFalse() const { return _mm_movemask_epi8(_mm_castpd_si128(m)) == 0x0000; }
};

//####################################################################
//#  SSE Zero
//####################################################################
struct SSEZero {
  inline SSEZero() {}
  inline operator __m128() const { return _mm_setzero_ps(); }
  inline operator __m128i() const { return _mm_setzero_si128(); }
  inline operator __m128d() const { return _mm_setzero_pd(); }
  inline operator SSEc() const { return (SSEc)_mm_setzero_si128(); }
  inline operator SSEs() const { return (SSEs)_mm_setzero_si128(); }
  inline operator SSEi() const { return (SSEi)_mm_setzero_si128(); }
  inline operator SSEf() const { return (SSEf)_mm_setzero_ps(); }
  inline operator SSEd() const { return (SSEd)_mm_setzero_pd(); }
};

//Static Casters
inline SSEf SSEi::castFloat() const {
  return (SSEf)_mm_castsi128_ps(m);
}
inline SSEi SSEf::castInt() const {
  return (SSEi)_mm_castps_si128(m);
}

//Memory addressing
inline SSEi SSEi::operator [](const unsigned char* const ptr) const {
  ALIGN16 unsigned int i[4];
  saveAligned((int*)i);
  i[0] = (unsigned int)ptr[i[0]];
  i[1] = (unsigned int)ptr[i[1]];
  i[2] = (unsigned int)ptr[i[2]];
  i[3] = (unsigned int)ptr[i[3]];
  return (SSEi)_mm_load_si128((__m128i const*)i);
};
inline SSEf SSEi::operator [](const float* const ptr) const {
  ALIGN16 float i[4];
  saveAligned((int*)i);
  i[0] = ptr[((int*)i)[0]];
  i[1] = ptr[((int*)i)[1]];
  i[2] = ptr[((int*)i)[2]];
  i[3] = ptr[((int*)i)[3]];
  return (SSEf)_mm_load_ps(i);
};

//####################################################################
//#  Template Specialization
//####################################################################
template <class TT> struct SSE {
  static const int id = SSEerrID;
};
template<> struct SSE<unsigned char> : public SSEc {
  static const int size = 16;
  static const int id = SSEcID;
  inline SSE() {}
  inline SSE(const SSEc& a) { m = a; };
  inline SSE(const unsigned char& a) : SSEc(a) {}
  inline SSE(const unsigned char* a) : SSEc(a) {}
};
template<> struct SSE<unsigned short> : public SSEs {
  static const int size = 8;
  static const int id = SSEsID;
  inline SSE() {}
  inline SSE(const SSEs& a) { m = a; };
  inline SSE(const unsigned short& a) : SSEs(a) {}
  inline SSE(const unsigned short* a) : SSEs(a) {}
};
template<> struct SSE<int> : public SSEi {
  static const int size = 4;
  static const int id = SSEiID;
  inline SSE() {}
  inline SSE(const SSEi& a) { m = a; };
  inline SSE(const int& a) : SSEi(a) {}
  inline SSE(const int* a) : SSEi(a) {}
};
template<> struct SSE<float> : public SSEf {
  static const int size = 4;
  static const int id = SSEfID;
  inline SSE() {}
  inline SSE(const SSEf& a) { m = a; };
  inline SSE(const float& a) : SSEf(a) {}
  inline SSE(const float* a) : SSEf(a) {}
};
template<> struct SSE<double> : public SSEd {
  static const int size = 2;
  static const int id = SSEdID;
  inline SSE() {}
  inline SSE(const SSEd& a) { m = a; };
  inline SSE(const double& a) : SSEd(a) {}
  inline SSE(const double* a) : SSEd(a) {}
};

//####################################################################
//#  Output Streams
//####################################################################
inline std::ostream& operator<<(std::ostream &os, const SSEc& a) {
  unsigned char i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12, i13, i14, i15, i16;
  a.save(i1, i2, i3, i4, i5, i6, i7, i8, i9, i10, i11, i12, i13, i14, i15, i16);
  os << (int)i1 << " " << (int)i2 << " " << (int)i3 << " " << (int)i4 << " "
     << (int)i5 << " " << (int)i6 << " " << (int)i7 << " " << (int)i8 << " "
     << (int)i9 << " " << (int)i10 << " " << (int)i11 << " " << (int)i12 << " "
     << (int)i13 << " " << (int)i14 << " " << (int)i15 << " " << (int)i16;
  return os;
}
inline std::ostream& operator<<(std::ostream &os, const SSEs& a) {
  unsigned short i1, i2, i3, i4, i5, i6, i7, i8;
  a.save(i1, i2, i3, i4, i5, i6, i7, i8);
  os << i1 << " " << i2 << " " << i3 << " " << i4 << " "
     << i5 << " " << i6 << " " << i7 << " " << i8;
  return os;
}
inline std::ostream& operator<<(std::ostream &os, const SSEi& a) {
  int i1, i2, i3, i4;
  a.save(i1, i2, i3, i4);
  os << i1 << " " << i2 << " " << i3 << " " << i4;
  return os;
}
inline std::ostream& operator<<(std::ostream &os, const SSEf& a) {
  float f1, f2, f3, f4;
  a.save(f1, f2, f3, f4);
  os << f1 << " " << f2 << " " << f3 << " " << f4;
  return os;
}
inline std::ostream& operator<<(std::ostream &os, const SSEd& a) {
  double d1, d2;
  a.save(d1, d2);
  os << d1 << " " << d2;
  return os;
}
inline void operator<<(unsigned char* p, const SSEc& a) {
  a.save(p);
}
inline void operator<<(unsigned short* p, const SSEs& a) {
  a.save(p);
}
inline void operator<<(int* p, const SSEi& a) {
  a.save(p);
}
inline void operator<<(float* p, const SSEf& a) {
  a.save(p);
}
inline void operator<<(double* p, const SSEd& a) {
  a.save(p);
}
inline void operator<<(SSEc& a, const unsigned char* p) {
  a = SSEc(p);
}
inline void operator<<(SSEs& a, const unsigned short* p) {
  a = SSEs(p);
}
inline void operator<<(SSEi& a, const int* p) {
  a = SSEi(p);
}
inline void operator<<(SSEf& a, const float* p) {
  a = SSEf(p);
}
inline void operator<<(SSEd& a, const double* p) {
  a = SSEd(p);
}

//####################################################################
//#  Extract First Element
//####################################################################
inline unsigned char SSEExtract(const SSEc& a) {
  int result;
  _mm_store_ss((float*)&result, _mm_castsi128_ps(a));
  return (unsigned char)result;
}
inline unsigned short SSEExtract(const SSEs& a) {
  return (unsigned short)_mm_extract_epi16(a,0);
}
inline int SSEExtract(const SSEi& a) {
  int result;
  _mm_store_ss((float*)&result, _mm_castsi128_ps(a));
  return result;
}
inline float SSEExtract(const SSEf& a) {
  float result;
  _mm_store_ss(&result, a);
  return result;
}
inline double SSEExtract(const SSEd& a) {
  return _mm_cvtsd_f64(a);
}

//####################################################################
//#  Addition
//####################################################################
inline SSEc operator+(const SSEc& a, const SSEc& b) {
  return (SSEc)_mm_add_epi8(a, b);
}
inline SSEs operator+(const SSEs& a, const SSEs& b) {
  return (SSEs)_mm_add_epi16(a, b);
}
inline SSEi operator+(const SSEi& a, const SSEi& b) {
  return (SSEi)_mm_add_epi32(a, b);
}
inline SSEf operator+(const SSEf& a, const SSEf& b) {
  return (SSEf)_mm_add_ps(a, b);
}
inline SSEd operator+(const SSEd& a, const SSEd& b) {
  return (SSEd)_mm_add_pd(a, b);
}
AUTO_EXTEND_FULL(+,SSEc,unsigned char)
AUTO_EXTEND_FULL(+,SSEs,unsigned short)
AUTO_EXTEND_FULL(+,SSEi,int)
AUTO_EXTEND_FULL(+,SSEf,float)
AUTO_EXTEND_FULL(+,SSEd,double)

//####################################################################
//#  Increment Operator
//####################################################################
inline SSEc& operator++(SSEc& a) {
  return a = a + 1;
}
inline SSEs& operator++(SSEs& a) {
  return a = a + 1;
}
inline SSEi& operator++(SSEi& a) {
  return a = a + 1;
}
inline SSEf& operator++(SSEf& a) {
  return a = a + 1.0f;
}
inline SSEd& operator++(SSEd& a) {
  return a = a + 1.0;
}
inline SSEc operator++(SSEc& a, int) {
  SSEc temp = a;
  a = a + 1;
  return temp;
}
inline SSEs operator++(SSEs& a, int) {
  SSEs temp = a;
  a = a + 1;
  return temp;
}
inline SSEi operator++(SSEi& a, int) {
  SSEi temp = a;
  a = a + 1;
  return temp;
}
inline SSEf operator++(SSEf& a, int) {
  SSEf temp = a;
  a = a + 1.0f;
  return temp;
}
inline SSEd operator++(SSEd& a, int) {
  SSEd temp = a;
  a = a + 1.0;
  return temp;
}

//####################################################################
//#  Subtraction
//####################################################################
inline SSEc operator-(const SSEc& a, const SSEc& b) {
  return (SSEc)_mm_sub_epi8(a, b);
}
inline SSEs operator-(const SSEs& a, const SSEs& b) {
  return (SSEs)_mm_sub_epi16(a, b);
}
inline SSEi operator-(const SSEi& a, const SSEi& b) {
  return (SSEi)_mm_sub_epi32(a, b);
}
inline SSEf operator-(const SSEf& a, const SSEf& b) {
  return (SSEf)_mm_sub_ps(a, b);
}
inline SSEd operator-(const SSEd& a, const SSEd& b) {
  return (SSEd)_mm_sub_pd(a, b);
}
AUTO_EXTEND_FULL(-,SSEc,unsigned char)
AUTO_EXTEND_FULL(-,SSEs,unsigned short)
AUTO_EXTEND_FULL(-,SSEi,int)
AUTO_EXTEND_FULL(-,SSEf,float)
AUTO_EXTEND_FULL(-,SSEd,double)

//####################################################################
//#  Subtraction Augmented Assignment
//####################################################################
inline SSEc& operator--(SSEc& a) {
  return a = a - 1;
}
inline SSEs& operator--(SSEs& a) {
  return a = a - 1;
}
inline SSEi& operator--(SSEi& a) {
  return a = a - 1;
}
inline SSEf& operator--(SSEf& a) {
  return a = a - 1.0f;
}
inline SSEd& operator--(SSEd& a) {
  return a = a - 1.0;
}
inline SSEc operator--(SSEc& a, int) {
  SSEc temp = a;
  a = a - 1;
  return temp;
}
inline SSEs operator--(SSEs& a, int) {
  SSEs temp = a;
  a = a - 1;
  return temp;
}
inline SSEi operator--(SSEi& a, int) {
  SSEi temp = a;
  a = a - 1;
  return temp;
}
inline SSEf operator--(SSEf& a, int) {
  SSEf temp = a;
  a = a - 1.0f;
  return temp;
}
inline SSEd operator--(SSEd& a, int) {
  SSEd temp = a;
  a = a - 1.0;
  return temp;
}

//####################################################################
//#  Negation
//####################################################################
inline SSEc operator-(const SSEc& a) {
  return (SSEc)_mm_sub_epi8(SSEZero(), a);
}
inline SSEs operator-(const SSEs& a) {
  return (SSEs)_mm_sub_epi16(SSEZero(), a);
}
inline SSEi operator-(const SSEi& a) {
  return (SSEi)_mm_sub_epi32(SSEZero(), a);
}
inline SSEf operator-(const SSEf& a) {
  return (SSEf)_mm_xor_ps(a, _mm_castsi128_ps(_mm_set1_epi32(0x80000000)));
}
inline SSEd operator-(const SSEd& a) {
  return (SSEd)_mm_xor_pd(a, _mm_castsi128_pd(_mm_setr_epi32(0x00000000,0x80000000,0x00000000,0x80000000)));
}

//####################################################################
//#  Multiplication
//####################################################################
inline SSEs operator*(const SSEs& a, const SSEs& b) {
  return (SSEs)_mm_mullo_epi16(a, b);
}
#if HAS_SSE >= _X_SSE4_1
inline SSEi operator*(const SSEi& a, const SSEi& b) {
  return (SSEi)_mm_mullo_epi32(a, b);
}
#else
inline SSEi operator*(const SSEi& a, const SSEi& b) {
  ALIGN16 int av[16], bv[16];
  a.saveAligned(av); b.saveAligned(bv);
  av[0] *= bv[0];
  av[1] *= bv[1];
  av[2] *= bv[2];
  av[3] *= bv[3];
  return SSEi(av);
}
#endif
inline SSEf operator*(const SSEf& a, const SSEf& b) {
  return (SSEf)_mm_mul_ps(a, b);
}
inline SSEd operator*(const SSEd& a, const SSEd& b) {
  return (SSEd)_mm_mul_pd(a, b);
}
AUTO_EXTEND_FULL(*,SSEs,unsigned short)
AUTO_EXTEND_OPER(*,SSEi,int)
AUTO_EXTEND_FULL(*,SSEf,float)
AUTO_EXTEND_FULL(*,SSEd,double)

//####################################################################
//#  Division
//####################################################################
inline SSEf operator/(const SSEf& a, const SSEf& b) {
  return (SSEf)_mm_div_ps(a, b);
}
inline SSEd operator/(const SSEd& a, const SSEd& b) {
  return (SSEd)_mm_div_pd(a, b);
}
inline SSEf operator/(const SSEf& a, const float& b) {
  return a * SSEf(1.0f/b);
}
inline SSEd operator/(const SSEd& a, const double& b) {
  return a * SSEd(1.0/b);
}
inline SSEf operator/(const float& a, const SSEf& b) {
  return SSEf(a) / b;
}
inline SSEd operator/(const double& a, const SSEd& b) {
  return SSEd(a) / b;
}
inline SSEf operator/(const SSEf& a, const float* b) {
  return a / SSEf(b);
}
inline SSEd operator/(const SSEd& a, const double* b) {
  return a / SSEd(b);
}
inline SSEf operator/(const float* a, const SSEf& b) {
  return SSEf(a) / b;
}
inline SSEd operator/(const double* a, const SSEd& b) {
  return SSEd(a) / b;
}

//####################################################################
//#  Division Augmented Assignment
//####################################################################
inline SSEf& operator/=(SSEf& a, const SSEf& b) {
  return a = a / b;
}
inline SSEd& operator/=(SSEd& a, const SSEd& b) {
  return a = a / b;
}
inline SSEf& operator/=(SSEf& a, const float& b) {
  return a = a / b;
}
inline SSEd& operator/=(SSEd& a, const double& b) {
  return a = a / b;
}
inline SSEf& operator/=(SSEf& a, const float* b) {
  return a = a / SSEf(b);
}
inline SSEd& operator/=(SSEd& a, const double* b) {
  return a = a / SSEd(b);
}

//####################################################################
//#  Equality
//####################################################################
inline SSEc operator==(const SSEc& a, const SSEc& b) {
  return (SSEc)_mm_cmpeq_epi8(a, b);
}
inline SSEs operator==(const SSEs& a, const SSEs& b) {
  return (SSEs)_mm_cmpeq_epi16(a, b);
}
inline SSEi operator==(const SSEi& a, const SSEi& b) {
  return (SSEi)_mm_cmpeq_epi32(a, b);
}
inline SSEf operator==(const SSEf& a, const SSEf& b) {
  return (SSEf)_mm_cmpeq_ps(a, b);
}
inline SSEd operator==(const SSEd& a, const SSEd& b) {
  return (SSEd)_mm_cmpeq_pd(a, b);
}
AUTO_EXTEND_OPER(==,SSEc,unsigned char)
AUTO_EXTEND_OPER(==,SSEs,unsigned short)
AUTO_EXTEND_OPER(==,SSEi,int)
AUTO_EXTEND_OPER(==,SSEf,float)
AUTO_EXTEND_OPER(==,SSEd,double)

//####################################################################
//#  Inequality
//####################################################################
inline SSEc operator!=(const SSEc& a, const SSEc& b) {
  return (SSEc)_mm_sub_epi8(_mm_set1_epi8(-1), _mm_cmpeq_epi8(a, b));
}
inline SSEs operator!=(const SSEs& a, const SSEs& b) {
  return (SSEs)_mm_sub_epi16(_mm_set1_epi16(-1), _mm_cmpeq_epi16(a, b));
}
inline SSEi operator!=(const SSEi& a, const SSEi& b) {
  return (SSEi)_mm_sub_epi32(_mm_set1_epi32(-1), _mm_cmpeq_epi32(a, b));
}
inline SSEf operator!=(const SSEf& a, const SSEf& b) {
  return (SSEf)_mm_cmpneq_ps(a, b);
}
inline SSEd operator!=(const SSEd& a, const SSEd& b) {
  return (SSEd)_mm_cmpneq_pd(a, b);
}
AUTO_EXTEND_OPER(!=,SSEc,unsigned char)
AUTO_EXTEND_OPER(!=,SSEs,unsigned short)
AUTO_EXTEND_OPER(!=,SSEi,int)
AUTO_EXTEND_OPER(!=,SSEf,float)
AUTO_EXTEND_OPER(!=,SSEd,double)

//####################################################################
//#  Greater Than
//####################################################################
inline SSEc operator>(const SSEc& a, const SSEc& b) {
  SSEc c(128); return (SSEc)_mm_cmpgt_epi8(_mm_sub_epi8(a,c), _mm_sub_epi8(b,c));
}
inline SSEs operator>(const SSEs& a, const SSEs& b) {
  SSEs c(32768); return (SSEs)_mm_cmpgt_epi16(_mm_sub_epi16(a,c), _mm_sub_epi16(b,c));
}
inline SSEi operator>(const SSEi& a, const SSEi& b) {
  return (SSEi)_mm_cmpgt_epi32(a, b);
}
inline SSEf operator>(const SSEf& a, const SSEf& b) {
  return (SSEf)_mm_cmpgt_ps(a, b);
}
inline SSEd operator>(const SSEd& a, const SSEd& b) {
  return (SSEd)_mm_cmpgt_pd(a, b);
}
AUTO_EXTEND_OPER(>,SSEc,unsigned char)
AUTO_EXTEND_OPER(>,SSEs,unsigned short)
AUTO_EXTEND_OPER(>,SSEi,int)
AUTO_EXTEND_OPER(>,SSEf,float)
AUTO_EXTEND_OPER(>,SSEd,double)

//####################################################################
//#  Greater Than Or Equal To
//####################################################################
inline SSEc operator>=(const SSEc& a, const SSEc& b) {
  return (SSEc)_mm_cmpeq_epi8(_mm_max_epu8(a, b), a);
}
inline SSEs operator>=(const SSEs& a, const SSEs& b) {
  return (SSEs)_mm_cmpeq_epi16(_mm_subs_epu16(b, a), SSEZero());
}
inline SSEi operator>=(const SSEi& a, const SSEi& b) {
  return (SSEi)_mm_cmplt_epi32(b, a);
}
inline SSEf operator>=(const SSEf& a, const SSEf& b) {
  return (SSEf)_mm_cmpge_ps(a, b);
}
inline SSEd operator>=(const SSEd& a, const SSEd& b) {
  return (SSEd)_mm_cmpge_pd(a, b);
}
AUTO_EXTEND_OPER(>=,SSEc,unsigned char)
AUTO_EXTEND_OPER(>=,SSEs,unsigned short)
AUTO_EXTEND_OPER(>=,SSEi,int)
AUTO_EXTEND_OPER(>=,SSEf,float)
AUTO_EXTEND_OPER(>=,SSEd,double)

//####################################################################
//#  Less Than
//####################################################################
inline SSEc operator<(const SSEc& a, const SSEc& b) {
  SSEc c(128); return (SSEc)_mm_cmplt_epi8(_mm_sub_epi8(a,c), _mm_sub_epi8(b,c));
}
inline SSEs operator<(const SSEs& a, const SSEs& b) {
  SSEs c(32768); return (SSEs)_mm_cmplt_epi16(_mm_sub_epi16(a,c), _mm_sub_epi16(b,c));
}
inline SSEi operator<(const SSEi& a, const SSEi& b) {
  return (SSEi)_mm_cmplt_epi32(a, b);
}
inline SSEf operator<(const SSEf& a, const SSEf& b) {
  return (SSEf)_mm_cmplt_ps(a, b);
}
inline SSEd operator<(const SSEd& a, const SSEd& b) {
  return (SSEd)_mm_cmplt_pd(a, b);
}
AUTO_EXTEND_OPER(<,SSEc,unsigned char)
AUTO_EXTEND_OPER(<,SSEs,unsigned short)
AUTO_EXTEND_OPER(<,SSEi,int)
AUTO_EXTEND_OPER(<,SSEf,float)
AUTO_EXTEND_OPER(<,SSEd,double)

//####################################################################
//#  Less Than Or Equal To
//####################################################################
inline SSEc operator<=(const SSEc& a, const SSEc& b) {
  return (SSEc)_mm_cmpeq_epi8(_mm_min_epu8(a, b), a);
}
inline SSEs operator<=(const SSEs& a, const SSEs& b) {
  return (SSEs)_mm_cmpeq_epi16(_mm_subs_epu16(a, b), SSEZero());
}
inline SSEi operator<=(const SSEi& a, const SSEi& b) {
  return (SSEi)_mm_cmpgt_epi32(b, a);
}
inline SSEf operator<=(const SSEf& a, const SSEf& b) {
  return (SSEf)_mm_cmple_ps(a, b);
}
inline SSEd operator<=(const SSEd& a, const SSEd& b) {
  return (SSEd)_mm_cmple_pd(a, b);
}
AUTO_EXTEND_OPER(<=,SSEc,unsigned char)
AUTO_EXTEND_OPER(<=,SSEs,unsigned short)
AUTO_EXTEND_OPER(<=,SSEi,int)
AUTO_EXTEND_OPER(<=,SSEf,float)
AUTO_EXTEND_OPER(<=,SSEd,double)

//####################################################################
//#  Bitwise Compliment
//####################################################################
inline SSEc operator~(const SSEc& a) {
  return (SSEc)_mm_xor_si128(a, _mm_set1_epi32(0xffffffff));
}
inline SSEs operator~(const SSEs& a) {
  return (SSEs)_mm_xor_si128(a, _mm_set1_epi32(0xffffffff));
}
inline SSEi operator~(const SSEi& a) {
  return (SSEi)_mm_xor_si128(a, _mm_set1_epi32(0xffffffff));
}
inline SSEf operator~(const SSEf& a) {
  return (SSEf)_mm_xor_ps(a, _mm_castsi128_ps(_mm_set1_epi32(0xffffffff)));
}
inline SSEd operator~(const SSEd& a) {
  return (SSEd)_mm_xor_pd(a, _mm_castsi128_pd(_mm_set1_epi32(0xffffffff)));
}

//####################################################################
//#  Bitwise And
//####################################################################
inline SSEc operator&(const SSEc& a, const SSEc& b) {
  return (SSEc)_mm_and_si128(a, b);
}
inline SSEs operator&(const SSEs& a, const SSEs& b) {
  return (SSEs)_mm_and_si128(a, b);
}
inline SSEi operator&(const SSEi& a, const SSEi& b) {
  return (SSEi)_mm_and_si128(a, b);
}
inline SSEf operator&(const SSEf& a, const SSEf& b) {
  return (SSEf)_mm_and_ps(a, b);
}
inline SSEd operator&(const SSEd& a, const SSEd& b) {
  return (SSEd)_mm_and_pd(a, b);
}
AUTO_EXTEND_FULL(&,SSEc,unsigned char)
AUTO_EXTEND_FULL(&,SSEs,unsigned short)
AUTO_EXTEND_FULL(&,SSEi,int)
AUTO_EXTEND_FULL(&,SSEf,float)
AUTO_EXTEND_FULL(&,SSEd,double)

//####################################################################
//#  Bitwise Or
//####################################################################
inline SSEc operator|(const SSEc& a, const SSEc& b) {
  return (SSEc)_mm_or_si128(a, b);
}
inline SSEs operator|(const SSEs& a, const SSEs& b) {
  return (SSEs)_mm_or_si128(a, b);
}
inline SSEi operator|(const SSEi& a, const SSEi& b) {
  return (SSEi)_mm_or_si128(a, b);
}
inline SSEf operator|(const SSEf& a, const SSEf& b) {
  return (SSEf)_mm_or_ps(a, b);
}
inline SSEd operator|(const SSEd& a, const SSEd& b) {
  return (SSEd)_mm_or_pd(a, b);
}
AUTO_EXTEND_FULL(|,SSEc,unsigned char)
AUTO_EXTEND_FULL(|,SSEs,unsigned short)
AUTO_EXTEND_FULL(|,SSEi,int)
AUTO_EXTEND_FULL(|,SSEf,float)
AUTO_EXTEND_FULL(|,SSEd,double)

//####################################################################
//#  Bitwise Xor
//####################################################################
inline SSEc operator^(const SSEc& a, const SSEc& b) {
  return (SSEc)_mm_xor_si128(a, b);
}
inline SSEs operator^(const SSEs& a, const SSEs& b) {
  return (SSEs)_mm_xor_si128(a, b);
}
inline SSEi operator^(const SSEi& a, const SSEi& b) {
  return (SSEi)_mm_xor_si128(a, b);
}
inline SSEf operator^(const SSEf& a, const SSEf& b) {
  return (SSEf)_mm_xor_ps(a, b);
}
inline SSEd operator^(const SSEd& a, const SSEd& b) {
  return (SSEd)_mm_xor_pd(a, b);
}
AUTO_EXTEND_FULL(^,SSEc,unsigned char)
AUTO_EXTEND_FULL(^,SSEs,unsigned short)
AUTO_EXTEND_FULL(^,SSEi,int)
AUTO_EXTEND_FULL(^,SSEf,float)
AUTO_EXTEND_FULL(^,SSEd,double)

// //####################################################################
// //#  Ternary Operation
// //####################################################################
// inline SSEc SSETernary(const SSEc& condition, const SSEc& doTrue, const SSEc& doFalse) {
//   return (condition & doTrue) | (SSEc)_mm_andnot_si128(condition, doFalse);
// }
// inline SSEc SSETernary(const SSEc& condition, const unsigned char doTrue, const SSEc& doFalse) {
//   return (condition & doTrue) | (SSEc)_mm_andnot_si128(condition, doFalse);
// }
// inline SSEc SSETernary(const SSEc& condition, const SSEc& doTrue, const unsigned char doFalse) {
//   return (condition & doTrue) | (SSEc)_mm_andnot_si128(condition, (SSEc)doFalse);
// }
// inline SSEc SSETernary(const SSEc& condition, const unsigned char doTrue, const unsigned char doFalse) {
//   return (condition & doTrue) | (SSEc)_mm_andnot_si128(condition, (SSEc)doFalse);
// }
// inline SSEs SSETernary(const SSEs& condition, const SSEs& doTrue, const SSEs& doFalse) {
//   return (condition & doTrue) | (SSEs)_mm_andnot_si128(condition, doFalse);
// }
// inline SSEs SSETernary(const SSEs& condition, const unsigned short doTrue, const SSEs& doFalse) {
//   return (condition & doTrue) | (SSEs)_mm_andnot_si128(condition, doFalse);
// }
// inline SSEs SSETernary(const SSEs& condition, const SSEs& doTrue, const unsigned short doFalse) {
//   return (condition & doTrue) | (SSEs)_mm_andnot_si128(condition, (SSEs)doFalse);
// }
// inline SSEs SSETernary(const SSEs& condition, const unsigned short doTrue, const unsigned short doFalse) {
//   return (condition & doTrue) | (SSEs)_mm_andnot_si128(condition, (SSEs)doFalse);
// }
// inline SSEi SSETernary(const SSEi& condition, const SSEi& doTrue, const SSEi& doFalse) {
//   return (condition & doTrue) | (SSEi)_mm_andnot_si128(condition, doFalse);
// }
// inline SSEi SSETernary(const SSEi& condition, const int doTrue, const SSEi& doFalse) {
//   return (condition & doTrue) | (SSEi)_mm_andnot_si128(condition, doFalse);
// }
// inline SSEi SSETernary(const SSEi& condition, const SSEi& doTrue, const int doFalse) {
//   return (condition & doTrue) | (SSEi)_mm_andnot_si128(condition, (SSEi)doFalse);
// }
// inline SSEi SSETernary(const SSEi& condition, const int doTrue, const int doFalse) {
//   return (condition & doTrue) | (SSEi)_mm_andnot_si128(condition, (SSEi)doFalse);
// }
// inline SSEf SSETernary(const SSEf& condition, const SSEf& doTrue, const SSEf& doFalse) {
//   return (condition & doTrue) | (SSEf)_mm_andnot_ps(condition, doFalse);
// }
// inline SSEf SSETernary(const SSEf& condition, const float doTrue, const SSEf& doFalse) {
//   return (condition & doTrue) | (SSEf)_mm_andnot_ps(condition, doFalse);
// }
// inline SSEf SSETernary(const SSEf& condition, const SSEf& doTrue, const float doFalse) {
//   return (condition & doTrue) | (SSEf)_mm_andnot_ps(condition, (SSEf)doFalse);
// }
// inline SSEf SSETernary(const SSEf& condition, const float doTrue, const float doFalse) {
//   return (condition & doTrue) | (SSEf)_mm_andnot_ps(condition, (SSEf)doFalse);
// }
//
// //####################################################################
// //#  Square Root
// //####################################################################
// inline SSEf SSESqrt(const SSEf& a) {
//   return (SSEf)_mm_sqrt_ps(a);
// }
// inline SSEd SSESqrt(const SSEd& a) {
//   return (SSEd)_mm_sqrt_pd(a);
// }
// inline SSEf SSEInvSqrt(const SSEf& a) {
//   return (SSEf)_mm_rsqrt_ps(a);
// }
//
// //####################################################################
// //#  Minimum
// //####################################################################
// inline SSEc SSEMin(const SSEc& a, const SSEc& b) {
//   return (SSEc)_mm_min_epu8(a,b);
// }
// #if HAS_SSE >= _X_SSE4_1
// inline SSEs SSEMin(const SSEs& a, const SSEs& b) {
//   return (SSEs)_mm_min_epu16(a,b);
// }
// inline SSEi SSEMin(const SSEi& a, const SSEi& b) {
//   return (SSEi)_mm_min_epi32(a,b);
// }
// #else
// inline SSEs SSEMin(const SSEs& a, const SSEs& b) {
//   return SSETernary(a<b, a, b);
// }
// inline SSEi SSEMin(const SSEi& a, const SSEi& b) {
//   return SSETernary(a<b, a, b);
// }
// #endif
// inline SSEf SSEMin(const SSEf& a, const SSEf& b) {
//   return (SSEf)_mm_min_ps(a,b);
// }
// inline SSEd SSEMin(const SSEd& a, const SSEd& b) {
//   return (SSEd)_mm_min_pd(a,b);
// }
// inline SSEc SSEMin(const SSEc& a, const unsigned char& b) {
//   return SSEMin(a, SSEc(b));
// }
// inline SSEs SSEMin(const SSEs& a, const unsigned short& b) {
//   return SSEMin(a, SSEs(b));
// }
// inline SSEi SSEMin(const SSEi& a, const int& b) {
//   return SSEMin(a, SSEi(b));
// }
// inline SSEf SSEMin(const SSEf& a, const float& b) {
//   return SSEMin(a, SSEf(b));
// }
// inline SSEd SSEMin(const SSEd& a, const double& b) {
//   return SSEMin(a, SSEd(b));
// }
// inline SSEc SSEMin(const unsigned char& a, const SSEc& b) {
//   return SSEMin(SSEc(a), b);
// }
// inline SSEs SSEMin(const unsigned short& a, const SSEs& b) {
//   return SSEMin(SSEs(a), b);
// }
// inline SSEi SSEMin(const int& a, const SSEi& b) {
//   return SSEMin(SSEi(a), b);
// }
// inline SSEf SSEMin(const float& a, const SSEf& b) {
//   return SSEMin(SSEf(a), b);
// }
// inline SSEd SSEMin(const double& a, const SSEd& b) {
//   return SSEMin(SSEd(a), b);
// }
// inline unsigned char SSEMin(const SSEc& a) {
//   SSEc x = SSEMin(a, (SSEc)_mm_srli_si128(a,8));
//   x = SSEMin(x, (SSEc)_mm_srli_si128(x,4));
//   x = SSEMin(x, (SSEc)_mm_srli_si128(x,2));
//   x = SSEMin(x, (SSEc)_mm_srli_si128(x,1));
//   return SSEExtract(x);
// }
// inline unsigned short SSEMin(const SSEs& a) {
//   SSEs x = SSEMin(a, (SSEs)_mm_srli_si128(a,8));
//   x = SSEMin(x, (SSEs)_mm_srli_si128(x,4));
//   x = SSEMin(x, (SSEs)_mm_srli_si128(x,2));
//   return SSEExtract(x);
// }
// inline int SSEMin(const SSEi& a) {
//   SSEi x = SSEMin(a, (SSEi)_mm_srli_si128(a,8));
//   x = SSEMin(x, (SSEi)_mm_srli_si128(x,4));
//   return SSEExtract(x);
// }
// inline float SSEMin(const SSEf& a) {
//   SSEf x = SSEMin(a, (SSEf)_mm_srli_si128(a.castInt(),8));
//   x = SSEMin(x, (SSEf)_mm_srli_si128(x.castInt(),4));
//   return SSEExtract(x);
// }
//
// //####################################################################
// //#  Maximum
// //####################################################################
// inline SSEc SSEMax(const SSEc& a, const SSEc& b) {
//   return (SSEc)_mm_max_epu8(a,b);
// }
// #if HAS_SSE >= _X_SSE4_1
// inline SSEs SSEMax(const SSEs& a, const SSEs& b) {
//   return (SSEs)_mm_max_epu16(a,b);
// }
// inline SSEi SSEMax(const SSEi& a, const SSEi& b) {
//   return (SSEi)_mm_max_epi32(a,b);
// }
// #else
// inline SSEs SSEMax(const SSEs& a, const SSEs& b) {
//   return SSETernary(a>b, a, b);
// }
// inline SSEi SSEMax(const SSEi& a, const SSEi& b) {
//   return SSETernary(a>b, a, b);
// }
// #endif
// inline SSEf SSEMax(const SSEf& a, const SSEf& b) {
//   return (SSEf)_mm_max_ps(a,b);
// }
// inline SSEd SSEMax(const SSEd& a, const SSEd& b) {
//   return (SSEd)_mm_max_pd(a,b);
// }
// inline SSEc SSEMax(const SSEc& a, const unsigned char& b) {
//   return SSEMax(a, SSEc(b));
// }
// inline SSEs SSEMax(const SSEs& a, const unsigned short& b) {
//   return SSEMax(a, SSEs(b));
// }
// inline SSEi SSEMax(const SSEi& a, const int& b) {
//   return SSEMax(a, SSEi(b));
// }
// inline SSEf SSEMax(const SSEf& a, const float& b) {
//   return SSEMax(a, SSEf(b));
// }
// inline SSEd SSEMax(const SSEd& a, const double& b) {
//   return SSEMax(a, SSEd(b));
// }
// inline SSEc SSEMax(const unsigned char& a, const SSEc& b) {
//   return SSEMax(SSEc(a), b);
// }
// inline SSEs SSEMax(const unsigned short& a, const SSEs& b) {
//   return SSEMax(SSEs(a), b);
// }
// inline SSEi SSEMax(const int& a, const SSEi& b) {
//   return SSEMax(SSEi(a), b);
// }
// inline SSEf SSEMax(const float& a, const SSEf& b) {
//   return SSEMax(SSEf(a), b);
// }
// inline SSEd SSEMax(const double& a, const SSEd& b) {
//   return SSEMax(SSEd(a), b);
// }
// inline unsigned char SSEMax(const SSEc& a) {
//   SSEc x = SSEMax(a, (SSEc)_mm_srli_si128(a,8));
//   x = SSEMax(x, (SSEc)_mm_srli_si128(x,4));
//   x = SSEMax(x, (SSEc)_mm_srli_si128(x,2));
//   x = SSEMax(x, (SSEc)_mm_srli_si128(x,1));
//   return SSEExtract(x);
// }
// inline unsigned short SSEMax(const SSEs& a) {
//   SSEs x = SSEMax(a, (SSEs)_mm_srli_si128(a,8));
//   x = SSEMax(x, (SSEs)_mm_srli_si128(x,4));
//   x = SSEMax(x, (SSEs)_mm_srli_si128(x,2));
//   return SSEExtract(x);
// }
// inline int SSEMax(const SSEi& a) {
//   SSEi x = SSEMax(a, (SSEi)_mm_srli_si128(a,8));
//   x = SSEMax(x, (SSEi)_mm_srli_si128(x,4));
//   return SSEExtract(x);
// }
// inline float SSEMax(const SSEf& a) {
//   SSEf x = SSEMax(a, (SSEf)_mm_srli_si128(a.castInt(),8));
//   x = SSEMax(x, (SSEf)_mm_srli_si128(x.castInt(),4));
//   return SSEExtract(x);
// }
//
// //####################################################################
// //#  Absolute Value
// //####################################################################
// #if HAS_SSE >= _X_SSSE3
// inline SSEc SSEAbs(const SSEc& a) {
//   return (SSEc)_mm_abs_epi8(a);
// }
// inline SSEs SSEAbs(const SSEs& a) {
//   return (SSEs)_mm_abs_epi16(a);
// }
// inline SSEi SSEAbs(const SSEi& a) {
//   return (SSEi)_mm_abs_epi32(a);
// }
// #else
// inline SSEc SSEAbs(const SSEc& a) {
//   return SSEMin(-a, a);
// }
// inline SSEs SSEAbs(const SSEs& a) {
//   return SSEMin(-a, a);
// }
// inline SSEi SSEAbs(const SSEi& a) {
//   return SSEMax(-a, a);
// }
// #endif
// inline SSEf SSEAbs(const SSEf& a) {
//   return SSEMax(-a, a);
// }
// inline SSEd SSEAbs(const SSEd& a) {
//   return SSEMax(-a, a);
// }
//
// //####################################################################
// //#  Weighted Multiplication
// //####################################################################
// //Equivalent to (a*b)/256
// inline SSEc SSEWMul(const SSEc& a, const SSEc& b) {
//   SSEc e = SSEZero();
//   SSEc c = (SSEc)_mm_mullo_epi16(_mm_unpacklo_epi8(a, e), _mm_unpacklo_epi8(b, e));
//   SSEc d = (SSEc)_mm_mullo_epi16(_mm_unpackhi_epi8(a, e), _mm_unpackhi_epi8(b, e));
//   e = (SSEc)_mm_unpacklo_epi8(c, d);
//   c = (SSEc)_mm_unpackhi_epi8(c, d);
//   d = (SSEc)_mm_unpacklo_epi8(e, c);
//   e = (SSEc)_mm_unpackhi_epi8(e, c);
//   c = (SSEc)_mm_unpacklo_epi8(d, e);
//   d = (SSEc)_mm_unpackhi_epi8(d, e);
//   return (SSEc)_mm_unpackhi_epi8(c, d);
// }
// inline SSEs SSEWMul(const SSEs& a, const SSEs& b) {
//   return (SSEs)_mm_mulhi_epu16(a, b);
// }
// inline SSEf SSEWMul(const SSEf& a, const SSEf& b) {
//   return (a*b)/255.0f;
// }
// inline SSEd SSEWMul(const SSEd& a, const SSEd& b) {
//   return (a*b)/255.0;
// }
// inline SSEc SSEWMul(const SSEc& a, unsigned char b) {
//   return SSEWMul(a, SSEc(b));
// }
// inline SSEs SSEWMul(const SSEs& a, unsigned short b) {
//   return SSEWMul(a, SSEs(b));
// }
// inline SSEf SSEWMul(const SSEf& a, float b) {
//   return SSEWMul(a, SSEf(b));
// }
// inline SSEd SSEWMul(const SSEd& a, double b) {
//   return SSEWMul(a, SSEd(b));
// }

//####################################################################
//#  Average
//####################################################################
inline SSEc SSEAvg(const SSEc& a, const SSEc& b) {
  return (SSEc)_mm_avg_epu8(a, b);
}
inline SSEs SSEAvg(const SSEs& a, const SSEs& b) {
  return (SSEs)_mm_avg_epu16(a, b);
}
inline SSEi SSEAvg(const SSEi& a, const SSEi& b) {
  return (a+b).div<2>();
}
inline SSEf SSEAvg(const SSEf& a, const SSEf& b) {
  return (a+b)*0.5f;
}
inline SSEd SSEAvg(const SSEd& a, const SSEd& b) {
  return (a+b)*0.5;
}

// //####################################################################
// //#  Sign Copy
// //####################################################################
// #if HAS_SSE >= _X_SSSE3
// inline SSEc SSECopySign(const SSEc& a, const SSEc& b) {
//   return (SSEc)_mm_sign_epi8(a, b);
// }
// inline SSEs SSECopySign(const SSEs& a, const SSEs& b) {
//   return (SSEs)_mm_sign_epi16(a, b);
// }
// inline SSEi SSECopySign(const SSEi& a, const SSEi& b) {
//   return (SSEi)_mm_sign_epi32(a, b);
// }
// #endif
//
// //####################################################################
// //#  Clamped Addition and Subtraction
// //####################################################################
// inline SSEc SSEAddClamp(const SSEc& a, const SSEc& b) {
//   return (SSEc)_mm_adds_epu8(a, b);
// }
// inline SSEs SSEAddClamp(const SSEs& a, const SSEs& b) {
//   return (SSEs)_mm_adds_epu16(a, b);
// }
// inline SSEf SSEAddClamp(const SSEf& a, const SSEf& b) {
//   return a + b;
// }
// inline SSEc SSEAddClamp(const SSEc& a, const unsigned char& b) {
//   return (SSEc)_mm_adds_epu8(a, SSEc(b));
// }
// inline SSEs SSEAddClamp(const SSEs& a, const unsigned short& b) {
//   return (SSEs)_mm_adds_epu16(a, SSEs(b));
// }
// inline SSEf SSEAddClamp(const SSEf& a, const float b) {
//   return a + b;
// }
// inline SSEc SSESubClamp(const SSEc& a, const SSEc& b) {
//   return (SSEc)_mm_subs_epu8(a, b);
// }
// inline SSEs SSESubClamp(const SSEs& a, const SSEs& b) {
//   return (SSEs)_mm_subs_epu16(a, b);
// }
// inline SSEf SSESubClamp(const SSEf& a, const SSEf& b) {
//   return a - b;
// }
// inline SSEc SSESubClamp(const SSEc& a, const unsigned char& b) {
//   return (SSEc)_mm_subs_epu8(a, SSEc(b));
// }
// inline SSEs SSESubClamp(const SSEs& a, const unsigned short& b) {
//   return (SSEs)_mm_subs_epu16(a, SSEs(b));
// }
// inline SSEf SSESubClamp(const SSEf& a, const float b) {
//   return a - b;
// }
// inline SSEc SSESubLimit(const SSEc& a, const SSEc& b) {
//   SSEc d = a - b;
//   return SSETernary(a > b, SSEMin(d, 127), SSEMax(d, 128));
// }
// inline SSEs SSESubLimit(const SSEs& a, const SSEs& b) {
//   return a - b;
// }
// inline SSEf SSESubLimit(const SSEf& a, const SSEf& b) {
//   return a - b;
// }
//
// //####################################################################
// //#  Horizontal Operations
// //####################################################################
// #if HAS_SSE >= _X_SSSE3
// inline int SSESum(const SSEi& a) {
//   SSEi x = (SSEi)_mm_hadd_epi32(a,a);
//   x = (SSEi)_mm_hadd_epi32(x,x);
//   return SSEExtract(x);
// }
// inline float SSESum(const SSEf& a) {
//   SSEf x = (SSEf)_mm_hadd_ps(a,a);
//   x = (SSEf)_mm_hadd_ps(x,x);
//   return SSEExtract(x);
// }
// inline double SSESum(const SSEd& a) {
//   SSEd x = (SSEd)_mm_hadd_pd(a,a);
//   return SSEExtract(x);
// }
// #elif HAS_SSE >= _X_SSE2
// inline int SSESum(const SSEi& a) {
//   SSEi x = a + (SSEi)_mm_srli_si128(a,8);
//   x += (SSEi)_mm_srli_si128(x,4);
//   return SSEExtract(x);
// }
// inline float SSESum(const SSEf& a) {
//   SSEf x = a + (SSEf)_mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(a),8));
//   x += (SSEf)_mm_castsi128_ps(_mm_srli_si128(_mm_castps_si128(x),4));
//   return SSEExtract(x);
// }
// inline double SSESum(const SSEd& a) {
//   SSEd x = a + (SSEd)_mm_castsi128_pd(_mm_srli_si128(_mm_castpd_si128(a),8));
//   return _mm_cvtsd_f64(x);
// }
// #endif
// inline int SSESum(const SSEs& a) {
//   SSEs zero = SSEZero();
//   SSEi b = (SSEi)_mm_unpacklo_epi16(a, zero);
//   SSEi c = (SSEi)_mm_unpackhi_epi16(a, zero);
//   return SSESum(b + c);
// }
// inline unsigned char SSEAvg(const SSEc& a) {
//   SSEc x = SSEAvg(a,(SSEc)_mm_srli_si128(a,8));
//   x = SSEAvg(x,(SSEc)_mm_srli_si128(x,4));
//   x = SSEAvg(x,(SSEc)_mm_srli_si128(x,2));
//   x = SSEAvg(x,(SSEc)_mm_srli_si128(x,1));
//   return SSEExtract(x);
// }
// inline unsigned short SSEAvg(const SSEs& a) {
//   SSEs x = SSEAvg(a,(SSEs)_mm_srli_si128(a,8));
//   x = SSEAvg(x,(SSEs)_mm_srli_si128(x,4));
//   x = SSEAvg(x,(SSEs)_mm_srli_si128(x,2));
//   return SSEExtract(x);
// }
// inline int SSEAvg(const SSEi& a) {
//   SSEi x = SSEAvg(a,(SSEi)_mm_srli_si128(a,8));
//   x = SSEAvg(x,(SSEi)_mm_srli_si128(x,4));
//   return SSEExtract(x);
// }
// inline float SSEAvg(const SSEf& a) {
//   return SSESum(a)*0.25f;
// }
// inline double SSEAvg(const SSEd& a) {
//   return SSESum(a)*0.5;
// }
//
// //####################################################################
// //#  Rotate
// //####################################################################
// inline SSEi SSERotateLeft(const SSEi& a) {
//   return (SSEi)_mm_shuffle_epi32(a, 0x39);
// }
// inline SSEf SSERotateLeft(const SSEf& a) {
//   return (SSEf)_mm_shuffle_ps(a, a, 0x39);
// }
// inline SSEi SSERotateRight(const SSEi& a) {
//   return (SSEi)_mm_shuffle_epi32(a, 0x93);
// }
// inline SSEf SSERotateRight(const SSEf& a) {
//   return (SSEf)_mm_shuffle_ps(a, a, 0x93);
// }
//
// //####################################################################
// //#  Interleaving
// //####################################################################
// //Deinterleave from memory
// inline void SSEDeinterleave(const unsigned char* p, SSEc& a, SSEc& b) {
//   SSEc c;
//   a << p;
//   b << p + 16;
//   c = (SSEc)_mm_unpacklo_epi8(a, b);
//   b = (SSEc)_mm_unpackhi_epi8(a, b);
//   a = (SSEc)_mm_unpacklo_epi8(c, b);
//   b = (SSEc)_mm_unpackhi_epi8(c, b);
//   c = (SSEc)_mm_unpacklo_epi8(a, b);
//   b = (SSEc)_mm_unpackhi_epi8(a, b);
//   a = (SSEc)_mm_unpacklo_epi8(c, b);
//   b = (SSEc)_mm_unpackhi_epi8(c, b);
// }
// inline void SSEDeinterleave(const unsigned short* p, SSEs& a, SSEs& b) {
//   SSEs c;
//   c << p;
//   b << p + 8;
//   a = (SSEs)_mm_unpacklo_epi16(c, b);
//   b = (SSEs)_mm_unpackhi_epi16(c, b);
//   c = (SSEs)_mm_unpacklo_epi16(a, b);
//   b = (SSEs)_mm_unpackhi_epi16(a, b);
//   a = (SSEs)_mm_unpacklo_epi16(c, b);
//   b = (SSEs)_mm_unpackhi_epi16(c, b);
// }
// inline void SSEDeinterleave(const int* p, SSEi& a, SSEi& b) {
//   SSEi c;
//   a << p;
//   b << p + 4;
//   c = (SSEi)_mm_unpacklo_epi32(a, b);
//   b = (SSEi)_mm_unpackhi_epi32(a, b);
//   a = (SSEi)_mm_unpacklo_epi32(c, b);
//   b = (SSEi)_mm_unpackhi_epi32(c, b);
// }
// inline void SSEDeinterleave(const float* p, SSEf& a, SSEf& b) {
//   SSEf c;
//   a << p;
//   b << p + 4;
//   c = (SSEf)_mm_unpacklo_ps(a, b);
//   b = (SSEf)_mm_unpackhi_ps(a, b);
//   a = (SSEf)_mm_unpacklo_ps(c, b);
//   b = (SSEf)_mm_unpackhi_ps(c, b);
// }
//
// //Interleaves to memory
// inline void SSEInterleave(unsigned char* p, const SSEc& a, const SSEc& b) {
//   p << (SSEc)_mm_unpacklo_epi8(a, b);
//   (p + 16) << (SSEc)_mm_unpackhi_epi8(a, b);
// }
// inline void SSEInterleave(unsigned short* p, const SSEs& a, const SSEs& b) {
//   p << (SSEs)_mm_unpacklo_epi16(a, b);
//   (p + 8) << (SSEs)_mm_unpackhi_epi16(a, b);
// }
// inline void SSEInterleave(int* p, const SSEi& a, const SSEi& b) {
//   p << (SSEi)_mm_unpacklo_epi32(a, b);
//   (p + 4) << (SSEi)_mm_unpackhi_epi32(a, b);
// }
// inline void SSEInterleave(float* p, const SSEf& a, const SSEf& b) {
//   p << (SSEf)_mm_unpacklo_ps(a, b);
//   (p + 4) << (SSEf)_mm_unpackhi_ps(a, b);
// }
//
// //Interleaves to register
// inline void SSEInterleave(const SSEc& a, const SSEc& b, SSEc& c, SSEc& d) {
//   c = (SSEc)_mm_unpacklo_epi8(a, b);
//   d = (SSEc)_mm_unpackhi_epi8(a, b);
// }
// inline void SSEInterleave(const SSEs& a, const SSEs& b, SSEs& c, SSEs& d) {
//   c = (SSEs)_mm_unpacklo_epi16(a, b);
//   d = (SSEs)_mm_unpackhi_epi16(a, b);
// }
// inline void SSEInterleave(const SSEi& a, const SSEi& b, SSEi& c, SSEi& d) {
//   c = (SSEi)_mm_unpacklo_epi32(a, b);
//   d = (SSEi)_mm_unpackhi_epi32(a, b);
// }
// inline void SSEInterleave(const SSEf& a, const SSEf& b, SSEf& c, SSEf& d) {
//   c = (SSEf)_mm_unpacklo_ps(a, b);
//   d = (SSEf)_mm_unpackhi_ps(a, b);
// }
//
// //####################################################################
// //#  Reduction
// //####################################################################
// //Reduce a block of memory into a register half the size by neighbor averaging
// inline SSEc SSEReduce(const unsigned char* p) {
//   SSEc a, b;
//   SSEDeinterleave(p, a, b);
//   return (SSEc)_mm_avg_epu8(a, b);
// }
// inline SSEs SSEReduce(const unsigned short* p) {
//   SSEs a, b;
//   SSEDeinterleave(p, a, b);
//   return (SSEs)_mm_avg_epu16(a, b);
// }
// inline SSEi SSEReduce(const int* p) {
//   SSEi a, b;
//   SSEDeinterleave(p, a, b);
//   return (SSEi)_mm_srai_epi32(a + b, 1);
// }
// inline SSEf SSEReduce(const float* p) {
//   SSEf a, b;
//   SSEDeinterleave(p, a, b);
//   return (a + b)*0.5f;
// }
//
// //####################################################################
// //#  Expansion
// //####################################################################
// //Exapand a block of memory to two sse registers by duplicating neighbors
// inline void SSEExpand(const unsigned char* p, SSEc& left, SSEc& right) {
//   SSEc a(p);
//   left = (SSEc)_mm_unpacklo_epi8(a, a);
//   right = (SSEc)_mm_unpackhi_epi8(a, a);
// }
// inline void SSEExpand(const unsigned short* p, SSEs& left, SSEs& right) {
//   SSEs a(p);
//   left = (SSEs)_mm_unpacklo_epi16(a, a);
//   right = (SSEs)_mm_unpackhi_epi16(a, a);
// }
// inline void SSEExpand(const int* p, SSEi& left, SSEi& right) {
//   SSEi a(p);
//   left = (SSEi)_mm_unpacklo_epi32(a, a);
//   right = (SSEi)_mm_unpackhi_epi32(a, a);
// }
// inline void SSEExpand(const float* p, SSEf& left, SSEf& right) {
//   SSEf a(p);
//   left = (SSEf)_mm_unpacklo_ps(a, a);
//   right = (SSEf)_mm_unpackhi_ps(a, a);
// }
//
// //####################################################################
// //#  Reverse Order
// //####################################################################
// inline SSEc SSEReverse(const SSEc& a) {
//   SSEc b = (SSEc)_mm_shuffle_epi32(a, 0x1B);
//   b = (SSEc)_mm_shufflelo_epi16(b, 0xB1);
//   b = (SSEc)_mm_shufflehi_epi16(b, 0xB1);
//   return (SSEc)_mm_srli_epi16(b, 8) | (SSEc)_mm_slli_epi16(b, 8);
// }
// inline SSEs SSEReverse(const SSEs& a) {
//   SSEs b = (SSEs)_mm_shuffle_epi32(a, 0x1B);
//   b = (SSEs)_mm_shufflelo_epi16(b, 0xB1);
//   return (SSEs)_mm_shufflehi_epi16(b, 0xB1);
// }
// inline SSEi SSEReverse(const SSEi& a) {
//   return (SSEi)_mm_shuffle_epi32(a, 0x1B);
// }
// inline SSEf SSEReverse(const SSEf& a) {
//   return (SSEf)_mm_shuffle_ps(a, a, 0x1B);
// }
//
// //####################################################################
// //#  Sorting
// //####################################################################
// inline SSEf SSESort(const SSEf& a) {
//   SSEf b = (SSEf)_mm_shuffle_ps(a, a, 0xB1);
//   SSEf c = SSEMin(b, a);
//   b = SSEMax(b, a);
//   b = SSEReverse(b);
//   SSEf d = SSEMin(b, c);
//   b = SSEMax(b, c);
//   c = (SSEf)_mm_unpacklo_ps(d, b);
//   d = (SSEf)_mm_unpackhi_ps(d, b);
//   b = SSEMin(c, d);
//   c = SSEMax(c, d);
//   return (SSEf)_mm_unpacklo_ps(b, c);
// }
//
// //####################################################################
// //#  NaN Handling
// //####################################################################
// inline SSEf SSEIsNaN(const SSEf& a) {
//   SSEf mask(_mm_castsi128_ps(SSEi(0x7F800000)));
//   return (a & mask) == mask;
// }

//####################################################################
//#  Conversion of types
//####################################################################
inline int SSEPackChars(const SSEi& a) {
  SSEc b = (SSEc)(a & 0x000000FF);
  b |= b.rshift<3>();
  b |= b.rshift<6>();
  return SSEExtract((SSEi)b);
}
inline SSEi SSEUnpackChars(int a) {
  SSEi b = SSEi(a);
  SSEi zero = SSEZero();
  return (SSEi)_mm_unpacklo_epi16(_mm_unpacklo_epi8(b, zero), zero);
}
inline void SSECharsToFloat(const SSEc& a, SSEf& s1, SSEf& s2, SSEf& s3, SSEf& s4) {
  SSEi b = (SSEi)_mm_unpacklo_epi8(a, a);
  SSEi c = (SSEi)_mm_unpackhi_epi8(a, a);
  SSEi d = (SSEi)_mm_unpacklo_epi8(b, b);
  b = (SSEi)_mm_unpackhi_epi8(b, b);
  SSEi e = (SSEi)_mm_unpacklo_epi8(c, c);
  c = (SSEi)_mm_unpackhi_epi8(c, c);
  SSEi mask(0x000000FF);
  s1 = (SSEf)(d & mask);
  s2 = (SSEf)(b & mask);
  s3 = (SSEf)(e & mask);
  s4 = (SSEf)(c & mask);
}
inline void SSECharsToShorts(const SSEc& a, SSEs& s1, SSEs& s2) {
  SSEc zero = SSEZero();
  s1 = (SSEs)_mm_unpacklo_epi8(zero, a);
  s2 = (SSEs)_mm_unpackhi_epi8(zero, a);
}
inline SSEc SSEShortsToChars(const SSEs& a, const SSEs& b) {
  SSEc c = (SSEc)_mm_unpacklo_epi8(a, b);
  SSEc d = (SSEc)_mm_unpackhi_epi8(a, b);
  SSEc e = (SSEc)_mm_unpacklo_epi8(c, d);
  d = (SSEc)_mm_unpackhi_epi8(c, d);
  c = (SSEc)_mm_unpacklo_epi8(e, d);
  d = (SSEc)_mm_unpackhi_epi8(e, d);
  return (SSEc)_mm_unpackhi_epi8(c, d);
}

// //####################################################################
// //#  Special Summations
// //####################################################################
// //Converts a char sum into an array of short partial sums
// inline SSEs SSEShortSum(const SSEc& a) {
//   SSEc zero = SSEZero();
//   SSEs b = (SSEs)_mm_unpacklo_epi8(a, zero);
//   SSEs c = (SSEs)_mm_unpackhi_epi8(a, zero);
//   return b + c;
// }
// //Converts a char sum into an array of int partial sums
// inline SSEi SSEIntSum(const SSEc& a) {
//   SSEc zero = SSEZero();
//   SSEs b = (SSEs)_mm_unpacklo_epi8(a, zero);
//   SSEs c = (SSEs)_mm_unpackhi_epi8(a, zero);
//   b = b + c;
//   SSEi d = (SSEi)_mm_unpacklo_epi16(b, zero);
//   SSEi e = (SSEi)_mm_unpackhi_epi16(b, zero);
//   return d + e;
// }
// //Converts a short sum into an array of int partial sums
// inline SSEi SSEIntSum(const SSEs& a) {
//   SSEs zero = SSEZero();
//   SSEi b = (SSEi)_mm_unpacklo_epi16(a, zero);
//   SSEi c = (SSEi)_mm_unpackhi_epi16(a, zero);
//   return b + c;
// }
// inline void SSEIntSum(const SSEc& a, SSEi& s1, SSEi& s2, SSEi& s3, SSEi& s4) {
//   SSEc zero = SSEZero();
//   SSEs b = (SSEs)_mm_unpacklo_epi8(a, zero);
//   s1 += (SSEi)_mm_unpacklo_epi16(b, zero);
//   s2 += (SSEi)_mm_unpackhi_epi16(b, zero);
//   b = (SSEs)_mm_unpackhi_epi8(a, zero);
//   s3 += (SSEi)_mm_unpacklo_epi16(b, zero);
//   s4 += (SSEi)_mm_unpackhi_epi16(b, zero);
// }

#else
#error "Need to implement architecture-specific SSE-emulation implementation"
#define HAS_SSE 0
#endif

#endif //__SSEFoo_h__
