#include "stdafx.h"
#include "Heartbeat.h"

Heartbeat::Heartbeat(uint32_t timeout, const std::function<void()>& callback):
  m_Callback(callback),
  m_Timeout(timeout),
  m_vigilance(Active)
{
  m_Thread = boost::thread([this] () {this->loop();});
}

Heartbeat::~Heartbeat() {
  // Signal that the thread should terminate:
  boost::unique_lock<boost::mutex> lock(m_Mutex);

  m_vigilance = Terminated;
  m_Condition.notify_all();

  lock.unlock();

  // Wait for the thread to respond to its notification:
  m_Thread.join();
}

void Heartbeat::Stop() {
  boost::unique_lock<boost::mutex> lock(m_Mutex);

  if (m_vigilance > Active) {
    // OK, vigilance is above Active, we can drop down
    m_vigilance = Active;
    m_Condition.notify_all();
  }
}

bool Heartbeat::Start(void) {
  boost::unique_lock<boost::mutex> lock(m_Mutex);

  // Trivial return check:
  if (!m_Callback || m_vigilance != Active) {
    return false;
  }
  m_vigilance = Running;
  m_Condition.notify_all();

  return true;
}

bool Heartbeat::Start(const std::function<void()>& callback) {
  boost::unique_lock<boost::mutex> lock(m_Mutex);

  m_Callback = callback;
  lock.unlock();

  return Start();
}

bool Heartbeat::Restart() {
  boost::unique_lock<boost::mutex> lock(m_Mutex);

  // Trivial return check:
  if (m_vigilance < Running || !m_Callback) {
    return false;
  }
  // Spuriously signal our state condition:
  m_vigilance = Resetting;
  m_Condition.notify_all();

  return true;
}

void Heartbeat::loop() {
  auto notActiveLambda = [this] () {return m_vigilance != Active;};

  boost::unique_lock<boost::mutex> lock(m_Mutex);

  while (
    [&, this] () -> bool {
      // Wait on the condition until we are no longer in the "active" state:
      m_Condition.wait(lock, notActiveLambda);

      // Proceed iff we are above the Active vigilance state
      return m_vigilance >= Active;
    }()
  ) {
    while (
      [&, this] () -> bool {
        do {
          if (m_vigilance == Resetting) {
            m_vigilance = Running;
          }
          if (m_vigilance == Running) {
            m_Condition.wait_for(lock, boost::chrono::milliseconds(m_Timeout));
          }
        } while (m_vigilance == Resetting);
        return m_vigilance == Running; // Only continue while we're running:
      }()
    ) {
      // Atomically obtain a copy of the callback function so we can unsynchronize while calling it:
      std::function<void()> fn = m_Callback;

      lock.unlock();
      fn(); // Call the callback before waiting on anything:
      lock.lock();
    }
  }
}
