/*==================================================================================================================

    Copyright (c) 2010 - 2013 Leap Motion. All rights reserved.

  The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
  protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
  strictly forbidden unless prior written permission is obtained from Leap Motion.

===================================================================================================================*/
#include "stdafx.h"
#include "CurrentApplication.h"

#if __APPLE__
#include <NSData.h>
#include <NSRunningApplication.h>
#include <NSURL.h>
#include <mach-o/dyld.h>
#include <sys/syslimits.h>
#include <stdlib.h>
#elif _WIN32
#include "DataStructures/Value.h"
#endif

CurrentApplication::CurrentApplication() : m_pid(0)
{
#if __APPLE__
  @autoreleasepool {
    NSRunningApplication* runningApplication = [NSRunningApplication currentApplication];
    m_pid = [runningApplication processIdentifier];
    if (m_pid == static_cast<pid_t>(-1)) {
      m_pid = getpid();
      const char* progname = getprogname();
      if (progname) {
        m_title = std::string(progname);
      }
      char path[PATH_MAX] = {0};
      uint32_t pathSize = sizeof(path);
      if (!_NSGetExecutablePath(path, &pathSize)) {
        char fullpath[PATH_MAX] = {0};
        if (realpath(path, fullpath)) {
          m_app = std::string(fullpath);
        }
      }
    } else {
      NSData* title = [[runningApplication localizedName] dataUsingEncoding:NSUTF8StringEncoding];
      if (title) {
        m_title = std::string(reinterpret_cast<const char*>([title bytes]), [title length]/sizeof(char));
      }
      NSData* app = [[[[runningApplication executableURL] filePathURL] path] dataUsingEncoding:NSUTF8StringEncoding];
      if (app) {
        m_app = std::string(reinterpret_cast<const char*>([app bytes]), [app length]/sizeof(char));
      }
    }
  }
#elif _WIN32
  m_pid = GetCurrentProcessId();
  WCHAR strPath[MAX_PATH] = { 0 };
  GetModuleFileNameW(NULL, strPath, ARRAYSIZE(strPath));
  m_app = Value::convertWideStringToUTF8String( strPath );
#else
  m_pid = getpid();
  // Linux -- FIXME
#endif
}

CurrentApplication::~CurrentApplication()
{
}
