#if !defined(__OutputPeripheralImplementation_h__)
#define __OutputPeripheralImplementation_h__

#include "common.h"

#if __APPLE__
#include "Peripherals/LPMac.h"
#include "Peripherals/LPOverlay.h"
#elif !defined _WIN32
#include "LPLinux.h"
#else
#include "FocusAppInfo.h"
#endif
#include "LPVirtualScreen.h"

#include "OutputPeripheralBasic.h"
#include "OutputPeripheralFingerMouse.h"
#include "OutputPeripheralGestureOnly.h"
#include "Touch.h"
#include "Peripherals/LPGesture.h"
#include "TouchManager.h"
#include <boost/circular_buffer.hpp>
#include <boost/thread/mutex.hpp>
#include <vector>

class LPIcon;
class LPImage;

namespace Touchless {

class OSInteractionDriver
{
public:
  OSInteractionDriver(LPVirtualScreen &virtualScreen);
  ~OSInteractionDriver();

  static OSInteractionDriver* New(LPVirtualScreen &virtualScreen);

  void destroy();

  bool initializeTouch();

  void useDefaultScreen(bool use);

  void clickDown(int button, int number = 1);
  void clickUp(int button, int number = 1);
  bool isClickedDown(int button) const;
  void keyDown(int code);
  void keyUp(int code);

  bool cursorPosition(float* fx, float* fy) const;
  void setCursorPosition(float fx, float fy, bool absolute = true);

  bool emitGestureEvents(const Frame& frame, const Frame& sinceFrame);
  void cancelGestureEvents();

  uint32_t gestureType() const;
  bool beginGesture(uint32_t gestureType);
  bool endGesture();
  bool applyZoom(float zoom);
  bool applyRotation(float rotation);
  bool applyScroll(float dx, float dy, int64_t timeDiff = 0);
  bool applyDesktopSwipe(float dx, float dy);
  void applyCharms(const Leap::Vector& aspectNormalized, int numPointablesActive, int& charmsMode);

  bool useCharmHelper() const;

  bool checkTouching(const Vector& position, float noTouchBorder) const;
  void emitTouchEvent(const TouchEvent& evt);

  bool touchAvailable() const;

  int numTouchScreens() const;

  void emitKeyboardEvent(int key, bool down);
  void emitKeyboardEvents(int* keys, int numKeys, bool down);
  void syncPosition();

  enum { NUM_BUTTONS = 8 };

  LPVirtualScreen &m_virtualScreen;
  LPGesture        m_gesture;
  TouchManager    *m_touchManager;
  bool             m_movingCursor;
  boost::mutex     m_touchMutex;
  bool             m_buttonDown;
  bool             m_clickedButtons[NUM_BUTTONS];
  bool             m_useCharmHelper;


};

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
  float acceptableClampDistance() const;

  void setIconVisibility(int index, bool visible);
  int findImageIndex(float z, float touchThreshold, float touchRange) const;
  void drawImageIcon(int iconIndex, int imageIndex, float x, float y, bool visible);

  static double touchDistanceToRadius (float touchDistance);
  void drawRasterIcon(int iconIndex, float x, float y, bool visible, const Vector3& velocity, float touchDistance, double radius, float clampDistance, float alphaMult, int numFingers = 1);
  bool checkTouching(const Vector& position, float noTouchBorder) const;
  void emitTouchEvent(const TouchEvent& evt);
  bool touchAvailable() const;

  int numTouchScreens() const;

  bool useProceduralOverlay() const;

  bool loadIfAvailable(std::shared_ptr<LPImage>& image, const std::string& fileName);

#if __APPLE__
  void flushOverlay();
#endif

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

namespace Leap {

class OutputPeripheralImplementation:
  public Interface::Implementation
{
public:
  OutputPeripheralImplementation();
  ~OutputPeripheralImplementation();

  static OutputPeripheralImplementation* New(void);

  void destroy();

  bool initializeTouchAndOverlay();

  void useDefaultScreen(bool use);
  bool usingDefaultScreen() const;

  void clickDown(int button, int number = 1);
  void clickUp(int button, int number = 1);
  bool isClickedDown(int button) const;
  void keyDown(int code);
  void keyUp(int code);

  bool deviceToScreen(const Vector& position, Vector& output, Vector& clampVec, float scale = 1, bool clamp = true);
  bool normalizedToScreen(const Vector& position, Vector& output, Vector& clampVec, float scale = 1, bool clamp = true);
  bool normalizedToAspect(const Vector& position, Vector& output, Vector& clampVec, float scale = 1, bool clamp = true);

  bool cursorPosition(float* fx, float* fy) const;
  void setCursorPosition(float fx, float fy, bool absolute = true);

  void registerApplication(const std::string& windowTitle, const std::string& exeName, int appCode);
  void registerApplication(const std::wstring& windowTitle, const std::wstring& exeName, int appCode);
  int getCurrentApplication();

  void setIconVisibility(int index, bool visible);
  int findImageIndex(float z, float touchThreshold, float touchRange) const;
  void drawImageIcon(int iconIndex, int imageIndex, float x, float y, bool visible);

  static double touchDistanceToRadius (float touchDistance);
  void drawRasterIcon(int iconIndex, float x, float y, bool visible, const Vector3& velocity, float touchDistance, double radius, float clampDistance, float alphaMult, int numFingers = 1);
  bool checkTouching(const Vector& position, float noTouchBorder) const;
  void emitTouchEvent(const TouchEvent& evt);
  bool touchAvailable() const;

  int numTouchScreens() const;

  bool emitGestureEvents(const Frame& frame, const Frame& sinceFrame);
  void cancelGestureEvents();

  void setOutputMode(Touchless::GestureInteractionMode mode);
  Touchless::GestureInteractionMode getOutputMode() const;
  Touchless::GestureInteractionManager* createPeripheralFromMode(Touchless::GestureInteractionMode mode);

  uint32_t gestureType() const;
  bool beginGesture(uint32_t gestureType);
  bool endGesture();
  bool applyZoom(float zoom);
  bool applyRotation(float rotation);
  bool applyScroll(float dx, float dy, int64_t timeDiff = 0);
  bool applyDesktopSwipe(float dx, float dy);
  void applyCharms(const Leap::Vector& aspectNormalized, int numPointablesActive, int& charmsMode);
  float acceptableClampDistance() const;

  bool useProceduralOverlay() const;
  bool useCharmHelper() const;

#if __APPLE__
  void flushOverlay();
#endif

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

private:

  void emitKeyboardEvent(int key, bool down);
  void emitKeyboardEvents(int* keys, int numKeys, bool down);
  void getForegroundAppInfo(std::wstring& windowTitle, std::wstring& exeName);
  void getForegroundAppInfo(std::wstring& windowTitle, std::wstring& exeName, float& left, float& top, float& right, float& bottom);
  bool loadIfAvailable(std::shared_ptr<LPImage>& image, const std::string& fileName);
  void syncPosition();

  struct AppInfo
  {
    std::wstring windowTitle;
    std::wstring exeName;
    int appCode;
  };

  enum { NUM_BUTTONS = 8 };

  CFocusAppInfo m_appInfo;
  LPVirtualScreen m_virtualScreen;
  TouchManager* m_touchManager;

  std::vector<AppInfo>            m_applicationInfos;
  AppInfo                         m_applicationInfoCache;
  LPGesture                       m_gesture;
  std::vector<std::shared_ptr<LPImage>> m_overlayImages;
  std::vector<std::shared_ptr<LPIcon>> m_overlayPoints;
  int                             m_numOverlayPoints;
  int                             m_numOverlayImages;
  int                             m_filledImageIdx;
  bool                            m_buttonDown;
  bool                            m_clickedButtons[NUM_BUTTONS];
  bool                            m_movingCursor;
  Touchless::GestureInteractionMode    m_lastOutputMode;
  Touchless::GestureInteractionMode    m_outputMode;
  int                             m_lastNumIcons;
  boost::mutex                    m_touchMutex;
  bool                            m_UseProceduralOverlay;
  bool                            m_useCharmHelper;
  Touchless::GestureInteractionManager      *m_outputPeripheralMode;
#if __APPLE__
  LPOverlay                       m_overlay;
#endif

public:
  Touchless::GestureInteractionManager* GetPeripheralMode(void) const {return m_outputPeripheralMode;}

  /// <summary>
  /// Emits the touch events specified in the passed r-value collection
  /// </summary>
  void emitTouchEvent(std::set<Touch>&& touches);
};

}

#endif // __OutputPeripheralImplementation_h__
