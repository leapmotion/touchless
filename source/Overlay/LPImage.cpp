#include "stdafx.h"
#include "Overlay/LPImage.h"
#include "Overlay/LPIcon.h"
#include <memory>
#include <algorithm>

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

LPImage::LPImage(void):
  m_colors(nullptr)
{
  m_hotspot.x = 0;
  m_hotspot.y = 0;
  m_size.cx = 0;
  m_size.cy = 0;
}

LPImage::~LPImage()
{
}

void LPImage::RasterCircle(const Vector2& centerOffset, const Vector2& direction, double velocity, double outerRadius, double borderRadius, double glow, float r, float g, float b, float a)
{
  if (!m_colors)
    return;

  //Create a velocity based scale warping
  const double radiusSq = outerRadius*outerRadius;
  const double borderSq = (outerRadius-borderRadius)*(outerRadius-borderRadius);
  const double minSize = std::min(m_size.cx/2.0, m_size.cy/2.0);
  const double glowSq = std::min(radiusSq*glow*glow, minSize*minSize); // clamp so that glow doesn't exceed icon
  velocity = std::max(velocity - 50.0, 0.0);
  const double yScale = 1.0 + std::min(2.0, velocity / 900.0);
  const double xScale = std::sqrt(1.0 / yScale);

  //Find axes and center
  Vector2 aunit = velocity == 0 ? Vector2::UnitX() : direction.normalized();
  Vector2 bunit = Vector2(aunit.y(), -aunit.x());
  const Vector2 center(m_size.cx/2.0 - 1.0 + centerOffset.x(), m_size.cy/2.0 - 1.0 + centerOffset.y());

  //Calculate the drawing bounds
  const double xx = Vector2(aunit.x()/xScale, bunit.x()/yScale).norm()*outerRadius*std::abs(glow);
  const double yy = Vector2(aunit.y()/xScale, bunit.y()/yScale).norm()*outerRadius*std::abs(glow);
  const long minX = std::min(static_cast<long>(m_size.cx), std::max(0L, static_cast<long>(center.x() - xx)));
  const long minY = std::min(static_cast<long>(m_size.cy), std::max(0L, static_cast<long>(center.y() - yy)));
  const long maxX = std::min(static_cast<long>(m_size.cx), std::max(0L, static_cast<long>(center.x() + xx + 1)));
  const long maxY = std::min(static_cast<long>(m_size.cy), std::max(0L, static_cast<long>(center.y() + yy + 1)));

  //Pre-scale axes
  aunit *= xScale;
  bunit *= yScale;

  //Setup colors
  // SSEf dark;
  __m128 dark;
  if (glow > 0.0) {
    float mult = (glow > 1.0 ? 1.0f : 0.2f);
    // dark = SSEf(mult*255, mult*255, mult*255, 200);
    dark = _mm_set_ps(200, mult*255, mult*255, mult*255);
  } else {
    // dark = SSEf(b*0.2f, g*0.2f, r*0.2f, 200);
    dark = _mm_set_ps(200, r*0.2f, g*0.2f, b*0.2f);
  }
  // SSEf color(b, g, r, 255);
  // SSEf gray = SSEAvg(color, SSEf(255));
  __m128 color(_mm_set_ps(255, r, g, b));
  __m128 gray = _mm_mul_ps(_mm_add_ps(color, _mm_set1_ps(255)), _mm_set1_ps(0.5f));

  //Fill dynamic image with solid transparent color
  memset(m_colors, 0, sizeof(RGBQUAD)*m_size.cx*m_size.cy);

  //Draw the oriented ellipse
  for (long y = minY; y < maxY; y++) {
    for (long x = minX; x < maxX; x++) {
      // Calculate index of destination
      const long ix = m_size.cx*y + x;

      //Calculate normalized distances for points
      Vector2 pt = Vector2(x+0.5, y+0.5) - center;
      double fx = pt.dot(aunit);
      double fy = pt.dot(bunit);
      double distSq = fx*fx + fy*fy;

      //Check if point is inside of glow radius
      if (distSq < glowSq) {
        //Drawing parameters
        float alpha = a;
        float blend = 1.0f;

        //Check if point is inside of ellipse
        // SSEf orig = color;
        __m128 orig = color;
        if (distSq < radiusSq) {
          blend = static_cast<float>(std::max(0.0, std::min(1.0, (borderSq - distSq)/(2*outerRadius))));
          alpha *= static_cast<float>(std::min(1.0, (glowSq - distSq)/(2*outerRadius)));
          alpha *= 1.0f - blend*0.4f;
          orig = gray;
        } else {
          blend = static_cast<float>(std::max(0.0, std::min(1.0, (distSq - radiusSq)/(2*outerRadius))));
          const float temp = static_cast<float>(1.0 - (distSq - radiusSq)/(glowSq - radiusSq));
          alpha *= temp*temp*temp*0.9f;
        }

        //Apply blending to pixel
        // SSEf blended = (orig*blend + dark*(1.0f - blend))*alpha;
        __m128 blended = _mm_mul_ps(_mm_add_ps(_mm_mul_ps(orig, _mm_set1_ps(blend)), _mm_mul_ps(dark, _mm_set1_ps(1.0f - blend))), _mm_set1_ps(alpha));
        __m128i packedColor = _mm_and_si128(_mm_cvttps_epi32(blended), _mm_set1_epi32(0x000000FF));
        packedColor = _mm_or_si128(packedColor, _mm_srli_si128(packedColor, 3));
        packedColor = _mm_or_si128(packedColor, _mm_srli_si128(packedColor, 6));
        uint32_t finalColor;// = SSEPackChars(_mm_cvttps_epi32(blended));
        _mm_store_ss((float*)&finalColor, _mm_castsi128_ps(packedColor));

        //Write pixel to memory
        (uint32_t&)m_colors[ix] = finalColor;
      }
    }
  }
}

void LPImage::Clear()
{
  if (m_colors) {
    memset(m_colors, 0, sizeof(*m_colors)*m_size.cx*m_size.cy);
  }
}
