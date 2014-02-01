#if !defined(_Touch_h_)
#define _Touch_h_

#include <set>
#include <stdint.h>

class TouchManager;

namespace Leap {
class Touch {
public:
  Touch(unsigned int id, double x = 0, double y = 0, bool touching = true, uint32_t orientation = 90):
    m_id(id),
    m_x(x),
    m_y(y),
    m_touching(touching),
    m_orientation(orientation)
  {
  }

  unsigned int id() const { return m_id; }
  double x() const { return m_x; }
  double y() const { return m_y; }
  uint32_t orientation() const { return m_orientation; }
  bool touching() const { return m_touching; }
  void setPos(double x, double y) { m_x = x; m_y = y; }

  /// <summary>
  /// Comparator override, for use in ordered sets
  /// </summary>
  bool operator<(const Touch& rhs) const { return m_id < rhs.m_id; }

  /// <summary>
  /// Hash override, for use in an unordered set
  /// </summary>
  operator size_t(void) const {return m_id;}

private:
  unsigned int m_id;
  double m_x;
  double m_y;
  uint32_t m_orientation;
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
    Touch touch(touchId);
    erase(touch);
  }

  void addTouchPoint(int touchId, float x, float y, bool touching) {
    insert(Touch(touchId, x, y, touching));
  }
};

}

#ifndef _MSC_VER
namespace std {
  namespace tr1 {
    template<class T>
    struct hash;

    template<>
    struct hash<Leap::Touch> {
      size_t operator()(const Leap::Touch& rhs) const {
        return rhs;
      }
    };
  }
}
#endif

#endif // _Touch_h_
