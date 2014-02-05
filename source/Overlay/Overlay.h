#if !defined(__Overlay_h__)
#define __Overlay_h__

#include "common.h"

#include "LeapMath.h"

#if __APPLE__
#include "Overlay/LPOverlay.h"
#elif !defined _WIN32
#endif
#include "Utility/LPVirtualScreen.h"
#include "AxisAlignedBox.h"
#include "FileSystemUtil.h"

#include <vector>

class LPIcon;
class LPImage;

namespace Touchless {
using Leap::Vector;

class OverlayDriver
{
public:
  OverlayDriver(LPVirtualScreen &virtualScreen);
  ~OverlayDriver();

  static OverlayDriver* New(LPVirtualScreen &virtualScreen);

  void destroy();

  bool initializeOverlay();

  bool deviceToScreen(const Vector& position, Vector& output, Vector& clampVec, float scale = 1, bool clamp = true);
  bool normalizedToScreen(const Vector& position, Vector& output, Vector& clampVec, float scale = 1, bool clamp = true);
  bool normalizedToAspect(const Vector& position, Vector& output, Vector& clampVec, float scale = 1, bool clamp = true);
  static float acceptableClampDistance();

  void setIconVisibility(int index, bool visible);
  int findImageIndex(float z, float touchThreshold, float touchRange) const;
  void drawImageIcon(int iconIndex, int imageIndex, float x, float y, bool visible);

  static double touchDistanceToRadius (float touchDistance);
  void drawRasterIcon(int iconIndex, float x, float y, bool visible, const Vector3& velocity, float touchDistance, double radius, float clampDistance, float alphaMult, int numFingers = 1);
  bool checkTouching(const Vector& position, float noTouchBorder) const;
  bool touchAvailable() const;

  int numTouchScreens() const;

  bool useProceduralOverlay() const;

  bool loadIfAvailable(std::shared_ptr<LPImage>& image, const std::string& fileName);

  void flushOverlay();

  LPVirtualScreen                        &m_virtualScreen;
  std::vector<std::shared_ptr<LPImage> >  m_overlayImages;
  std::vector<std::shared_ptr<LPIcon> >   m_overlayPoints;
  int                                     m_numOverlayPoints;
  int                                     m_numOverlayImages;
  int                                     m_filledImageIdx;
  int                                     m_lastNumIcons;
  bool                                    m_UseProceduralOverlay;
#if __APPLE__
  LPOverlay                               m_overlay;
#endif

};

}

#endif // __Overlay_h__
