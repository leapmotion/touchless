// Copyright (c) 2010 - 2013 Leap Motion. All rights reserved. Proprietary and confidential.
#ifndef __Touchless_h__
#define __Touchless_h__

#include <qapplication.h>

// forward declarations
class TouchlessListener;
class TouchlessUI;
namespace Leap {
  class Controller;
}

class TouchlessApp : public QApplication {

  Q_OBJECT

public:

  TouchlessApp(int argc, char** argv);
  ~TouchlessApp();
  int Run();
  bool IsAlreadyRunning() const;

public Q_SLOTS:
  
  void disabledModeSlot();
  void introModeSlot();
  void basicModeSlot();
  void advancedModeSlot();
  void multiMonitorSlot(bool use);

private:

  TouchlessUI* m_UI;
  TouchlessListener* m_listener;
  Leap::Controller* m_controller;

};

#endif
