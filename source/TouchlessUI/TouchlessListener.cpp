// Copyright (c) 2010 - 2013 Leap Motion. All rights reserved. Proprietary and confidential.
#include "TouchlessListener.h"
#include "Configuration/Config.h"
#include "FileSystemUtil.h"
#include <fstream>

TouchlessListener::TouchlessListener()
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
  m_desiredMode = Touchless::GestureInteractionMode::OUTPUT_MODE_DISABLED;
  m_updateSettings = false;
  m_useMultipleMonitors = false;
  m_ready = false;
  m_osInteractionDriver = Touchless::OSInteractionDriver::New(m_virtualScreen);
  m_overlayDriver       = Touchless::OverlayDriver::New(m_virtualScreen);
  m_interactionManager  = Touchless::GestureInteractionManager::New(m_desiredMode, *m_osInteractionDriver, *m_overlayDriver);

  m_osInteractionDriver->initializeTouch();
  m_overlayDriver->initializeOverlay();

  updateDefaultScreen();
}

TouchlessListener::~TouchlessListener() {
  delete m_osInteractionDriver;
  delete m_overlayDriver;
  delete m_interactionManager;
}

void TouchlessListener::onInit(const Leap::Controller& leap) { }

void TouchlessListener::onConnect(const Leap::Controller& leap) {
  leap.setPolicyFlags(static_cast<Leap::Controller::PolicyFlag>((0xDA << 24) | // Secret sauce
                      Leap::Controller::POLICY_BACKGROUND_FRAMES));
  int tempMode = static_cast<int>(Touchless::GestureInteractionMode::OUTPUT_MODE_DISABLED);
  Config::GetAttribute<int>("os_interaction_mode", tempMode);
  m_desiredMode = static_cast<enum Touchless::GestureInteractionMode>(tempMode);
  Config::GetAttribute<bool>("os_interaction_multi_monitor", m_useMultipleMonitors);
  boost::unique_lock<boost::mutex> lock(m_mutex);
  if (!m_ready) {
    m_condVar.wait(lock);
  }
  Q_EMIT(connectChangedSignal(true, tempMode, m_useMultipleMonitors));
}

void TouchlessListener::onDisconnect(const Leap::Controller& leap) {
  m_osInteractionDriver->cancelGestureEvents();
  m_lastFrame = Leap::Frame();
  Q_EMIT(connectChangedSignal(false, static_cast<int>(m_desiredMode), m_useMultipleMonitors));
}

void TouchlessListener::onExit(const Leap::Controller& leap) {
  m_osInteractionDriver->cancelGestureEvents();
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
  if (m_interactionManager) {
    m_interactionManager->processFrame(frame, m_lastFrame);
  }
  m_lastFrame = frame;
}

void TouchlessListener::onFocusGained(const Leap::Controller& leap) {
  updateDefaultScreen();
}

void TouchlessListener::onFocusLost(const Leap::Controller& leap) {
  m_osInteractionDriver->cancelGestureEvents();
  m_lastFrame = Leap::Frame();
}

Touchless::GestureInteractionMode TouchlessListener::getDesiredMode() const {
  return m_desiredMode;
}

void TouchlessListener::setDesiredMode(Touchless::GestureInteractionMode mode) {
  m_desiredMode = mode;
  m_updateSettings = true;

  delete m_interactionManager;
  m_osInteractionDriver->cancelGestureEvents();
  m_interactionManager  = Touchless::GestureInteractionManager::New(m_desiredMode, *m_osInteractionDriver, *m_overlayDriver);
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
  return m_osInteractionDriver->touchVersion();
}

void TouchlessListener::setReady() {
  boost::unique_lock<boost::mutex> lock(m_mutex);
  m_ready = true;
  m_condVar.notify_all();
}

void TouchlessListener::updateDefaultScreen() {
  if (m_useMultipleMonitors) {
    const int numScreens = m_osInteractionDriver->numTouchScreens();
    m_osInteractionDriver->useDefaultScreen(numScreens == 1);
  } else {
    m_osInteractionDriver->useDefaultScreen(true);
  }
}
