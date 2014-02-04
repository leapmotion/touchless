// #include "stdafx.h"
#include "Overlay.h"
#include "Overlay/LPIcon.h"
#include "Overlay/LPImage.h"
#include "Utility/LPVirtualScreen.h"
#include EXCEPTION_PTR_HEADER
#include <fstream>


namespace Touchless
{

OverlayDriver::OverlayDriver(LPVirtualScreen &virtualScreen)
  : m_virtualScreen(virtualScreen),
  m_UseProceduralOverlay(true),
  m_numOverlayPoints(0),
  m_numOverlayImages(0),
  m_filledImageIdx(0)
{}

OverlayDriver::~OverlayDriver() {}

OverlayDriver* OverlayDriver::New(LPVirtualScreen &virtualScreen)
{
  return new OverlayDriver(virtualScreen);
}

bool OverlayDriver::initializeOverlay()
{
  m_numOverlayPoints = 32;
  std::string folder = "circle_icons_64";
  std::string prefix = "circle_";
  std::string suffix = "_64.png";

  if (m_UseProceduralOverlay) {
    m_numOverlayImages = m_numOverlayPoints;
    m_filledImageIdx = 0;
  } else {
    m_numOverlayImages = 32;
    m_filledImageIdx = 24;
    std::ifstream fileInput("icons.txt");
    if (fileInput.good()) {
      fileInput >> folder;
      fileInput >> prefix;
      fileInput >> suffix;
      fileInput >> m_numOverlayImages;
      fileInput >> m_filledImageIdx;
      m_filledImageIdx--;
    }
  }

  for (int i=0; i<m_numOverlayPoints; i++) {
    m_overlayPoints.push_back(std::shared_ptr<LPIcon>(new LPIcon()));
  }
  for (int i=0; i<m_numOverlayImages; i++) {
    m_overlayImages.push_back(std::shared_ptr<LPImage>(new LPImage()));
  }
  if (m_filledImageIdx >= m_numOverlayImages) {
    m_filledImageIdx = (m_numOverlayImages > 0) ? m_numOverlayImages - 1 : 0;
  }

  if (m_UseProceduralOverlay) {
    SIZE iconSize;
    iconSize.cx = 100;
    iconSize.cy = 100;
    for (int i=0; i<m_numOverlayImages; i++) {
      m_overlayImages[i]->SetImage(iconSize);
    }
  } else {
    std::stringstream ss;
    for (int i=0; i<m_numOverlayImages; i++) {
      ss.str("");
      ss << folder << "/" << prefix << (i+1) << suffix;
      loadIfAvailable(m_overlayImages[i], ss.str());
    }
  }

  return true;
}

bool OverlayDriver::deviceToScreen(const Vector& position, Vector& output, Vector& clampVec, float scale, bool clamp)
{
  static Vector interactionCenter(0.0f, 200.0f, 0.0f);
  static Vector interactionSize(200.0f, 200.0f, 200.0f);

  const float& x = position.x;
  const float& y = position.y;
  const float& z = position.z;

  float fx = ((x-interactionCenter.x) + (interactionSize.x/2))/interactionSize.x;
  float fy = ((y-interactionCenter.y) + (interactionSize.y/2))/interactionSize.y;
  float fz = ((z-interactionCenter.z) + (interactionSize.z/2))/interactionSize.z;

  return normalizedToScreen(Vector(fx, fy, fz), output, clampVec, scale, clamp);
}

bool OverlayDriver::normalizedToScreen(const Vector& position, Vector& output, Vector& clampVec, float scale, bool clamp)
{
  bool isOkay = normalizedToAspect(position, output, clampVec, scale, clamp);

  LPPoint pos = LPPointMake(static_cast<LPFloat>(output.x), static_cast<LPFloat>(output.y));
  pos = m_virtualScreen.Denormalize(pos);
  if (clamp) {
    pos = m_virtualScreen.ClipPosition(pos);
  }
  output.x = static_cast<float>(pos.x);
  output.y = static_cast<float>(pos.y);

  return isOkay;
}

float OverlayDriver::acceptableClampDistance() const
{
  return 0.6f;
}

bool OverlayDriver::normalizedToAspect(const Vector& position, Vector& output, Vector& clampVec, float scale, bool clamp)
{
  bool isOkay = true;
  output = position;

  output.y = (output.y - 0.5f) * m_virtualScreen.AspectRatio() + 0.5f;

  // adjust each coordinate by the scale factor
  output.x = (scale * (output.x-0.5f)) + 0.5f;
  output.y = (scale * (output.y-0.5f)) + 0.5f;
  output.z = (scale * (output.z-0.5f)) + 0.5f;

  // need to flip y axis
  output.y = (1 - output.y);

  static const AxisAlignedBox<3> unitBox(Vector3::Zero(), Vector3::Ones());
  Vector3 temp = unitBox.ClosestPoint(output.toVector3<Vector3>());
  Vector clampedPos(static_cast<float>(temp.x()), static_cast<float>(temp.y()), output.z);
  clampVec = clampedPos - output;
  float clampDist = clampVec.magnitude();
  isOkay = (clampDist <= acceptableClampDistance());

  if (clamp) {
    output = clampedPos;
  }

  return isOkay;
}

double OverlayDriver::touchDistanceToRadius (float touchDistance)
{
  return 40.0*std::min(std::max(touchDistance, 0.0f), 0.7f) + 10.0;
}

void OverlayDriver::drawRasterIcon(int iconIndex, float x, float y, bool visible, const Vector3& velocity, float touchDistance, double radius, float clampDistance, float alphaMult, int numFingers)
{
  if (iconIndex < 0 || iconIndex >= m_numOverlayPoints) {
    return;
  }
  if (visible) {
    Vector2 velXY(velocity.x(), velocity.y());
    double velNorm = velXY.norm();
    float r = 255;
    float g = 255;
    float b = 255;
    float a = std::min(1.0f - touchDistance*touchDistance, 1.0f);
    double borderRadius = std::min(radius, 2.5 + std::max(0.0, 15.0 - radius));
    double glow = std::max(-touchDistance*2.0, 0.0) + (touchDistance < 0.0 ? 3.0 : 1.0);
    if (touchDistance < 0.0f) {
      r = 100;
      g = 225;
      b = 100;
      borderRadius = 2;
      //radius = 10.0;
    }
    a *= (1.0f - std::min(1.0f, (clampDistance / acceptableClampDistance())));
    a *= alphaMult;
    Vector2 centerOffset(std::fmod(x, 1.0f), 1.0f-std::fmod(y, 1.0f));
    LPPoint position = LPPointMake(static_cast<LPFloat>(x), static_cast<LPFloat>(y));
    m_overlayImages[iconIndex]->RasterCircle(centerOffset, velXY, velNorm, radius, borderRadius, glow, r, g, b, a);
    m_overlayPoints[iconIndex]->SetImage(m_overlayImages[iconIndex], false);
    m_overlayPoints[iconIndex]->SetPosition(position);
  }
  setIconVisibility(iconIndex, visible);
}

bool OverlayDriver::useProceduralOverlay() const
{
  return m_UseProceduralOverlay;
}

bool OverlayDriver::loadIfAvailable(std::shared_ptr<LPImage>& image, const std::string& fileName)
{
  if (image && FileSystemUtil::FileExists(fileName)) {
    std::wstring wide(fileName.begin(), fileName.end());
    return image->SetImage(wide);
  }
  return false;
}

void OverlayDriver::setIconVisibility(int index, bool visible)
{
  if (index < 0 || index >= m_numOverlayPoints) {
    return;
  }
  m_overlayPoints[index]->SetVisibility(visible);
  m_overlayPoints[index]->Update();
#if __APPLE__
  if (visible) {
    m_overlay.AddIcon(m_overlayPoints[index]);
  } else {
    m_overlay.RemoveIcon(m_overlayPoints[index]);
  }
#endif
}

int OverlayDriver::findImageIndex(float z, float touchThreshold, float touchRange) const
{
  if (m_numOverlayImages > 0) {
    float min = touchThreshold - touchRange;
    float middle = touchThreshold;
    float max = touchThreshold + touchRange;
    if (z < min) {
      return m_numOverlayImages - 1;
    } else if (z >= min && z < middle) {
      float amt = 1 - ((z - min) / (middle - min));
      return m_filledImageIdx + static_cast<int>((m_numOverlayImages - m_filledImageIdx - 1) * amt);
    } else if (z >= middle && z < max) {
      float amt = 1 - ((z - middle) / (max - middle));
      return static_cast<int>(m_filledImageIdx * amt);
    }
  }
  return 0;
}

void OverlayDriver::drawImageIcon(int iconIndex, int imageIndex, float x, float y, bool visible)
{
  if (iconIndex < 0 || iconIndex >= m_numOverlayPoints || imageIndex < 0 || imageIndex >= m_numOverlayImages) {
    return;
  }
  LPPoint position = LPPointMake(static_cast<LPFloat>(x), static_cast<LPFloat>(y));
  m_overlayPoints[iconIndex]->SetImage(m_overlayImages[imageIndex], false);
  m_overlayPoints[iconIndex]->SetPosition(position);
  setIconVisibility(iconIndex, visible);
}


#if __APPLE__
void OverlayDriver::flushOverlay()
{
  m_overlay.Flush();
}
#endif

}
