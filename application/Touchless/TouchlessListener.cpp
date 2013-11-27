// Copyright (c) 2010 - 2013 Leap Motion. All rights reserved. Proprietary and confidential.
#include "TouchlessListener.h"
#include "Configuration/Config.h"
#include "FileSystemUtil.h"

#include <fstream>

TouchlessListener::TouchlessListener()
  : m_outputPeripheral(new Leap::OutputPeripheralImplementation())
{
  Config::InitializeDefaults();
  std::string configPath = FileSystemUtil::GetUserPath("touchless-config.json");
  if (!FileSystemUtil::FileExists(configPath)) {
    try {
      std::ofstream configFile(configPath.c_str());
      Value::Hash emptyConfig;
      emptyConfig["configuration"] = Value::Hash();
      configFile << Value(emptyConfig).ToJSON();
      configFile.close();
    } catch (...) { }
  }

  Config::LoadFromFile(configPath, false);
  m_outputPeripheral.initializeTouchAndOverlay();
  m_desiredMode = Leap::OutputPeripheral::OUTPUT_MODE_DISABLED;
  m_updateSettings = false;
  m_useMultipleMonitors = false;
  m_ready = false;
  updateDefaultScreen();
  m_outputPeripheral.setOutputMode(m_desiredMode);
}

TouchlessListener::~TouchlessListener() {
  m_outputPeripheral.destroy();
}

void TouchlessListener::onInit(const Leap::Controller& leap) { }

void TouchlessListener::onConnect(const Leap::Controller& leap) {
  leap.setPolicyFlags(static_cast<Leap::Controller::PolicyFlag>((0xDA << 24) | // Secret sauce
                      Leap::Controller::POLICY_BACKGROUND_FRAMES));
  int tempMode = static_cast<int>(Leap::OutputPeripheral::OUTPUT_MODE_DISABLED);
  Config::GetAttribute<int>("os_interaction_mode", tempMode);
  m_desiredMode = static_cast<enum Leap::OutputPeripheral::OutputMode>(tempMode);
  Config::GetAttribute<bool>("os_interaction_multi_monitor", m_useMultipleMonitors);
  boost::unique_lock<boost::mutex> lock(m_mutex);
  if (!m_ready) {
    m_condVar.wait(lock);
  }
  Q_EMIT(connectChangedSignal(true, tempMode, m_useMultipleMonitors));
}

void TouchlessListener::onDisconnect(const Leap::Controller& leap) {
  m_outputPeripheral.cancelGestureEvents();
  m_lastFrame = Leap::Frame();
  Q_EMIT(connectChangedSignal(false, static_cast<int>(m_desiredMode), m_useMultipleMonitors));
}

void TouchlessListener::onExit(const Leap::Controller& leap) {
  m_outputPeripheral.cancelGestureEvents();
  Q_EMIT(connectChangedSignal(false, static_cast<int>(m_desiredMode), m_useMultipleMonitors));
}

void TouchlessListener::onFrame(const Leap::Controller& leap) {
  const Leap::Frame frame = leap.frame();

  if (m_updateSettings) {
    Config::SetAttribute("os_interaction_multi_monitor", m_useMultipleMonitors, true);
    Config::SetAttribute("os_interaction_mode", static_cast<int>(m_desiredMode), true);
    Config::Save();
    m_updateSettings = false;
  }
  m_outputPeripheral.emitGestureEvents(frame, m_lastFrame);
  m_lastFrame = frame;
}

void TouchlessListener::onFocusGained(const Leap::Controller& leap) {
  updateDefaultScreen();
}

void TouchlessListener::onFocusLost(const Leap::Controller& leap) {
  m_outputPeripheral.cancelGestureEvents();
  m_lastFrame = Leap::Frame();
}

Leap::OutputPeripheral::OutputMode TouchlessListener::getDesiredMode() const {
  return m_desiredMode;
}

void TouchlessListener::setDesiredMode(Leap::OutputPeripheral::OutputMode mode) {
  m_desiredMode = mode;
  m_updateSettings = true;
  m_outputPeripheral.setOutputMode(m_desiredMode);
}

bool TouchlessListener::getUseMultipleMonitors() const {
  return m_useMultipleMonitors;
}

void TouchlessListener::setUseMultipleMonitors(bool use) {
  m_useMultipleMonitors = use;
  m_updateSettings = true;
  updateDefaultScreen();
}

int TouchlessListener::supportedTouchVersion() const {
  return m_outputPeripheral.touchVersion();
}

void TouchlessListener::setReady() {
  boost::unique_lock<boost::mutex> lock(m_mutex);
  m_ready = true;
  m_condVar.notify_all();
}

void TouchlessListener::updateDefaultScreen() {
  if (m_useMultipleMonitors) {
    const int numScreens = m_outputPeripheral.numTouchScreens();
    m_outputPeripheral.useDefaultScreen(numScreens == 1);
  } else {
    m_outputPeripheral.useDefaultScreen(true);
  }
}
