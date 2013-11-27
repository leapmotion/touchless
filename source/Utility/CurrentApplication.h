/*==================================================================================================================

    Copyright (c) 2010 - 2013 Leap Motion. All rights reserved.

  The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
  protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
  strictly forbidden unless prior written permission is obtained from Leap Motion.

===================================================================================================================*/

#if !defined(__CurrentApplication_h__)
#define __CurrentApplication_h__

#include "FrontmostChecker.h"
#include <string>

class CurrentApplication {
  public:
    CurrentApplication();
    ~CurrentApplication();

    pid_t pid() const { return m_pid; }
    const std::string& title() const { return m_title; }
    const std::string& app() const { return m_app; }

  private:
    pid_t m_pid;
    std::string m_title;
    std::string m_app;
};

#endif // __CurrentApplication_h__
