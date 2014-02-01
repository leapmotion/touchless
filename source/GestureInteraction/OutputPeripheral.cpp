#include "stdafx.h"
#include "OutputPeripheralImplementation.h"
#if _WIN32
#include "FocusAppInfo.h"
#endif
#include "TouchPeripheral.h"
#include "API/SharedObject.h"
#include "DataStructures/AxisAlignedBox.h"
#include "DataStructures/Value.h"
#include "Peripherals/LPIcon.h"
#include "Peripherals/LPImage.h"
#include "Peripherals/LPVirtualScreen.h"
#include "Peripherals/TouchManager.h"
#include "Utility/FileSystemUtil.h"
#include EXCEPTION_PTR_HEADER
#include <fstream>


namespace Touchless
{

#ifdef _WIN32
void windowsKeyDown(INPUT& input, WORD key) {
  input.type = INPUT_KEYBOARD;
  input.ki.wVk = key;
  input.ki.dwFlags = 0;
}
void windowsKeyUp(INPUT& input, WORD key) {
  input.type = INPUT_KEYBOARD;
  input.ki.wVk = key;
  input.ki.dwFlags = KEYEVENTF_KEYUP;
}
void windowsKeyCombo(WORD key1) {
  INPUT input[2] = {0};
  windowsKeyDown(input[0], key1);
  windowsKeyUp(input[1], key1);
  SendInput(2,input,sizeof(INPUT));
}
void windowsKeyCombo(WORD key1, WORD key2) {
  INPUT input[4] = {0};
  windowsKeyDown(input[0], key1);
  windowsKeyDown(input[1], key2);
  windowsKeyUp(input[2], key2);
  windowsKeyUp(input[3], key1);
  SendInput(4,input,sizeof(INPUT));
}
void windowsKeyCombo(WORD key1, WORD key2, WORD key3) {
  INPUT input[6] = {0};
  windowsKeyDown(input[0], key1);
  windowsKeyDown(input[1], key2);
  windowsKeyDown(input[2], key3);
  windowsKeyUp(input[3], key3);
  windowsKeyUp(input[4], key2);
  windowsKeyUp(input[5], key1);
  SendInput(6,input,sizeof(INPUT));
}
#endif


OSInteractionDriver::OSInteractionDriver(LPVirtualScreen &virtualScreen)
  : m_virtualScreen(virtualScreen) {}

OSInteractionDriver::~OSInteractionDriver() {}

OSInteractionDriver* OSInteractionDriver::New(LPVirtualScreen &virtualScreen) {return new OSInteractionDriver(virtualScreen);}

bool OSInteractionDriver::initializeTouch() {return true;}

void OSInteractionDriver::useDefaultScreen(bool use)
{
  m_virtualScreen.UseDefaultScreen(use);
}


void OSInteractionDriver::clickDown(int button, int number)
{
#if __APPLE__
  int clickNum = (number < 1) ? 1 : ((number > 3) ? 3 : number);
  const CGPoint& position = m_virtualScreen.Position();
  CGEventRef eventRef = CGEventCreateMouseEvent(0, kCGEventLeftMouseDown, position, kCGMouseButtonLeft);
  CGEventSetIntegerValueField(eventRef, kCGMouseEventClickState, clickNum);
  CGEventPost(kCGHIDEventTap, eventRef);
  CFRelease(eventRef);
  m_buttonDown = true;
#elif _WIN32
  INPUT input = { 0 };
  input.type = INPUT_MOUSE;
  input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
  SendInput(1, &input, sizeof(INPUT));
#else
  throw_rethrowable std::logic_error("OSInteractionDriver not implemented"); // Linux -- FIXME
#endif
  if (button >= 0 && button < NUM_BUTTONS) {
    m_clickedButtons[button] = true;
  }
}

void OSInteractionDriver::clickUp(int button, int number)
{
  if (button >= 0 && button < NUM_BUTTONS) {
    m_clickedButtons[button] = false;
  }
#if __APPLE__
  int clickNum = (number < 1) ? 1 : ((number > 3) ? 3 : number);
  const CGPoint& position = m_virtualScreen.Position();
  CGEventRef eventRef = CGEventCreateMouseEvent(0, kCGEventLeftMouseUp, position, kCGMouseButtonLeft);
  CGEventSetIntegerValueField(eventRef, kCGMouseEventClickState, clickNum);
  CGEventPost(kCGHIDEventTap, eventRef);
  CFRelease(eventRef);
  m_buttonDown = false;
#elif _WIN32
  INPUT input = { 0 };
  input.type = INPUT_MOUSE;
  input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
  SendInput(1, &input, sizeof(INPUT));
#else
  throw_rethrowable std::logic_error("OSInteractionDriver not implemented"); // Linux -- FIXME
#endif
}

bool OSInteractionDriver::isClickedDown(int button) const
{
  if (button >= 0 && button < NUM_BUTTONS) {
    return m_clickedButtons[button];
  }
  return false;
}

void OSInteractionDriver::keyDown(int code)
{
  emitKeyboardEvent(code, true);
}

void OSInteractionDriver::keyUp(int code)
{
  emitKeyboardEvent(code, false);
}

bool OSInteractionDriver::cursorPosition(float* fx, float* fy) const
{
#if __APPLE__
  CGEventRef event = CGEventCreate(0);
  CGPoint cursor = CGEventGetLocation(event);
  if (fx) {
    *fx = static_cast<float>(cursor.x);
  }
  if (fy) {
    *fy = static_cast<float>(cursor.y);
  }
  CFRelease(event);
  return true;
#elif _WIN32
  POINT cursor = { 0, 0 };

  if (GetCursorPos(&cursor)) {
    if (fx) { *fx = static_cast<float>(cursor.x); }
    if (fy) { *fy = static_cast<float>(cursor.y); }
    return true;
  }
  return false;
#else
  throw_rethrowable std::logic_error("OutputPeripheral not implemented"); // Linux -- FIXME
#endif
}

void OSInteractionDriver::setCursorPosition(float fx, float fy, bool absolute)
{
#if defined(__APPLE__) || defined(_WIN32)
  LPPoint position = LPPointMake(static_cast<LPFloat>(fx), static_cast<LPFloat>(fy));
#endif

#if __APPLE__
  CGEventRef eventRef;
  if (!absolute) {
    eventRef = CGEventCreate(0);
    CGPoint cursor = CGEventGetLocation(eventRef);
    position.x += cursor.x;
    position.y += cursor.y;
    CFRelease(eventRef);
  }
  position = m_virtualScreen.SetPosition(position);
  eventRef = CGEventCreateMouseEvent(0, m_buttonDown ? kCGEventLeftMouseDragged : kCGEventMouseMoved,
                                     position, kCGMouseButtonLeft);
  CGEventPost(kCGHIDEventTap, eventRef);
  CFRelease(eventRef);
#elif _WIN32
  INPUT input = { 0 };
  input.type = INPUT_MOUSE;
  input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK;

  if (!absolute) {
    POINT cursor = { 0, 0 };

    if (GetCursorPos(&cursor)) {
      position.x += static_cast<LPFloat>(cursor.x);
      position.y += static_cast<LPFloat>(cursor.y);
    }
  } else {
    position.x += 0.5f;
    position.y += 0.5f;
  }
  position = m_virtualScreen.SetPosition(position);
  position = m_virtualScreen.Normalize(position, false);

  // When specifying absolute coordinates, they must be mapped such that max val = 65535.

  input.mi.dx = (int)(65535.0f*static_cast<float>(position.x) + 0.5f);
  input.mi.dy = (int)(65535.0f*static_cast<float>(position.y) + 0.5f);

  SendInput(1, &input, sizeof(INPUT));
#else
  throw_rethrowable std::logic_error("OutputPeripheral not implemented"); // Linux -- FIXME
#endif
}

void OSInteractionDriver::cancelGestureEvents()
{
  m_movingCursor = false;

#if _WIN32
  boost::unique_lock<boost::mutex> lock(m_touchMutex);
  m_touchManager->clearTouches();
#endif
  endGesture();
}

uint32_t OSInteractionDriver::gestureType() const
{
  return m_gesture.type();
}

bool OSInteractionDriver::beginGesture(uint32_t gestureType)
{
  syncPosition();
  return m_gesture.begin(gestureType);
}

bool OSInteractionDriver::endGesture()
{
  syncPosition();
  return m_gesture.end();
}

bool OSInteractionDriver::applyZoom(float zoom)
{
  syncPosition();
  return m_gesture.applyZoom(zoom - 1.0f);
}

bool OSInteractionDriver::applyRotation(float rotation)
{
  syncPosition();
  return m_gesture.applyRotation(rotation);
}

bool OSInteractionDriver::applyScroll(float dx, float dy, int64_t timeDiff)
{
  syncPosition();
  return m_gesture.applyScroll(dx, dy, timeDiff);
}

bool OSInteractionDriver::applyDesktopSwipe(float dx, float dy)
{
  syncPosition();
  return m_gesture.applyDesktopSwipe (dx, dy);
}

void OSInteractionDriver::applyCharms(const Leap::Vector& aspectNormalized, int numPointablesActive, int& charmsMode)
{
#if _WIN32
  if (charmsMode == 0 && numPointablesActive == 1) {
    if (aspectNormalized.x >= 1.07f) {
      //Send a "Win + C" to bring out or hide the charm bar
      windowsKeyCombo(VK_LWIN, VkKeyScan('c'));
      charmsMode = 1;
    } else if (aspectNormalized.x <= -0.07f) {
      //Send out a "Win + Ctrl + Tab" to bring out the switchable programs
      windowsKeyCombo(VK_LWIN, VK_CONTROL, VK_TAB);
      charmsMode = 2;
    } else if (aspectNormalized.y <= -0.07f) {
      //Send a "Win + Z" to bring out or hide the app commands
      windowsKeyCombo(VK_LWIN, VkKeyScan('z'));
      charmsMode = 3;
    }
  } else if (aspectNormalized.x >= 0.05f && aspectNormalized.x <= 0.95f &&
             aspectNormalized.y >= 0.05f && aspectNormalized.y <= 0.95f) {
    charmsMode = 0;
  }
#endif
}

bool OSInteractionDriver::checkTouching(const Vector& position, float noTouchBorder) const
{
  return false;
}

void OSInteractionDriver::emitTouchEvent(const TouchEvent& evt)
{
#if _WIN32
  // Translate and convert:
  std::set<Touch> unordered;

  for(auto q = evt.begin(); q != evt.end(); q++) {
    const Touch& cur = *q;

    // Clip to our screen:
    auto position = m_virtualScreen.ClipPosition(LPPoint((LPFloat)cur.x(), (LPFloat)cur.y()));

    // Insert into the new map:
    unordered.insert(Touch(
                           cur.id(),
                           position.x,
                           position.y,
                           cur.touching()
                           ));
  }
  m_touchManager->setTouches(std::move(unordered));
#endif
}

bool OSInteractionDriver::touchAvailable() const
{
#if _WIN32
  return true;
#else
  return false;
#endif
}


int OSInteractionDriver::numTouchScreens() const
{
#if _WIN32
  return m_touchManager->numTouchScreens();
#else
  return 0;
#endif
}

bool OSInteractionDriver::useCharmHelper() const
{
  return m_useCharmHelper;
}

void OSInteractionDriver::emitKeyboardEvent(int key, bool down)
{
#if __APPLE__
  CGEventRef eventRef = CGEventCreateKeyboardEvent(0, LPKeyboard::GetKeyCode(key), down);
  CGEventPost(kCGHIDEventTap, eventRef);
  CFRelease(eventRef);
#elif _WIN32
  INPUT input = { 0 };
  KEYBDINPUT kb = { 0 };
  kb.dwFlags = KEYEVENTF_EXTENDEDKEY;
  if (!down) {
    kb.dwFlags |= KEYEVENTF_KEYUP;
  }
  kb.wVk = key;
  input.type = INPUT_KEYBOARD;
  input.ki = kb;
  SendInput(1, &input, sizeof(INPUT));
#else
  throw_rethrowable std::logic_error("OutputPeripheral not implemented"); // Linux -- FIXME
#endif
}

void OSInteractionDriver::emitKeyboardEvents(int* keys, int numKeys, bool down)
{

if (numKeys <= 0) {
    return;
  }
#if __APPLE__

#elif _WIN32
  INPUT* inputs = new INPUT[numKeys];
  for (int i=0; i<numKeys; i++) {
    KEYBDINPUT kb = { 0 };
    kb.dwFlags = KEYEVENTF_EXTENDEDKEY;
    if (!down) {
      kb.dwFlags |= KEYEVENTF_KEYUP;
    }
    kb.wVk = keys[i];
    inputs[i].type = INPUT_KEYBOARD;
    inputs[i].ki = kb;
  }
  SendInput(static_cast<UINT>(numKeys), inputs, sizeof(INPUT));
  delete[] inputs;
#else
  throw_rethrowable std::logic_error("OutputPeripheral not implemented"); // Linux -- FIXME
#endif
}


void OSInteractionDriver::syncPosition()
{
  LPPoint position;
#if __APPLE__
  CGEventRef eventRef = CGEventCreate(0);
  position = CGEventGetLocation(eventRef);
  CFRelease(eventRef);
#elif _WIN32
  POINT cursor = { 0, 0 };

  if (GetCursorPos(&cursor)) {
    position.x = static_cast<LPFloat>(cursor.x);
    position.y = static_cast<LPFloat>(cursor.y);
  }
#else
  throw_rethrowable std::logic_error("OutputPeripheralImplementation not implemented"); // Linux -- FIXME
#endif
  position = m_virtualScreen.SetPosition(position);
  m_gesture.setPosition(position.x, position.y);
}





OverlayDriver::OverlayDriver(LPVirtualScreen &virtualScreen)
  : m_virtualScreen(virtualScreen) {}
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


using namespace Leap;

//
// OutputPeripheralImplementation
//

OutputPeripheralImplementation::OutputPeripheralImplementation() :
  m_movingCursor(false),
  m_UseProceduralOverlay(true),
  m_outputPeripheralMode(nullptr),
  m_numOverlayPoints(0),
  m_numOverlayImages(0),
  m_filledImageIdx(0),
  m_useCharmHelper(true),
  m_touchManager(TouchManager::New())
{
  m_outputMode = OutputPeripheral::OUTPUT_MODE_DISABLED;
  m_lastOutputMode = OutputPeripheral::OUTPUT_MODE_DISABLED;

#if __APPLE__
  m_applicationInfoCache.appCode = -1;
  m_buttonDown = false;
#endif
  for (int button = 0; button < NUM_BUTTONS; button++) {
    m_clickedButtons[button] = false;
  }
}

OutputPeripheralImplementation::~OutputPeripheralImplementation()
{
  for (int i = 0; i < m_numOverlayImages; ++i) {
    setIconVisibility(i, false);
  }
  delete m_touchManager;
}

void OutputPeripheralImplementation::destroy() {
  delete m_outputPeripheralMode;
  m_outputPeripheralMode = nullptr;
}

bool OutputPeripheralImplementation::initializeTouchAndOverlay() {
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

void OutputPeripheralImplementation::useDefaultScreen(bool use)
{
  m_virtualScreen.UseDefaultScreen(use);
}

bool OutputPeripheralImplementation::usingDefaultScreen() const {
  return m_virtualScreen.UsingDefaultScreen();
}

void OutputPeripheralImplementation::clickDown(int button, int number)
{
#if __APPLE__
  int clickNum = (number < 1) ? 1 : ((number > 3) ? 3 : number);
  const CGPoint& position = m_virtualScreen.Position();
  CGEventRef eventRef = CGEventCreateMouseEvent(0, kCGEventLeftMouseDown, position, kCGMouseButtonLeft);
  CGEventSetIntegerValueField(eventRef, kCGMouseEventClickState, clickNum);
  CGEventPost(kCGHIDEventTap, eventRef);
  CFRelease(eventRef);
  m_buttonDown = true;
#elif _WIN32
  INPUT input = { 0 };
  input.type = INPUT_MOUSE;
  input.mi.dwFlags = MOUSEEVENTF_LEFTDOWN;
  SendInput(1, &input, sizeof(INPUT));
#else
  throw_rethrowable std::logic_error("OutputPeripheral not implemented"); // Linux -- FIXME
#endif
  if (button >= 0 && button < NUM_BUTTONS) {
    m_clickedButtons[button] = true;
  }
}

void OutputPeripheralImplementation::clickUp(int button, int number)
{
  if (button >= 0 && button < NUM_BUTTONS) {
    m_clickedButtons[button] = false;
  }
#if __APPLE__
  int clickNum = (number < 1) ? 1 : ((number > 3) ? 3 : number);
  const CGPoint& position = m_virtualScreen.Position();
  CGEventRef eventRef = CGEventCreateMouseEvent(0, kCGEventLeftMouseUp, position, kCGMouseButtonLeft);
  CGEventSetIntegerValueField(eventRef, kCGMouseEventClickState, clickNum);
  CGEventPost(kCGHIDEventTap, eventRef);
  CFRelease(eventRef);
  m_buttonDown = false;
#elif _WIN32
  INPUT input = { 0 };
  input.type = INPUT_MOUSE;
  input.mi.dwFlags = MOUSEEVENTF_LEFTUP;
  SendInput(1, &input, sizeof(INPUT));
#else
  throw_rethrowable std::logic_error("OutputPeripheral not implemented"); // Linux -- FIXME
#endif
}

void OutputPeripheralImplementation::keyDown(int code)
{
  emitKeyboardEvent(code, true);
}

void OutputPeripheralImplementation::keyUp(int code)
{
  emitKeyboardEvent(code, false);
}

void OutputPeripheralImplementation::emitKeyboardEvent(int key, bool down)
{
#if __APPLE__
  CGEventRef eventRef = CGEventCreateKeyboardEvent(0, LPKeyboard::GetKeyCode(key), down);
  CGEventPost(kCGHIDEventTap, eventRef);
  CFRelease(eventRef);
#elif _WIN32
  INPUT input = { 0 };
  KEYBDINPUT kb = { 0 };
  kb.dwFlags = KEYEVENTF_EXTENDEDKEY;
  if (!down) {
    kb.dwFlags |= KEYEVENTF_KEYUP;
  }
  kb.wVk = key;
  input.type = INPUT_KEYBOARD;
  input.ki = kb;
  SendInput(1, &input, sizeof(INPUT));
#else
  throw_rethrowable std::logic_error("OutputPeripheral not implemented"); // Linux -- FIXME
#endif
}

void OutputPeripheralImplementation::emitKeyboardEvents(int* keys, int numKeys, bool down) {
  if (numKeys <= 0) {
    return;
  }
#if __APPLE__

#elif _WIN32
  INPUT* inputs = new INPUT[numKeys];
  for (int i=0; i<numKeys; i++) {
    KEYBDINPUT kb = { 0 };
    kb.dwFlags = KEYEVENTF_EXTENDEDKEY;
    if (!down) {
      kb.dwFlags |= KEYEVENTF_KEYUP;
    }
    kb.wVk = keys[i];
    inputs[i].type = INPUT_KEYBOARD;
    inputs[i].ki = kb;
  }
  SendInput(static_cast<UINT>(numKeys), inputs, sizeof(INPUT));
  delete[] inputs;
#else
  throw_rethrowable std::logic_error("OutputPeripheral not implemented"); // Linux -- FIXME
#endif
}

bool OutputPeripheralImplementation::isClickedDown(int button) const
{
  if (button >= 0 && button < NUM_BUTTONS) {
    return m_clickedButtons[button];
  }
  return false;
}

bool OutputPeripheralImplementation::deviceToScreen(const Vector& position, Vector& output, Vector& clampVec, float scale, bool clamp)
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

bool OutputPeripheralImplementation::normalizedToAspect(const Vector& position, Vector& out, Vector& clampVec, float scale, bool clamp) {
  bool isOkay = true;
  out = position;

  out.y = (out.y - 0.5f) * m_virtualScreen.AspectRatio() + 0.5f;

  // adjust each coordinate by the scale factor
  out.x = (scale * (out.x-0.5f)) + 0.5f;
  out.y = (scale * (out.y-0.5f)) + 0.5f;
  out.z = (scale * (out.z-0.5f)) + 0.5f;

  // need to flip y axis
  out.y = (1 - out.y);

  static const AxisAlignedBox<3> unitBox(Vector3::Zero(), Vector3::Ones());
  Vector3 temp = unitBox.ClosestPoint(out.toVector3<Vector3>());
  Vector clampedPos(static_cast<float>(temp.x()), static_cast<float>(temp.y()), out.z);
  clampVec = clampedPos - out;
  float clampDist = clampVec.magnitude();
  isOkay = (clampDist <= acceptableClampDistance());

  if (clamp) {
    out = clampedPos;
  }

  return isOkay;
}

bool OutputPeripheralImplementation::normalizedToScreen(const Vector& position, Vector& out, Vector& clampVec, float scale, bool clamp) {
  bool isOkay = normalizedToAspect(position, out, clampVec, scale, clamp);

  LPPoint pos = LPPointMake(static_cast<LPFloat>(out.x), static_cast<LPFloat>(out.y));
  pos = m_virtualScreen.Denormalize(pos);
  if (clamp) {
    pos = m_virtualScreen.ClipPosition(pos);
  }
  out.x = static_cast<float>(pos.x);
  out.y = static_cast<float>(pos.y);

  return isOkay;
}

void OutputPeripheralImplementation::setCursorPosition(float fx, float fy, bool absolute)
{
#if defined(__APPLE__) || defined(_WIN32)
  LPPoint position = LPPointMake(static_cast<LPFloat>(fx), static_cast<LPFloat>(fy));
#endif

#if __APPLE__
  CGEventRef eventRef;
  if (!absolute) {
    eventRef = CGEventCreate(0);
    CGPoint cursor = CGEventGetLocation(eventRef);
    position.x += cursor.x;
    position.y += cursor.y;
    CFRelease(eventRef);
  }
  position = m_virtualScreen.SetPosition(position);
  eventRef = CGEventCreateMouseEvent(0, m_buttonDown ? kCGEventLeftMouseDragged : kCGEventMouseMoved,
                                     position, kCGMouseButtonLeft);
  CGEventPost(kCGHIDEventTap, eventRef);
  CFRelease(eventRef);
#elif _WIN32
  INPUT input = { 0 };
  input.type = INPUT_MOUSE;
  input.mi.dwFlags = MOUSEEVENTF_MOVE | MOUSEEVENTF_ABSOLUTE | MOUSEEVENTF_VIRTUALDESK;

  if (!absolute) {
    POINT cursor = { 0, 0 };

    if (GetCursorPos(&cursor)) {
      position.x += static_cast<LPFloat>(cursor.x);
      position.y += static_cast<LPFloat>(cursor.y);
    }
  } else {
    position.x += 0.5f;
    position.y += 0.5f;
  }
  position = m_virtualScreen.SetPosition(position);
  position = m_virtualScreen.Normalize(position, false);

  // When specifying absolute coordinates, they must be mapped such that max val = 65535.

  input.mi.dx = (int)(65535.0f*static_cast<float>(position.x) + 0.5f);
  input.mi.dy = (int)(65535.0f*static_cast<float>(position.y) + 0.5f);

  SendInput(1, &input, sizeof(INPUT));
#else
  throw_rethrowable std::logic_error("OutputPeripheral not implemented"); // Linux -- FIXME
#endif
}

bool OutputPeripheralImplementation::cursorPosition(float* fx, float* fy) const
{
#if __APPLE__
  CGEventRef event = CGEventCreate(0);
  CGPoint cursor = CGEventGetLocation(event);
  if (fx) {
    *fx = static_cast<float>(cursor.x);
  }
  if (fy) {
    *fy = static_cast<float>(cursor.y);
  }
  CFRelease(event);
  return true;
#elif _WIN32
  POINT cursor = { 0, 0 };

  if (GetCursorPos(&cursor)) {
    if (fx) { *fx = static_cast<float>(cursor.x); }
    if (fy) { *fy = static_cast<float>(cursor.y); }
    return true;
  }
  return false;
#else
  throw_rethrowable std::logic_error("OutputPeripheral not implemented"); // Linux -- FIXME
#endif
}

void OutputPeripheralImplementation::registerApplication(const std::string& windowTitle, const std::string& exeName, int appCode)
{
  std::wstring windowTitleWide = Value(windowTitle).To<std::wstring>();
  std::wstring exeNameWide = Value(exeName).To<std::wstring>();

  registerApplication(windowTitleWide, exeNameWide, appCode);
}

void OutputPeripheralImplementation::registerApplication(const std::wstring& windowTitle, const std::wstring& exeName, int appCode)
{
  AppInfo temp;
  temp.windowTitle = windowTitle;
  temp.exeName = exeName;
  temp.appCode = appCode;
  if (m_applicationInfos.empty()) {
    m_applicationInfoCache = temp; // Cache the first entry as the "current" application
  }
  m_applicationInfos.push_back(temp);
}

int OutputPeripheralImplementation::getCurrentApplication()
{
  std::wstring windowTitle, exeName;

  getForegroundAppInfo(windowTitle, exeName);

  // If we are the same application as last time, return the same code
  if (windowTitle == m_applicationInfoCache.windowTitle &&
      exeName == m_applicationInfoCache.exeName) {
    return m_applicationInfoCache.appCode;
  }

  // Cache the new application info
  m_applicationInfoCache.windowTitle = windowTitle;
  m_applicationInfoCache.exeName = exeName;
  m_applicationInfoCache.appCode = -1;

  // Search for a match
  for (size_t i=0; i<m_applicationInfos.size(); i++) {
    if ((m_applicationInfos[i].windowTitle == windowTitle ||
       (!m_applicationInfos[i].windowTitle.empty() &&
         windowTitle.find(m_applicationInfos[i].windowTitle) != std::wstring::npos &&
        !windowTitle.empty()))
     && (m_applicationInfos[i].exeName == exeName ||
       (!m_applicationInfos[i].exeName.empty() &&
         exeName.find(m_applicationInfos[i].exeName) != std::wstring::npos &&
        !exeName.empty()))) {
      m_applicationInfoCache.appCode = m_applicationInfos[i].appCode;
      break;
    }
  }
  return m_applicationInfoCache.appCode;
}

void OutputPeripheralImplementation::setIconVisibility(int index, bool visible)
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

int OutputPeripheralImplementation::findImageIndex(float z, float touchThreshold, float touchRange) const
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

void OutputPeripheralImplementation::drawImageIcon(int iconIndex, int imageIndex, float x, float y, bool visible)
{
  if (iconIndex < 0 || iconIndex >= m_numOverlayPoints || imageIndex < 0 || imageIndex >= m_numOverlayImages) {
    return;
  }
  LPPoint position = LPPointMake(static_cast<LPFloat>(x), static_cast<LPFloat>(y));
  m_overlayPoints[iconIndex]->SetImage(m_overlayImages[imageIndex], false);
  m_overlayPoints[iconIndex]->SetPosition(position);
  setIconVisibility(iconIndex, visible);
}

double OutputPeripheralImplementation::touchDistanceToRadius (float touchDistance) {
  return 40.0*std::min(std::max(touchDistance, 0.0f), 0.7f) + 10.0;
}

void OutputPeripheralImplementation::drawRasterIcon(int iconIndex,
                                                    float x,
                                                    float y,
                                                    bool visible,
                                                    const Vector3& velocity,
                                                    float touchDistance,
                                                    double radius,
                                                    float clampDistance,
                                                    float alphaMult,
                                                    int numFingers) {
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

void OutputPeripheralImplementation::emitTouchEvent(const TouchEvent& evt)
{
#if _WIN32
  // Translate and convert:
  std::set<Touch> unordered;

  for(auto q = evt.begin(); q != evt.end(); q++) {
    const Touch& cur = *q;

    // Clip to our screen:
    auto position = m_virtualScreen.ClipPosition(LPPoint((LPFloat)cur.x(), (LPFloat)cur.y()));

    // Insert into the new map:
    unordered.insert(Touch(
      cur.id(),
      position.x,
      position.y,
      cur.touching()
    ));
  }
  m_touchManager->setTouches(std::move(unordered));
#endif
}

void OutputPeripheralImplementation::emitTouchEvent(std::set<Touch>&& touches)
{
#if _WIN32
  m_touchManager->setTouches(std::move(touches));
#endif
}

bool OutputPeripheralImplementation::touchAvailable() const
{
#if _WIN32
  return true;
#else
  return false;
#endif
}

int OutputPeripheralImplementation::numTouchScreens() const {
#if _WIN32
  return m_touchManager->numTouchScreens();
#else
  return 0;
#endif
}

bool OutputPeripheralImplementation::emitGestureEvents(const Frame& frame, const Frame& sinceFrame)
{
  if (m_outputMode != m_lastOutputMode) {
    cancelGestureEvents();
    delete m_outputPeripheralMode;
//    m_outputPeripheralMode = createPeripheralFromMode(m_outputMode);
  }
  m_lastOutputMode = m_outputMode;
  if (m_outputPeripheralMode) {
    m_outputPeripheralMode->processFrame(frame, sinceFrame);
  }
  return true;
}

void OutputPeripheralImplementation::cancelGestureEvents()
{
  m_movingCursor = false;

#if _WIN32
  boost::unique_lock<boost::mutex> lock(m_touchMutex);
  m_touchManager->clearTouches();
#endif
  for (int i=0; i<m_numOverlayPoints; i++) {
    setIconVisibility(i, false);
  }
#if __APPLE__
  flushOverlay();
#endif
  endGesture();
}

#ifdef _WIN32
void windowsKeyDown(INPUT& input, WORD key) {
  input.type = INPUT_KEYBOARD;
  input.ki.wVk = key;
  input.ki.dwFlags = 0;
}
void windowsKeyUp(INPUT& input, WORD key) {
  input.type = INPUT_KEYBOARD;
  input.ki.wVk = key;
  input.ki.dwFlags = KEYEVENTF_KEYUP;
}
void windowsKeyCombo(WORD key1) {
  INPUT input[2] = {0};
  windowsKeyDown(input[0], key1);
  windowsKeyUp(input[1], key1);
  SendInput(2,input,sizeof(INPUT));
}
void windowsKeyCombo(WORD key1, WORD key2) {
  INPUT input[4] = {0};
  windowsKeyDown(input[0], key1);
  windowsKeyDown(input[1], key2);
  windowsKeyUp(input[2], key2);
  windowsKeyUp(input[3], key1);
  SendInput(4,input,sizeof(INPUT));
}
void windowsKeyCombo(WORD key1, WORD key2, WORD key3) {
  INPUT input[6] = {0};
  windowsKeyDown(input[0], key1);
  windowsKeyDown(input[1], key2);
  windowsKeyDown(input[2], key3);
  windowsKeyUp(input[3], key3);
  windowsKeyUp(input[4], key2);
  windowsKeyUp(input[5], key1);
  SendInput(6,input,sizeof(INPUT));
}

void OutputPeripheralImplementation::applyCharms(const Leap::Vector& aspectNormalized, int numPointablesActive, int& charmsMode) {
  if (charmsMode == 0 && numPointablesActive == 1) {
    if (aspectNormalized.x >= 1.07f) {
      //Send a "Win + C" to bring out or hide the charm bar
      windowsKeyCombo(VK_LWIN, VkKeyScan('c'));
      charmsMode = 1;
    } else if (aspectNormalized.x <= -0.07f) {
      //Send out a "Win + Ctrl + Tab" to bring out the switchable programs
      windowsKeyCombo(VK_LWIN, VK_CONTROL, VK_TAB);
      charmsMode = 2;
    } else if (aspectNormalized.y <= -0.07f) {
      //Send a "Win + Z" to bring out or hide the app commands
      windowsKeyCombo(VK_LWIN, VkKeyScan('z'));
      charmsMode = 3;
    }
  } else if (aspectNormalized.x >= 0.05f && aspectNormalized.x <= 0.95f &&
             aspectNormalized.y >= 0.05f && aspectNormalized.y <= 0.95f) {
    charmsMode = 0;
  }
}
#endif

float OutputPeripheralImplementation::acceptableClampDistance() const {
  return 0.6f;
}

void OutputPeripheralImplementation::setOutputMode(Touchless::GestureInteractionMode mode) {
  m_outputMode = mode;
}

Touchless::GestureInteractionMode OutputPeripheralImplementation::getOutputMode() const {
  return m_outputMode;
}

//Touchless::GestureInteractionManager* OutputPeripheralImplementation::createPeripheralFromMode(Touchless::GestureInteractionMode mode) {
//  switch(mode) {
//    case OutputPeripheral::OUTPUT_MODE_INTRO:
//      return new Touchless::OutputPeripheralGestureOnly(*this);
//    case OutputPeripheral::OUTPUT_MODE_BASIC:
//      OutputPeripheralBasic::SetBasicMode();
//      return new OutputPeripheralBasic(*this);
//    case OutputPeripheral::OUTPUT_MODE_ADVANCED:
//#if _WIN32
//      return new TouchPeripheral(*this);
//#else
//      return new OutputPeripheralFingerMouse(*this);
//#endif
//    default:
//      return nullptr;
//  }
//}

void OutputPeripheralImplementation::getForegroundAppInfo(std::wstring& windowTitle, std::wstring& exeName)
{
  m_appInfo.Update();
  windowTitle = m_appInfo.m_windowTitle;
  exeName = m_appInfo.m_ownerExeName;
}

void OutputPeripheralImplementation::getForegroundAppInfo(std::wstring& windowTitle, std::wstring& exeName, float& left, float& top, float& right, float& bottom)
{
  m_appInfo.Update();
  windowTitle = m_appInfo.m_windowTitle;
  exeName = m_appInfo.m_ownerExeName;
  RECT temp;
  temp = m_appInfo.m_rcClient;
  left = static_cast<float>(temp.left);
  top = static_cast<float>(temp.top);
  right = static_cast<float>(temp.right);
  bottom = static_cast<float>(temp.bottom);
}

bool OutputPeripheralImplementation::loadIfAvailable(std::shared_ptr<LPImage>& image, const std::string& fileName)
{
  if (image && FileSystemUtil::FileExists(fileName)) {
    std::wstring wide(fileName.begin(), fileName.end());
    return image->SetImage(wide);
  }
  return false;
}

uint32_t OutputPeripheralImplementation::gestureType() const
{
  return m_gesture.type();
}

bool OutputPeripheralImplementation::beginGesture(uint32_t gestureType)
{
  syncPosition();
  return m_gesture.begin(gestureType);
}

bool OutputPeripheralImplementation::endGesture()
{
  syncPosition();
  return m_gesture.end();
}

bool OutputPeripheralImplementation::applyZoom(float zoom)
{
  syncPosition();
  return m_gesture.applyZoom(zoom - 1.0f);
}

bool OutputPeripheralImplementation::applyRotation(float rotation)
{
  syncPosition();
  return m_gesture.applyRotation(rotation);
}

bool OutputPeripheralImplementation::applyScroll(float dx, float dy, int64_t timeDiff)
{
  syncPosition();
  return m_gesture.applyScroll(dx, dy, timeDiff);
}

bool OutputPeripheralImplementation::applyDesktopSwipe(float dx, float dy)
{
  syncPosition();
  return m_gesture.applyDesktopSwipe (dx, dy);
}

bool OutputPeripheralImplementation::useProceduralOverlay() const
{
  return m_UseProceduralOverlay;
}

bool OutputPeripheralImplementation::useCharmHelper() const {
  return m_useCharmHelper;
}

#if __APPLE__
void OutputPeripheralImplementation::flushOverlay()
{
  m_overlay.Flush();
}
#endif

void OutputPeripheralImplementation::syncPosition()
{
  LPPoint position;
#if __APPLE__
  CGEventRef eventRef = CGEventCreate(0);
  position = CGEventGetLocation(eventRef);
  CFRelease(eventRef);
#elif _WIN32
  POINT cursor = { 0, 0 };

  if (GetCursorPos(&cursor)) {
    position.x = static_cast<LPFloat>(cursor.x);
    position.y = static_cast<LPFloat>(cursor.y);
  }
#else
  throw_rethrowable std::logic_error("OutputPeripheralImplementation not implemented"); // Linux -- FIXME
#endif
  position = m_virtualScreen.SetPosition(position);
  m_gesture.setPosition(position.x, position.y);
}

//
// OutputPeripheral
//

OutputPeripheral::OutputPeripheral(Implementation* impl) : Interface(CREATE_SHARED_PTR_POINTER(impl)) {}
void OutputPeripheral::destroy() { get<OutputPeripheralImplementation>()->destroy(); }
bool OutputPeripheral::initializeTouchAndOverlay() { return get<OutputPeripheralImplementation>()->initializeTouchAndOverlay(); }
void OutputPeripheral::useDefaultScreen(bool use) { get<OutputPeripheralImplementation>()->useDefaultScreen(use); }
bool OutputPeripheral::usingDefaultScreen() const { return get<OutputPeripheralImplementation>()->usingDefaultScreen(); }
void OutputPeripheral::clickDown(int button) { get<OutputPeripheralImplementation>()->clickDown(button); }
void OutputPeripheral::clickUp(int button) { get<OutputPeripheralImplementation>()->clickUp(button); }
bool OutputPeripheral::isClickedDown(int button) const { return get<OutputPeripheralImplementation>()->isClickedDown(button); }
void OutputPeripheral::keyDown(int button) { get<OutputPeripheralImplementation>()->keyDown(button); }
void OutputPeripheral::keyUp(int button) { get<OutputPeripheralImplementation>()->keyUp(button); }
Vector OutputPeripheral::deviceToScreen(const Vector& position, float scale, bool clamp) const { Vector out, temp; get<OutputPeripheralImplementation>()->deviceToScreen(position, out, temp, scale, clamp); return out; }
bool OutputPeripheral::cursorPosition(float* x, float* y) const { return get<OutputPeripheralImplementation>()->cursorPosition(x, y); }
void OutputPeripheral::setCursorPosition(float x, float y, bool absolute) { get<OutputPeripheralImplementation>()->setCursorPosition(x, y, absolute); }
void OutputPeripheral::registerApplication(const std::string& windowTitle, const std::string& exeName, int appCode) { get<OutputPeripheralImplementation>()->registerApplication(windowTitle, exeName, appCode); }
void OutputPeripheral::registerApplication(const std::wstring& windowTitle, const std::wstring& exeName, int appCode) { get<OutputPeripheralImplementation>()->registerApplication(windowTitle, exeName, appCode); }
int OutputPeripheral::getCurrentApplication() { return get<OutputPeripheralImplementation>()->getCurrentApplication(); }
void OutputPeripheral::setIconVisibility(int index, bool visible) { get<OutputPeripheralImplementation>()->setIconVisibility(index, visible); }
int OutputPeripheral::findImageIndex(float z, float touchThreshold, float touchRange) const { return get<OutputPeripheralImplementation>()->findImageIndex(z, touchThreshold, touchRange); }
void OutputPeripheral::drawIcon(int iconIndex, int imageIndex, float x, float y, bool visible) { get<OutputPeripheralImplementation>()->drawImageIcon(iconIndex, imageIndex, x, y, visible); }
void OutputPeripheral::emitTouchEvent(const TouchEvent& evt) { get<OutputPeripheralImplementation>()->emitTouchEvent(evt); }
bool OutputPeripheral::touchAvailable() const { return get<OutputPeripheralImplementation>()->touchAvailable(); }
int OutputPeripheral::numTouchScreens() const { return get<OutputPeripheralImplementation>()->numTouchScreens(); }
int OutputPeripheral::touchVersion() const { return -1; }
bool OutputPeripheral::emitGestureEvents(const Frame& frame, const Frame& sinceFrame) { return get<OutputPeripheralImplementation>()->emitGestureEvents(frame, sinceFrame); }
void OutputPeripheral::cancelGestureEvents() { get<OutputPeripheralImplementation>()->cancelGestureEvents(); }
void OutputPeripheral::setOutputMode(Touchless::GestureInteractionMode mode) { get<OutputPeripheralImplementation>()->setOutputMode(mode); }
Touchless::GestureInteractionMode OutputPeripheral::getOutputMode() const { return get<OutputPeripheralImplementation>()->getOutputMode(); }
