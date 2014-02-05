#if !defined(__LPImage_h__)
#define __LPImage_h__
#include "common.h"
#include "Utility/LPGeometry.h"
#include <set>

class LPIcon;

typedef struct tagRGBQUAD RGBQUAD;

class LPImage {
public:
  LPImage();
  virtual ~LPImage();

  static LPImage* New(void);

  void RasterCircle(const Vector2& centerOffset, const Vector2& direction, double velocity, double outerRadius, double borderRadius, double glow, float r, float g, float b, float a);
  void Clear();

  /// <summary>
  /// Creates an image from the passed file
  /// </summary>
  virtual bool SetImage(const std::wstring& filename, const POINT* pHotspot = 0) = 0;

  /// <summary>
  /// Creates the image from a given width and height
  /// </summary>
  virtual bool SetImage(const SIZE& size, const POINT* pHotspot = 0) = 0;

  const POINT& GetHotspot() const { return m_hotspot; }
  uint32_t* GetInternalImage() const { return (uint32_t*)m_colors; }

protected:
  RGBQUAD* m_colors;
  POINT m_hotspot;
  SIZE m_size;

  /// <summary>
  /// Constructs an image
  /// </summary>
  /// <param name="width">The width of the passed image</param>
  /// <param name="height">The height of the passed image</param>
  /// <param name="pHotspot">The hot spot for the image.  If left null, it will default to the geometric center.</param>
  virtual bool InitImage(long width, long height, const POINT* pHotspot = 0) = 0;

  friend class LPIcon;

public:
  // Accessor methods:
  int32_t GetWidth() const { return static_cast<int32_t>(m_size.cx); }
  int32_t GetHeight() const { return static_cast<int32_t>(m_size.cy); }

  /// <summary>
  /// Verifies that the size and pixel data
  /// </summary>
  bool operator==(const LPImage& rhs) const {
    return
      m_size.cx == rhs.m_size.cx &&
      m_size.cy == rhs.m_size.cy &&
      !memcmp(m_colors, rhs.m_colors, m_size.cx * m_size.cy * sizeof(*m_colors));
  }

  /// <summary>
  /// Convenience overload
  /// </summary>
  bool operator!=(const LPImage& rhs) const {
    return !(*this == rhs);
  }
};

#endif // __LPImage_h__
