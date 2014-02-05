/*==================================================================================================================

    Copyright (c) 2010 - 2013 Leap Motion. All rights reserved.

  The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
  protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
  strictly forbidden unless prior written permission is obtained from Leap Motion.

===================================================================================================================*/

#if !defined(__LPGeometry_h__)
#define __LPGeometry_h__

typedef float LPFloat;

#if __APPLE__

#include <ApplicationServices/ApplicationServices.h>
typedef CGPoint LPPoint;
typedef CGSize LPSize;
typedef CGRect LPRect;
#define LPPoint CGPoint
#define LPSize CGSize
#define LPRect CGRect
#define LPPointZero CGPointZero
#define LPPointMake(X, Y) CGPointMake((X), (Y))
#define LPRectZero CGRectZero
#define LPRectMake(X, Y, W, H) CGRectMake((X), (Y), (W), (H))
#define LPRectGetMinX(R) CGRectGetMinX(R)
#define LPRectGetMinY(R) CGRectGetMinY(R)
#define LPRectGetMaxX(R) CGRectGetMaxX(R)
#define LPRectGetMaxY(R) CGRectGetMaxY(R)
#define LPRectContainsPoint(R, P) CGRectContainsPoint((R), (P))
#define LPRectUnion(R1, R2) CGRectUnion((R1), (R2))

#else
#include <algorithm>

struct LPPoint {
  LPPoint(LPFloat _x = 0, LPFloat _y = 0) : x(_x), y(_y) {}
  LPFloat x;
  LPFloat y;
};

struct LPSize {
  LPSize(LPFloat _width = 0, LPFloat _height = 0) : width(_width), height(_height) {}
  LPFloat width;
  LPFloat height;
};

struct LPRect {
  LPRect(LPFloat _x = 0, LPFloat _y = 0, LPFloat _width = 0, LPFloat _height = 0) : origin(_x, _y), size(_width, _height) {}
  LPPoint origin;
  LPSize size;
};

static LPPoint LPPointZero;
static inline LPPoint LPPointMake(LPFloat x, LPFloat y) { return LPPoint(x, y); }
static LPRect LPRectZero;
static inline LPRect LPRectMake(LPFloat x, LPFloat y, LPFloat width, LPFloat height) {
  return LPRect(x, y, width, height);
}

static inline LPFloat LPRectGetMinX(const LPRect& r) { return r.origin.x; }
static inline LPFloat LPRectGetMinY(const LPRect& r) { return r.origin.y; }
static inline LPFloat LPRectGetMaxX(const LPRect& r) { return (r.origin.x + r.size.width); }
static inline LPFloat LPRectGetMaxY(const LPRect& r) { return (r.origin.y + r.size.height); }

static inline bool LPRectContainsPoint(const LPRect& r, const LPPoint& p) {
  return (p.x >= r.origin.x && p.x < (r.origin.x + r.size.width) &&
          p.y >= r.origin.y && p.y < (r.origin.y + r.size.height));
}

static inline LPRect LPRectUnion(const LPRect& r1, const LPRect& r2)
{
  LPFloat x0 = std::min(r1.origin.x, r2.origin.x);
  LPFloat x1 = std::max(r1.origin.x + r1.size.width,
                        r2.origin.x + r2.size.width);
  LPFloat y0 = std::min(r1.origin.y, r2.origin.y);
  LPFloat y1 = std::max(r1.origin.y + r1.size.height,
                        r2.origin.y + r2.size.height);
  return LPRect(x0, y0, x1 - x0, y1 - y0);
}

#endif

#if !_WIN32

#pragma pack(push, 1)
typedef struct tagRGBQUAD {
  unsigned char    rgbBlue;
  unsigned char    rgbGreen;
  unsigned char    rgbRed;
  unsigned char    rgbReserved;
} RGBQUAD;
#pragma pack(pop)

typedef struct _RECT {
  int32_t left;
  int32_t right;
  int32_t top;
  int32_t bottom;
} RECT, *PRECT;

typedef struct _POINT {
  int32_t x;
  int32_t y;
} POINT, *PPOINT;

typedef struct _SIZE {
  int32_t cx;
  int32_t cy;
} SIZE, *PSIZE;

#endif

#endif // __LPGeometry_h__
