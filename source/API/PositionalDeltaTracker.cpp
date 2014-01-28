#include "stdafx.h"
#include "PositionalDeltaTracker.h"

using namespace Leap;

PositionalDeltaTracker::PositionalDeltaTracker(void) {
  clear();
}

void PositionalDeltaTracker::setPosition(int32_t handId, int32_t pointableId, const Vector &position) {
  // if this object is uninitialized, set everything and return.
  if (m_handId == -1 && m_pointableId == -1) {
    m_handId = handId;
    m_pointableId = pointableId;
    m_delta = Vector::zero();
    m_position = position;
    return;
  }

  // determine if a delta should be accumulated.
  bool accumulateDelta = false;
  if (handId == -1 || m_handId == -1) {
    // -1 for hand id matches everything, so don't accumulate delta.
  } else if (handId != m_handId) {
    accumulateDelta = true;
  } else {
    if (pointableId != m_pointableId) {
      accumulateDelta = true;
    } else {
      // the pointable ids match, so don't accumulate delta.
    }
  }

  // if a delta accumulation was indicated, do so.
  if (accumulateDelta) {
    m_delta = m_position + m_delta - position;
  }

  // update state variables
  m_handId = handId;
  m_pointableId = pointableId;
  m_position = position;
}

void PositionalDeltaTracker::clear(void) {
  m_handId = -1;
  m_pointableId = -1;
  m_delta = Vector::zero();
  m_position = Vector::zero();
}