#pragma once
#include "Leap.h"
#include <cassert>

namespace Leap {

/// <summary>
/// Utility stabilization class
/// </summary>
/// <remarks>
/// This is for removing discontinuities in cursor/overlay position resulting
/// from switching between pointable and palm positions for setting the cursor/
/// overlay position.  Positional deltas are accumulated so that the output
/// position is continuous.  There should be one of these objects per tracked
/// hand, and tracking 2+ hands will require additional logic to determine which
/// one of these objects corresponds to which hand.
/// </summary>
class PositionalDeltaTracker {
public:
  PositionalDeltaTracker(void);

private:
  /// <summary>
  /// Updates a given hand with the specified coordinates
  /// </summary>
  /// <remarks>
  /// A hand id of -1 is considered to be equal to any hand id.  Otherwise, different
  /// ids are considered to belong to different hands/pointables.  If pointableId is
  /// specified, then position is understood to be the pointable position, otherwise
  /// it is the hand position.  Specifying -1 for both handId and pointableId is not
  /// allowed.
  ///
  /// Different possible transitions:
  ///
  /// - If the hand id is different, then the delta is cleared, and the new position
  ///   is used directly.
  /// - Otherwise the hand id is the same.
  ///   * If the pointable id is the same, no delta is accumulated.
  ///   * Otherwise, the pointable id changed, so a delta is accumulated.
  /// </remarks>
  void setPosition(int32_t handId, int32_t pointableId, const Vector &position);

  int32_t                   m_handId;
  int32_t                   m_pointableId;
  Vector                    m_delta;
  Vector                    m_position;

public:
  Vector getTrackedPosition(void) const { return m_position + m_delta; }

  void setPositionToStabilizedPositionOf(const Hand &hand) {
    assert(hand.isValid());
    setPosition(hand.id(), -1, hand.stabilizedPalmPosition());
  }
  void setPositionToStabilizedPositionOf(const Pointable &pointable) {
    assert(pointable.isValid());
    setPosition(pointable.hand().id(), pointable.id(), pointable.stabilizedTipPosition());
  }

  void clear(void);
};

}