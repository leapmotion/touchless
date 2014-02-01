// Copyright (c) 2010 - 2013 Leap Motion. All rights reserved. Proprietary and confidential.
#include "Touchless.h"
#include "TouchlessListener.h"
#include "TouchlessUI.h"
#include "common.h"
#include "Leap.h"
#include "Utility/FileSystemUtil.h"

#include <QMessageBox>
#include <qlocalserver.h>
#include <qlocalsocket.h>
#include <qdir.h>

TouchlessApp::TouchlessApp(int argc, char** argv)
  : QApplication(argc, argv)
  , m_listener(0)
  , m_controller(0)
  , m_UI(0)
{ }

TouchlessApp::~TouchlessApp() {
#if 0 // the below cleanup code seems to cause an occasional hang on exit
  if (m_controller && m_listener) {
    m_controller->removeListener(*m_listener);
  }
  if (m_listener) {
    delete m_listener;
    m_listener = 0;
  }
  if (m_controller) {
    delete m_controller;
    m_controller = 0;
  }
#endif
}

#define SINGLE_INSTANCE_SERVER_NAME "Touchless"

int TouchlessApp::Run() {
  int returnCode = 0;
  try {
    {
      QDir dir(QApplication::applicationDirPath());
      // set working directory so path to static-docs is accurate.
#if __APPLE__
      dir.cd("../Resources");
#endif

      FileSystemUtil::ChangeWorkingDirectory(QDir::cleanPath(dir.absolutePath()).toUtf8().constData());
    }

    setQuitOnLastWindowClosed(false);
    QLocalServer* pLocalServer = new QLocalServer(this);
    pLocalServer->listen(SINGLE_INSTANCE_SERVER_NAME);

    m_listener = new TouchlessListener();

#ifdef _WIN32
    //Check to see if touch is supported at all
    if (m_listener->supportedTouchVersion() == 0) {
      QMessageBox messageBox;
      messageBox.setWindowIcon(TouchlessUI::GenerateTouchlessIcon());
      messageBox.setWindowFlags(Qt::WindowStaysOnTopHint);
      messageBox.setWindowTitle("Touchless");
      messageBox.setIcon(QMessageBox::Critical);
      messageBox.setText("The Leap Motion Touch Emulation Driver was corrupted or missing.  You may need to reinstall the Leap Motion Software.");
      messageBox.exec();
      return 1;
    }
#endif

    m_controller = new Leap::Controller(*m_listener);
    m_UI = new TouchlessUI();
    connect(m_UI, SIGNAL(disabledModeSignal()), this, SLOT(disabledModeSlot()));
    connect(m_UI, SIGNAL(introModeSignal()), this, SLOT(introModeSlot()));
    connect(m_UI, SIGNAL(basicModeSignal()), this, SLOT(basicModeSlot()));
    connect(m_UI, SIGNAL(advancedModeSignal()), this, SLOT(advancedModeSlot()));
    connect(m_UI, SIGNAL(multiMonitorSignal(bool)), this, SLOT(multiMonitorSlot(bool)));
    connect(m_listener, SIGNAL(connectChangedSignal(bool, int, bool)), m_UI, SLOT(connectChangedSlot(bool, int, bool)));
    if (m_listener->supportedTouchVersion() == 7) {
      m_UI->setMultipleMonitorsActionVisible(false);
    }
    m_listener->setReady();
    returnCode = exec();
    delete pLocalServer;
  } catch(...) { }
  return returnCode;
}

bool TouchlessApp::IsAlreadyRunning() const {
  QLocalSocket socket;
  socket.connectToServer(SINGLE_INSTANCE_SERVER_NAME);
  if (socket.waitForConnected(500)) {
    return true; // Exit already a process running
  }
  return false;
}

void TouchlessApp::disabledModeSlot() {
  m_listener->setDesiredMode(Touchless::GestureInteractionMode::OUTPUT_MODE_DISABLED);
}

void TouchlessApp::introModeSlot() {
  m_listener->setDesiredMode(Touchless::GestureInteractionMode::OUTPUT_MODE_INTRO);
}

void TouchlessApp::basicModeSlot() {
  m_listener->setDesiredMode(Touchless::GestureInteractionMode::OUTPUT_MODE_BASIC);
}

void TouchlessApp::advancedModeSlot() {
  m_listener->setDesiredMode(Touchless::GestureInteractionMode::OUTPUT_MODE_ADVANCED);
}

void TouchlessApp::multiMonitorSlot(bool use) {
  m_listener->setUseMultipleMonitors(use);
}

inline void loadResources() {
  //Q_INIT_RESOURCE();
}

int main(int argc, char ** argv)
{
  loadResources();
  TouchlessApp app(argc, argv);
  if (!app.IsAlreadyRunning()) {
    return app.Run();
  }

  return 0;
}

#ifdef _WIN32
#include <Windows.h>
int __stdcall WinMain(HINSTANCE hThisInstance, HINSTANCE hPrevInstance, LPSTR lpszArgument, int iCmdShow)
{
  return main( __argc, __argv );
}
#endif // _WIN32
