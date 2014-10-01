#pragma once
#include <set>
#include <stdint.h>

class TouchManager;

namespace Touchless {
class Touch {
public:
  Touch(uint32_t id, uint32_t frameId, double x, double y, bool touching):
    m_id(id),
    m_frameId(frameId),
    m_x(x),
    m_y(y),
    m_touching(touching) {
  }

  uint32_t id() const { return m_id; }
  uint32_t frameId() const { return m_frameId; }
  double x() const { return m_x; }
  double y() const { return m_y; }
  bool touching() const { return m_touching; }

  void setPos(double x, double y) { m_x = x; m_y = y; }
  void setId(uint32_t id) { m_id = id; }

  /// <summary>
  /// Comparator override, for use in ordered sets
  /// </summary>
  bool operator<(const Touch& rhs) const { return m_id < rhs.m_id; }

  /// <summary>
  /// Hash override, for use in an unordered set
  /// </summary>
  operator size_t(void) const {return m_id;}

private:
  uint32_t m_id;
  uint32_t m_frameId;
  double m_x;
  double m_y;
  bool m_touching;
};

//
// Public Interface
//
class TouchEvent:
  public std::set<Touch>
{
public:
  void clearTouchPoints(void) {
    clear();
  }

  void removeTouchPoint(int touchId) {
    Touch touch(*begin());
    touch.setId(touchId);
    erase(touch);
  }

  void addTouchPoint(int touchId, int frameId, float x, float y, bool touching) {
    insert(Touch(touchId, frameId, x, y, touching));
  }
};

}

#ifndef _MSC_VER
namespace std {
  namespace tr1 {
    template<class T>
    struct hash;

    template<>
    struct hash<Touchless::Touch> {
      size_t operator()(const Touchless::Touch& rhs) const {
        return rhs;
      }
    };
  }
}
#endif
