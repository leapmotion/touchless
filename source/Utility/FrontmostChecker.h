/*==================================================================================================================

    Copyright (c) 2010 - 2013 Leap Motion. All rights reserved.

  The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
  protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
  strictly forbidden unless prior written permission is obtained from Leap Motion.

===================================================================================================================*/

#if !defined(__FrontmostChecker_h__)
#define __FrontmostChecker_h__

#if _WIN32
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
typedef DWORD pid_t;
#else // apple or linux
#include <sys/sysctl.h>
#include <sys/types.h>
#include <unistd.h>
#endif

class FrontmostChecker {
  public:
    FrontmostChecker(pid_t pid = getpid());
    ~FrontmostChecker();

    bool IsFrontmost(pid_t frontmostPid);

  private:
#if _WIN32
    static pid_t getpid() { return GetCurrentProcessId(); }
#else // apple or linux
    char* m_buffer;
    size_t m_bufferLength;
#endif
    pid_t m_pid;
    pid_t m_frontmostPid;
    bool m_isFrontmost;
};

#endif // __FrontmostChecker_h__
