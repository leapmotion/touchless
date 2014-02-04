/*==================================================================================================================

    Copyright (c) 2010 - 2012 Leap Motion. All rights reserved.

  The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
  protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
  strictly forbidden unless prior written permission is obtained from Leap Motion.

===================================================================================================================*/

#ifndef __BoundedQueue_h__
#define __BoundedQueue_h__

#include <cstdlib>
#include "common.h"

/// <summary>
/// Templated queue class with a maximum capacity
/// </summary>
/// <remarks>
/// This class uses an array to represent a circular queue.
///
/// Maintainers: Raffi
/// </remarks>

template <class T, size_t maxSize>
class BoundedQueue {

public:

  BoundedQueue() {
    Reset();
  }

  void Reset() {
    m_CurFront = 0;
    m_CurBack = 0;
    m_CurSize = 0;
  }

  bool Enqueue(const T& data) {
    if (m_CurSize == maxSize) {
      return false;
    }
    m_Data[m_CurBack] = data;
    m_CurBack = (m_CurBack+1) % maxSize;
    m_CurSize++;
    return true;
  }

  T Dequeue() {
    T data = m_Data[m_CurFront];
    m_CurFront = (m_CurFront+1) % maxSize;
    m_CurSize--;
    return data;
  }



  const T& PeekFront(size_t idxFromFront = 0) const {
    return m_Data[(m_CurFront+idxFromFront) % maxSize];
  }

  T& PeekFront(size_t idxFromFront = 0) {
    return m_Data[(m_CurFront+idxFromFront) % maxSize];
  }

  size_t Size() const {
    return m_CurSize;
  }

  bool IsEmpty() const {
    return (m_CurSize == 0);
  }

  bool IsFull() const {
    return (m_CurSize == maxSize);
  }

  const T& operator[](size_t idx) const {
    return m_Data[(m_CurFront+idx) % maxSize];
  }

  T& operator[](size_t idx) {
    return m_Data[(m_CurFront+idx) % maxSize];
  }

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

private:

  T         m_Data[maxSize];
  size_t    m_CurFront;
  size_t    m_CurBack;
  size_t    m_CurSize;

};

#endif
