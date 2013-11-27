// Copyright (c) 2010 - 2013 Leap Motion. All rights reserved. Proprietary and confidential.
#ifndef __TouchlessUI_h__
#define __TouchlessUI_h__

#include "ocuConfig.h"

#include <qmainwindow.h>
#include <qsystemtrayicon.h>

// forward declarations
class QMenu;
class QAction;
class QActionGroup;
class QWebView;
class QWidget;

class TouchlessUI : public QMainWindow {

  Q_OBJECT

public:

  TouchlessUI();
  static QIcon GenerateTouchlessIcon();

Q_SIGNALS:

  void disabledModeSignal();
  void introModeSignal();
  void basicModeSignal();
  void advancedModeSignal();
  void multiMonitorSignal(bool use);

public Q_SLOTS:

  void disabledModeSlot(bool enabled);
  void introModeSlot(bool enabled);
  void basicModeSlot(bool enabled);
  void advancedModeSlot(bool enabled);
  void exitSlot();
  void updateTrayIconSlot();
  void trayIconActivatedSlot(QSystemTrayIcon::ActivationReason reason);
  void connectChangedSlot(bool connected, int mode, bool useMultiMonitors);
  void openTutorialSlot();
  void openDocumentationSlot();
  void setMultipleMonitorsActionVisible(bool visible);

private:

  bool isFirstRun();
  void writeFirstRunSentinel();

  QSystemTrayIcon* m_trayIcon;
  QMenu* m_trayMenu;
  QMenu* m_modeMenu;
  QAction* m_disabledAction;
  QAction* m_introAction;
  QAction* m_basicAction;
  QAction* m_advancedAction;
  QAction* m_multiMonitorAction;
  QAction* m_openTutorialAction;
  QAction* m_openDocsAction;
  QAction* m_exitAction;
  QActionGroup* m_modeGroup;
#if LAUNCH_DOCS_IN_BROWSER
  QWebView* m_webView;
  QWidget* m_docsWidget;
#endif

};

#endif
