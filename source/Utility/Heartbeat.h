#if !defined(__Heartbeat_h__)
#define __Heartbeat_h__
#include "common.h"
#include <boost/thread/condition_variable.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/thread/thread.hpp>
#include FUNCTIONAL_HEADER

#if defined(_MSC_VER) && (_MSC_VER < 1600)
// Visual Studio 2008
typedef unsigned __int32 uint32_t;
#endif

/// <summary>
/// Class for periodically calling a callback
/// </summary>
/// <remarks>
/// This is a class for calling a callback at a specified periodic interval until the objects goes out of scope.
/// </remarks>
class Heartbeat {
public:
  Heartbeat(uint32_t timeout, const std::function<void()>& callback = [] () {});
  ~Heartbeat();

private:
  void loop();

  std::function<void()> m_Callback;
  boost::thread m_Thread;
  boost::mutex m_Mutex;
  boost::condition_variable m_Condition;
  uint32_t m_Timeout;

  // Vigilance states in ascending order:
  enum eVigilance {
    Terminated,
    Active,
    Running,
    Resetting
  };

  volatile eVigilance m_vigilance;

public:
  // Mutator methods:
  void SetTimeout(uint32_t timeout) {m_Timeout = timeout;}

  void Stop();

  /// <summary>
  /// Begins pulsing the internal callback at the timeout rate
  /// </summary>
  bool Start(void);

  /// <summary>
  /// Starts the Heartbeat with the passed callback function
  /// </summary>
  bool Start(const std::function<void()>& callback);

  /// <summary>
  /// Immediately signals the heartbeat to invoke the callback and restart its wait
  /// </summary>
  bool Restart();
};

#endif // __Heartbeat_h__
