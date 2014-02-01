// Copyright (c) 2010 - 2013 Leap Motion. All rights reserved. Proprietary and confidential.
#ifndef __TouchlessListener_h__
#define __TouchlessListener_h__

#include "Leap.h"
#include "LPVirtualScreen.h"
#include "OSInteraction.h"
#include "Overlay.h"
#include "GestureInteractionManager.h"
#include "GestureInteractionManager.h"
#include "OutputPeripheralBasic.h"
#include "OutputPeripheralFingerMouse.h"
#include "OutputPeripheralGestureOnly.h"

#include <qobject.h>

#include <boost/thread.hpp>

class TouchlessListener : public QObject, public Leap::Listener {

  Q_OBJECT

public:

  TouchlessListener();
  ~TouchlessListener();

  virtual void onInit(const Leap::Controller& leap) override;
  virtual void onConnect(const Leap::Controller& leap) override;
  virtual void onDisconnect(const Leap::Controller& leap) override;
  virtual void onExit(const Leap::Controller& leap) override;
  virtual void onFrame(const Leap::Controller& leap) override;
  virtual void onFocusGained(const Leap::Controller& leap) override;
  virtual void onFocusLost(const Leap::Controller& leap) override;

  Touchless::GestureInteractionMode getDesiredMode() const;
  void setDesiredMode(Touchless::GestureInteractionMode mode);

  bool getUseMultipleMonitors() const;
  void setUseMultipleMonitors(bool use);
  int supportedTouchVersion() const;

  void setReady();

Q_SIGNALS:

  void connectChangedSignal(bool connected, int mode, bool useMultiMonitors);

private:

  void updateDefaultScreen();

  Leap::Frame m_lastFrame;
  bool m_updateSettings;
  bool m_useMultipleMonitors;
  boost::condition_variable m_condVar;
  boost::mutex m_mutex;
  bool m_ready;

  LPVirtualScreen                       m_virtualScreen;
  Touchless::OSInteractionDriver       *m_osInteractionDriver;
  Touchless::OverlayDriver             *m_overlayDriver;
  Touchless::GestureInteractionMode     m_desiredMode;
  Touchless::GestureInteractionManager *m_interactionManager;

};

#endif
