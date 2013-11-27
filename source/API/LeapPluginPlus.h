/******************************************************************************\
* Copyright (C) 2012-2013 Leap Motion, Inc. All rights reserved.               *
* Leap Motion proprietary and confidential. Not for distribution.              *
* Use subject to the terms of the Leap Motion SDK Agreement available at       *
* https://developer.leapmotion.com/sdk_agreement, or another agreement         *
* between Leap Motion and you, your company or other organization.             *
\******************************************************************************/

#if !defined(__LeapPluginPlus_h__)
#define __LeapPluginPlus_h__

#include "LeapPlugin.h"

namespace Leap {

//
// Public Interface
//

//
// This is the interface to the underlying OS peripherals (i.e., keyboard,
// mouse, touch, display). This is intended to be used with the legacy Plugin
// interface to simulate OS events based on Leap events.
//
// Do not attempt to access the OutputPeripheral information in your
// sub-classed Plugin constructor. References to that class will not be
// available until the onInit method.
//
class OutputPeripheral : public Interface {
  public:

    enum OutputMode { OUTPUT_MODE_DISABLED = 0, OUTPUT_MODE_INTRO, OUTPUT_MODE_BASIC, OUTPUT_MODE_ADVANCED };

    OutputPeripheral(Implementation*);

    void destroy();

    bool initializeTouchAndOverlay();

    void useDefaultScreen(bool use = true);
    bool usingDefaultScreen() const;

    void clickDown(int button);
    void clickUp(int button);
    bool isClickedDown(int button) const;
    void keyDown(int code);
    void keyUp(int code);

    Vector deviceToScreen(const Vector& position, float scale = 1, bool clamp = true) const;

    bool cursorPosition(float* x, float* y) const;
    void setCursorPosition(float x, float y, bool absolute = true);

    void registerApplication(const std::string& windowTitle, const std::string& exeName, int appCode);
    void registerApplication(const std::wstring& windowTitle, const std::wstring& exeName, int appCode);
    int getCurrentApplication();

    void setIconVisibility(int index, bool visible);
    int findImageIndex(float z, float touchThreshold, float touchRange) const;
    void drawIcon(int iconIndex, int imageIndex, float x, float y, bool visible);
    void clearTouchPoints();
    void removeTouchPoint(int touchId);
    void addTouchPoint(int touchId, float x, float y, bool touching);
    void emitTouchEvent();
    bool touchAvailable() const;
    int numTouchScreens() const;
    int touchVersion() const;

    bool emitGestureEvents(const Frame& frame, const Frame& sinceFrame);
    void cancelGestureEvents();

    void setOutputSensitivity(double sens);
    void setOutputMode(OutputMode mode);
    OutputMode getOutputMode() const;
};

class PluginPlus : public Plugin {
  public:
    PluginPlus(const Plugin& plugin);
    PluginPlus();

    OutputPeripheral& outputPeripheral() const;
};

}

#endif // __LeapPluginPlus_h__
