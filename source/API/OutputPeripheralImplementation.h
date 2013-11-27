/*==================================================================================================================

    Copyright (c) 2010 - 2013 Leap Motion. All rights reserved.

  The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
  protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
  strictly forbidden unless prior written permission is obtained from Leap Motion.

===================================================================================================================*/

#if !defined(__OutputPeripheralImplementation_h__)
#define __OutputPeripheralImplementation_h__

#include "ocuType.h"

#if LEAP_API_INTERNAL
#include "LeapInternal.h"
#else
#include "Leap.h"
#endif

#if __APPLE__
#include "LPMac.h"
#include "LPOverlay.h"
#elif _WIN32
#include <Windows.h>
#include "FocusAppInfo.h"
#else
#include "LPLinux.h"
#endif

#include "LeapPlugin.h"
#include "LeapPluginPlus.h"
#include "LPVirtualScreen.h"
#include "LPGesture.h"
#include "LPImage.h"
#include "LPIcon.h"
#include "DataTypes.h"
#include "OutputPeripheralBasic.h"
#include "OutputPeripheralFingerMouse.h"
#include "OutputPeripheralGestureOnly.h"
#include "TouchManager.h"

#include <vector>
#include <boost/circular_buffer.hpp>
#include <boost/thread.hpp>

namespace Leap {

class OutputPeripheralImplementation : public Interface::Implementation {
  public:
    OutputPeripheralImplementation();
    ~OutputPeripheralImplementation();

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
    void clearTouchPoints();
    void removeTouchPoint(int touchId);
    void addTouchPoint(int touchId, float x, float y, bool touching);
    void emitTouchEvent();
    bool touchAvailable() const;
    int numTouchScreens() const;
    int touchVersion() const;

    bool emitGestureEvents(const Frame& frame, const Frame& sinceFrame);
    void cancelGestureEvents();

    void setOutputMode(OutputPeripheral::OutputMode mode);
    OutputPeripheral::OutputMode getOutputMode() const;
    OutputPeripheralMode* createPeripheralFromMode(OutputPeripheral::OutputMode mode);

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

    std::vector<AppInfo>            m_applicationInfos;
    AppInfo                         m_applicationInfoCache;
    LPVirtualScreen                 m_virtualScreen;
    LPGesture                       m_gesture;
    std::vector<std::shared_ptr<LPImage>> m_overlayImages;
    std::vector<std::shared_ptr<LPIcon>> m_overlayPoints;
    int                             m_numOverlayPoints;
    int                             m_numOverlayImages;
    int                             m_filledImageIdx;
    CFocusAppInfo                   m_appInfo;
    bool                            m_buttonDown;
    bool                            m_clickedButtons[NUM_BUTTONS];
    bool                            m_movingCursor;
    OutputPeripheral::OutputMode    m_lastOutputMode;
    OutputPeripheral::OutputMode    m_outputMode;
#if _WIN32
    ULONG_PTR                       m_gdiplusToken;
#endif
    int                             m_lastNumIcons;
    TouchManager                    m_touchManager;
    std::set<Touch>                 m_touches;
    boost::mutex                    m_touchMutex;
    bool                            m_UseProceduralOverlay;
    bool                            m_useCharmHelper;
    OutputPeripheralMode           *m_outputPeripheralMode;
#if __APPLE__
    LPOverlay                       m_overlay;
#endif
};

}

#endif // __OutputPeripheralImplementation_h__
