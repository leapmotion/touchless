/*==================================================================================================================

    Copyright (c) 2010 - 2013 Leap Motion. All rights reserved.

  The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
  protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
  strictly forbidden unless prior written permission is obtained from Leap Motion.

===================================================================================================================*/

#if !defined(_Touch_h_)
#define _Touch_h_

#include <set>
#include <stdint.h>

class TouchManager;

class Touch {
  public:
    enum State {
      STATE_UNDEFINED = 0,
      STATE_HOVER = 1,
      STATE_BEGIN = 2,
      STATE_UPDATE = 3,
      STATE_HOVER_END = 4,
      STATE_END = 5
    };

    Touch(unsigned int id, double x = 0, double y = 0, bool touching = true, uint32_t orientation = 90) :
      m_id(id), m_x(x), m_y(y), m_touching(touching), m_orientation(orientation), m_state(STATE_UNDEFINED) {}

    unsigned int id() const { return m_id; }
    double x() const { return m_x; }
    double y() const { return m_y; }
    uint32_t orientation() const { return m_orientation; }
    State state() const { return m_state; }
    bool touching() const { return m_touching; }
    void setPos(double x, double y) { m_x = x; m_y = y; }

    bool operator<(const Touch& rhs) const { return (m_id < rhs.m_id); }

  private:
    unsigned int m_id;
    double m_x;
    double m_y;
    uint32_t m_orientation;
    State m_state;
    bool m_touching;

    void setState(State state) { m_state = state; }

    friend class TouchManager;
};

#endif // _Touch_h_
