// Copyright (c) 2010 - 2013 Leap Motion. All rights reserved. Proprietary and confidential.
#if defined(LEAP_OS_MAC)
  #define LAUNCH_DOCS_IN_BROWSER 0
#else
  #define LAUNCH_DOCS_IN_BROWSER 1
#endif

#include "TouchlessUI.h"
#include "Utility/FileSystemUtil.h"

#include "GestureInteractionManager.h"

#include <qapplication.h>
#include <qmenu.h>
#include <qaction.h>
#include <qactiongroup.h>
#if !__APPLE__
#include <qwebview.h>
#endif
#include <qpen.h>
#include <qpainter.h>
#include <qcolor.h>
#include <qboxlayout.h>
#include <qstyle.h>
#include <qdesktopwidget.h>
#include <qfile.h>
#include <qfileinfo.h>
#include <qurl.h>
#include <qdesktopservices.h>
#include <qwebframe.h>

#if __APPLE__
  #define EXIT_STRING "Quit"
#else
  #define EXIT_STRING "Exit"
#endif

#define SENTINEL_FILE_NAME "firstruntouchless.sentinel"

TouchlessUI::TouchlessUI() {
  m_trayIcon = new QSystemTrayIcon(this);
  m_trayIcon->setToolTip("Touchless");

  m_trayMenu = new QMenu(this);
  m_modeMenu = new QMenu(tr("Interaction"), this);
  connect(m_trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(trayIconActivatedSlot(QSystemTrayIcon::ActivationReason)));

  m_modeGroup = new QActionGroup(this);
  m_modeGroup->setEnabled(false);
#if __APPLE__
  // Workaround the problem of QActionGroup items not obeying setEnabled(false); just hide the who submenu.
  m_modeGroup->setVisible(false);
#endif
  m_modeGroup->setExclusive(true);

  // mode switching actions
  m_disabledAction = new QAction(tr("Disabled"), m_modeGroup);
  m_disabledAction->setCheckable(true);
  m_introAction = new QAction(tr("Intro"), m_modeGroup);
  m_introAction->setCheckable(true);
  m_basicAction = new QAction(tr("Basic"), m_modeGroup);
  m_basicAction->setCheckable(true);
  m_advancedAction = new QAction(tr("Advanced"), m_modeGroup);
  m_advancedAction->setCheckable(true);
  m_multiMonitorAction = new QAction(tr("Use Multiple Monitors"), this);
  m_multiMonitorAction->setCheckable(true);
  m_multiMonitorAction->setEnabled(false);
  m_openTutorialAction = new QAction(tr("Getting Started"), this);
  m_openDocsAction = new QAction(tr("Open Help Guide"), this);
  m_exitAction = new QAction(tr(EXIT_STRING), this);
  connect(m_disabledAction, SIGNAL(toggled(bool)), this, SLOT(disabledModeSlot(bool)));
  connect(m_introAction, SIGNAL(toggled(bool)), this, SLOT(introModeSlot(bool)));
  connect(m_basicAction, SIGNAL(toggled(bool)), this, SLOT(basicModeSlot(bool)));
  connect(m_advancedAction, SIGNAL(toggled(bool)), this, SLOT(advancedModeSlot(bool)));
  connect(m_multiMonitorAction, SIGNAL(toggled(bool)), this, SIGNAL(multiMonitorSignal(bool)));
  connect(m_openTutorialAction, SIGNAL(triggered()), this, SLOT(openTutorialSlot()));
  connect(m_openDocsAction, SIGNAL(triggered()), this, SLOT(openDocumentationSlot()));
  connect(m_exitAction, SIGNAL(triggered()), this, SLOT(exitSlot()));

#if !LAUNCH_DOCS_IN_BROWSER
  // create help guide
  m_docsWidget = new QWidget();
  QVBoxLayout* layout = new QVBoxLayout(m_docsWidget);
  m_webView = new QWebView();
  layout->addWidget(m_webView);

  m_webView->grabGesture(Qt::PanGesture);
  m_webView->grabGesture(Qt::PinchGesture);
  m_webView->grabGesture(Qt::SwipeGesture);
  m_docsWidget->grabGesture(Qt::PanGesture);
  m_docsWidget->grabGesture(Qt::PinchGesture);
  m_docsWidget->grabGesture(Qt::SwipeGesture);
  this->setAttribute(Qt::WA_AcceptTouchEvents, true);
  m_webView->setAttribute(Qt::WA_AcceptTouchEvents, true);
  m_docsWidget->setAttribute(Qt::WA_AcceptTouchEvents, true);

  m_webView->settings()->setAttribute(QWebSettings::PluginsEnabled, true);
  m_webView->settings()->setAttribute(QWebSettings::JavascriptEnabled, true);
  m_webView->settings()->setAttribute(QWebSettings::FrameFlatteningEnabled, true);
  m_webView->page()->mainFrame()->setScrollBarPolicy(Qt::Horizontal, Qt::ScrollBarAlwaysOff);
  m_docsWidget->hide();
#endif

  m_modeMenu->addAction(m_disabledAction);
#if __APPLE__
  m_modeMenu->addAction(m_introAction);
#endif
  m_modeMenu->addAction(m_basicAction);
  m_modeMenu->addAction(m_advancedAction);

  m_trayMenu->addMenu(m_modeMenu);
  m_trayMenu->addAction(m_openTutorialAction);
  m_trayMenu->addAction(m_openDocsAction);
  m_trayMenu->addAction(m_multiMonitorAction);
  m_trayMenu->addSeparator();
  m_trayMenu->addAction(m_exitAction);

  updateTrayIconSlot();
  m_trayIcon->setContextMenu(m_trayMenu);
  m_trayIcon->show();

  if (isFirstRun()) {
    openTutorialSlot();
    writeFirstRunSentinel();
#if _WIN32
    m_trayIcon->showMessage(tr("Welcome"), tr("Welcome to Touchless for Windows"));
#else
    m_trayIcon->showMessage(tr("Welcome"), tr("Welcome to Touchless for Mac"));
#endif
  }
}

void TouchlessUI::disabledModeSlot(bool enabled) {
  if (enabled) {
    m_trayIcon->setToolTip("Disabled");
    Q_EMIT(disabledModeSignal());
  }
}

void TouchlessUI::introModeSlot(bool enabled) {
  if (enabled) {
    m_trayIcon->setToolTip("Intro Mode");
    Q_EMIT(introModeSignal());
  }
}

void TouchlessUI::basicModeSlot(bool enabled) {
  if (enabled) {
    m_trayIcon->setToolTip("Basic Mode");
    Q_EMIT(basicModeSignal());
  }
}

void TouchlessUI::advancedModeSlot(bool enabled) {
  if (enabled) {
    m_trayIcon->setToolTip("Advanced Mode");
    Q_EMIT(advancedModeSignal());
  }
}

void TouchlessUI::exitSlot() {
  m_trayIcon->hide();
  qApp->quit();
}

void TouchlessUI::updateTrayIconSlot() {
  m_trayIcon->setIcon(GenerateTouchlessIcon());
}

QIcon TouchlessUI::GenerateTouchlessIcon() {
  QPixmap pixmap(16, 16);
  pixmap.fill(Qt::transparent);
  QPen pen;
  QPainter painter(&pixmap);
  painter.setRenderHint(QPainter::Antialiasing, true);
  QColor borderColor(40, 40, 40);
  QColor centerColor(230, 230, 230);
  pen.setWidthF(1.25);
  pen.setBrush(borderColor);
  painter.setPen(pen);
  painter.setBrush(QBrush(centerColor));

  painter.drawEllipse(QPointF(3.75, 8), 2.5, 2.5);
  painter.drawEllipse(QPointF(8, 6.5), 3.0, 3.0);
  painter.drawEllipse(QPointF(12, 9.5), 2.75, 2.75);

  return QIcon(pixmap);
}

void TouchlessUI::trayIconActivatedSlot(QSystemTrayIcon::ActivationReason reason) { }

void TouchlessUI::connectChangedSlot(bool connected, int mode, bool useMultiMonitors) {
  m_modeGroup->setEnabled(connected);
#if __APPLE__
  // Workaround the problem of QActionGroup items not obeying setEnabled(false); just hide the whole submenu.
  m_modeGroup->setVisible(connected);
#endif
  m_multiMonitorAction->setEnabled(connected);

  Touchless::GestureInteractionMode outputMode = static_cast<enum Touchless::GestureInteractionMode>(mode);

  QAction* action = 0;
  switch (outputMode) {
  case Touchless::GestureInteractionMode::OUTPUT_MODE_DISABLED: action = m_disabledAction; break;
  case Touchless::GestureInteractionMode::OUTPUT_MODE_INTRO:    action = m_introAction;    break;
  case Touchless::GestureInteractionMode::OUTPUT_MODE_BASIC:    action = m_basicAction;    break;
  case Touchless::GestureInteractionMode::OUTPUT_MODE_ADVANCED: action = m_advancedAction; break;
  default: break;
  }

  if (action) {
    action->setChecked(true);
  }

  m_multiMonitorAction->setChecked(useMultiMonitors);
}

bool TouchlessUI::isFirstRun() {
  return !FileSystemUtil::FileExists( FileSystemUtil::GetUserPath(SENTINEL_FILE_NAME) );
}

void TouchlessUI::writeFirstRunSentinel() {
  QFile sentinel( FileSystemUtil::GetUserPath(SENTINEL_FILE_NAME).c_str() );
  sentinel.open(QIODevice::WriteOnly);
  sentinel.write("datablob");
  sentinel.close();
}

void TouchlessUI::openTutorialSlot() {
#if _WIN32
  static const char DISK_TUTORIAL_PATH[] = "static-docs/slide1w.html";
#else
  static const char DISK_TUTORIAL_PATH[] = "static-docs/slide1m.html";
#endif

  QUrl tutorialUrl;
  QFileInfo tutorialFileInfo(DISK_TUTORIAL_PATH);
  if (tutorialFileInfo.exists()) {
    // first check to see if the on-disk tutorial is available
    tutorialUrl = QUrl::fromUserInput(tutorialFileInfo.absoluteFilePath());
  } else {
    return;
  }

#if LAUNCH_DOCS_IN_BROWSER
  // load in user's default web browser
  QDesktopServices::openUrl(tutorialUrl);
#else
  // load in QWebView
  m_webView->load(tutorialUrl);
  m_docsWidget->setWindowTitle(tr("Touchless Getting Started"));
  m_docsWidget->showMaximized();
#endif
}

void TouchlessUI::openDocumentationSlot() {
#if _WIN32
  static const char DISK_DOCUMENTATION_PATH[] = "static-docs/windows-basic.html";
#else
  static const char DISK_DOCUMENTATION_PATH[] = "static-docs/mac-basic.html";
#endif

  QUrl docsUrl;
  QFileInfo docsFileInfo(DISK_DOCUMENTATION_PATH);
  if (docsFileInfo.exists()) {
    // first check to see if the on-disk tutorial is available
    docsUrl = QUrl::fromUserInput(docsFileInfo.absoluteFilePath());
  } else {
    // on-disk tutorial not found
    return;
  }

#if LAUNCH_DOCS_IN_BROWSER
  // load in user's default web browser
  QDesktopServices::openUrl(docsUrl);
#else
  // load in QWebView
  m_webView->load(docsUrl);
  m_docsWidget->setWindowTitle(tr("Touchless Help Guide"));
  m_docsWidget->showMaximized();
#endif
}

void TouchlessUI::setMultipleMonitorsActionVisible(bool visible) {
  m_multiMonitorAction->setVisible(visible);
}
