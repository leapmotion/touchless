// Copyright (c) 2010 - 2013 Leap Motion. All rights reserved. Proprietary and confidential.
#ifndef __TouchlessListener_h__
#define __TouchlessListener_h__

#include "Leap.h"
#include "LeapPluginPlus.h"
#include "OutputPeripheralImplementation.h"

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

  Leap::OutputPeripheral::OutputMode getDesiredMode() const;
  void setDesiredMode(Leap::OutputPeripheral::OutputMode mode);

  bool getUseMultipleMonitors() const;
  void setUseMultipleMonitors(bool use);
  int supportedTouchVersion() const;

  void setReady();

Q_SIGNALS:

  void connectChangedSignal(bool connected, int mode, bool useMultiMonitors);

private:

  void updateDefaultScreen();

  Leap::OutputPeripheral m_outputPeripheral;
  Leap::Frame m_lastFrame;
  bool m_updateSettings;
  bool m_useMultipleMonitors;
  Leap::OutputPeripheral::OutputMode m_desiredMode;
  boost::condition_variable m_condVar;
  boost::mutex m_mutex;
  bool m_ready;

};

#endif
