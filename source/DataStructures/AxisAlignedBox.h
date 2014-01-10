/*==================================================================================================================

    Copyright (c) 2010 - 2012 Leap Motion. All rights reserved.

  The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
  protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
  strictly forbidden unless prior written permission is obtained from Leap Motion.

===================================================================================================================*/

/// <summary>
/// A simple axis-aligned bounding box
/// </summary>
/// <remarks>
/// This class holds an axis-aligned bounding box, as defined by two corner positions. Each component of the "max"
/// corner is greater than or equal to each component of the "min" corner, making it easy to calculate volume,
/// unions, intersections, etc.
///
/// Maintainers: Raffi
/// </remarks>

#ifndef __AxisAlignedBox_h__
#define __AxisAlignedBox_h__

#include "common.h"

template <int DIM>
class AxisAlignedBox {

public:

  typedef Eigen::Matrix<double, DIM, 1> Point;

  // constructors
  AxisAlignedBox() : m_Min(-Point::Ones()), m_Max(Point::Ones()) { }
  AxisAlignedBox(const Point& min, const Point& max) : m_Min(min), m_Max(max) { }

  // modifiers
  inline void SetBounds(const Point& min, const Point& max) {
    m_Min = min;
    m_Max = max;
  }
  inline void SetCenterSize(const Point& center, const Point& size) {
    m_Min = center - size/2.0;
    m_Max = center + size/2.0;
  }
  inline void Intersection(const AxisAlignedBox<DIM>& other) {
    m_Min = m_Min.cwiseMax(other.m_Min);
    m_Max = m_Max.cwiseMin(other.m_Max);
  }
  inline void Union(const AxisAlignedBox<DIM>& other) {
    m_Min = m_Min.cwiseMin(other.m_Min);
    m_Max = m_Max.cwiseMax(other.m_Max);
  }
  inline void Expand(const Point& pos) {
    m_Min = m_Min.cwiseMin(pos);
    m_Max = m_Max.cwiseMax(pos);
  }
  inline void Scale(const Point& scale) {
    Point center = Center();
    m_Min = (scale.cwiseProduct(m_Min - center)) + center;
    m_Max = (scale.cwiseProduct(m_Max - center)) + center;
  }

  // const getters
  inline bool IsValid() const { return (m_Max.array() >= m_Min.array()).all(); }
  inline bool Contains(const Point& pos) const { return (pos.array() >= m_Min.array()).all() && (pos.array() <= m_Max.array()).all(); }
  inline const Point& Min() const { return m_Min; }
  inline const Point& Max() const { return m_Max; }
  inline Point Center() const { return (m_Max + m_Min)/2.0; }
  inline Point Size() const { return m_Max - m_Min; }
  inline double Volume() const { return Size().prod(); }
  inline double DiagonalLength() const { return Size().norm(); }
  inline Point ClosestPoint(const Point& pos) const { return pos.cwiseMax(m_Min).cwiseMin(m_Max); }
  inline Point NormalizeClamp(const Point& pos) const { return (ClosestPoint(pos) - m_Min).cwiseQuotient(Size()); }
  inline Point Normalize(const Point& pos) const { return (pos - m_Min).cwiseQuotient(Size()); }

  // intersection tests
  bool SphereIntersect(const Point& pos, double radius) const {
    return (pos - ClosestPoint(pos)).squaredNorm() <= radius*radius;
  }
  bool RayIntersect(const Point& rayOrigin, const Point& rayDirection, double& t) const {
    Point lower = (m_Min - rayOrigin).cwiseQuotient(rayDirection);
    Point upper = (m_Max - rayOrigin).cwiseQuotient(rayDirection);
    double tmin = lower.cwiseMin(upper).maxCoeff();
    double tmax = lower.cwiseMax(upper).minCoeff();
    if (tmin < tmax) {
      if (tmin > 0 && tmin < t) { t = tmin; return true; }
      if (tmax > 0 && tmax < t) { t = tmax; return true; }
    }
    return false;
  }

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW

private:

  Point m_Min;
  Point m_Max;

};

#endif
