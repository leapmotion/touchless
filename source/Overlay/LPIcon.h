#if !defined(__LPIcon_h__)
#define __LPIcon_h__
#include "C++11/cpp11.h"
#include "Utility/LPGeometry.h"
#include SHARED_PTR_HEADER

class LPImage;

class LPIcon {
public:
  LPIcon();
  virtual ~LPIcon();

  static LPIcon* New(void);

  void SetImage(const std::shared_ptr<LPImage>& image = std::shared_ptr<LPImage>(), bool update = true);
  virtual void SetPosition(const LPPoint& position) = 0;
  virtual void SetVisibility(bool isVisible) = 0;
  virtual bool Update(void) = 0;

  const std::shared_ptr<LPImage>& GetImage() const { return m_image; }
  virtual LPPoint GetPosition() const = 0;
  virtual bool GetVisibility() const = 0;

protected:
  std::shared_ptr<LPImage> m_image;
};

#endif // __LPIcon_h__
