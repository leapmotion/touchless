/*==================================================================================================================

    Copyright (c) 2010 - 2013 Leap Motion. All rights reserved.

  The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
  protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
  strictly forbidden unless prior written permission is obtained from Leap Motion.

===================================================================================================================*/
#ifndef __TimedHistory_h__
#define __TimedHistory_h__

#include <cassert>
#include <deque>

/*
design criteria for a buffer containing at least X seconds of history
- the number of frames stored by this object is irrelevant
- because when the object is initialized it doesn't have any history,
  there should be a flag indicating "has enough history"
- there should be a "clear history" method which also resets the flag
- the duration of history could (should) be run-time specifiable, and
  would affect the flag
- the underlying container would be a deque, and it may be useful to
  occasionally trim it in case it gets giant and then the number of
  frames required to store drops (so there is a bunch of unused storage)
- there needs to be a way to access all frames (maybe just direct const
  access to the underlying deque)
- there needs to be a way to access frames based on "X seconds ago"
  * probably accepting a duration of history and returning a range of frames
- there needs to be a way of adding frames (and updating the "current time")
  * this would automatically discard frames that are too old
  * maybe make it return a range of discarded frames which could be
    processed to save computation (e.g. so that the whole range of history
    doesn't have to be iterated over each frame)
- should provide STL-container-class-like interface methods when possible
*/

// Time can be specified independently, e.g. an unsigned type -- Duration should be signed.
template <typename Frame, typename Duration, typename Time = Duration>
class TimedHistory : private std::deque<std::pair<Time,Frame> > {
public:

  typedef std::deque<std::pair<Time,Frame> > TimeAndFramePairDeque;
  typedef std::pair<Time,Frame> TimeAndFramePair;

  TimedHistory (Duration historyDuration) : m_historyDuration(historyDuration) { }

  // expose a bunch of the STL container class types and methods
  typedef typename TimeAndFramePairDeque::size_type size_type;
  typedef typename TimeAndFramePairDeque::iterator iterator;
  typedef typename TimeAndFramePairDeque::const_iterator const_iterator;
  typedef typename TimeAndFramePairDeque::reverse_iterator reverse_iterator;
  typedef typename TimeAndFramePairDeque::const_reverse_iterator const_reverse_iterator;
  using TimeAndFramePairDeque::begin;
  using TimeAndFramePairDeque::end;
  using TimeAndFramePairDeque::rbegin;
  using TimeAndFramePairDeque::rend;
//   using TimeAndFramePairDeque::cbegin;
//   using TimeAndFramePairDeque::cend;
//   using TimeAndFramePairDeque::crbegin;
//   using TimeAndFramePairDeque::crend;
  using TimeAndFramePairDeque::size;
  using TimeAndFramePairDeque::empty;
//   using TimeAndFramePairDeque::shrink_to_fit;
  using TimeAndFramePairDeque::operator[];
  using TimeAndFramePairDeque::at;
  using TimeAndFramePairDeque::front;
  using TimeAndFramePairDeque::back;
  using TimeAndFramePairDeque::clear;

  Time oldestFrameTime () const {
    if (empty()) {
      return static_cast<Time>(0); // not really a sentinel value, just a dummy
    } else {
      return rbegin()->first;
    }
  }
  Time mostRecentFrameTime () const {
    if (empty()) {
      return static_cast<Time>(0); // not really a sentinel value, just a dummy
    } else {
      return begin()->first;
    }
  }
  bool hasSufficientHistory () const {
    if (empty()) {
      return false;
    } else {
      return static_cast<Duration>(begin()->first - rbegin()->first) >= m_historyDuration;
    }
  }
  const_iterator getFrameHavingAgeAtLeast (Duration age) const {
    // TODO: implement faster search algorithm
    const_iterator it = begin(); // begin is the most recent
    if (it == end())
      return it;

    Time mostRecentTime = it->first; // time of the most recent frame
    while (it != end() && static_cast<Duration>(mostRecentTime - it->first) < age) {
      ++it;
    }
    return it; // this may return end()
  }

  // frames must be added in nondecreasing time order
  void addFrame (Time time, const Frame &frame) {
    if (!empty()) {
      assert(time >= begin()->first);
    }
    this->push_front(TimeAndFramePair(time, frame));
    purgeExcessHistory();
  }
  //void addFrameAndReturnDiscards (const Frame &frame, TimeType frameTime, range-of-frames &X); // or return a deque/vector of frames
  //void cleanUpDiscards (); // necessary to call after addFrameAndReturnDiscards

  void setHistoryDuration (Duration historyDuration) {
    m_historyDuration = historyDuration;
    purgeExcessHistory();
  }
  // TODO: version of setHistoryDuration which returns discarded frames?

private:

  void purgeExcessHistory () {
    if (empty()) {
      return; // nothing to do
    }
    // ensure that, by looking at the second-to-oldest frame, we will have enough history
    // after popping -- repeat this process until it's no longer true.
    Time mostRecentTime = begin()->first;
    reverse_iterator it = rbegin();
    ++it;
    while (it != rend() && static_cast<Duration>(mostRecentTime - it->first) >= m_historyDuration) {
      ++it;
      this->pop_back();
    }
  }

  // the oldest frame will be at rbegin, the most recent will be at begin -- referring to the inherited std::deque

  // the amount of history to keep -- any frames past the first frame that comprise this
  // history duration will be discarded.
  Duration m_historyDuration;
};

#endif //__TimedHistory_h__
