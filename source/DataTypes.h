/*==================================================================================================================

    Copyright (c) 2010 - 2013 Leap Motion. All rights reserved.

  The intellectual and technical concepts contained herein are proprietary and confidential to Leap Motion, and are
  protected by trade secret or copyright law. Dissemination of this information or reproduction of this material is
  strictly forbidden unless prior written permission is obtained from Leap Motion.

===================================================================================================================*/

#ifndef __DataTypes_h__
#define __DataTypes_h__

#include "ocuConfig.h"
#include "ocuType.h"

#ifdef __APPLE__
#include RVALUE_HEADER
#endif

// Define integer types for Visual Studio 2008 and earlier
#if defined(_MSC_VER) && (_MSC_VER < 1600)
typedef __int32 int32_t;
typedef __int64 int64_t;
typedef unsigned __int32 uint32_t;
typedef unsigned __int64 uint64_t;
#else
#include <stdint.h>
#endif

#ifdef _WIN64
  // warning C4244: 'argument' : conversion from '__int64' to 'int', possible loss of data
  #pragma warning(push)
  #pragma warning(disable: 4244)
#endif

#if !_WIN32
  #pragma GCC diagnostic push
  #pragma GCC diagnostic ignored "-Wunused-variable"
#endif

#include <Eigen/Dense>

#if !_WIN32
  #pragma GCC diagnostic pop
#endif

//Standard Library
#include <vector>
#include <Eigen/StdVector>
#ifdef _WIN64
  #pragma warning(pop)
#endif

static const int POINT_SIZE_2D = 2; // x y
static const int POINT_SIZE_3D = 3; // x y z
static const int POINT_SIZE_4D = 4; // x y z id

typedef double MATH_TYPE;

// geometry storage types
typedef Eigen::Matrix<MATH_TYPE, 1, POINT_SIZE_2D> Point2D;
typedef Eigen::Matrix<MATH_TYPE, 1, POINT_SIZE_3D> Point3D;
typedef Eigen::Matrix<MATH_TYPE, 1, POINT_SIZE_4D> Point4D;
typedef std::vector<Point2D, Eigen::aligned_allocator<Point2D> > stdvectorPoint2D;

// matrices
typedef Eigen::Matrix<MATH_TYPE, 1, 1> Matrix1x1;
typedef Eigen::Matrix<MATH_TYPE, 2, 2> Matrix2x2;
typedef Eigen::Matrix<MATH_TYPE, 2, 3> Matrix2x3;
typedef Eigen::Matrix<MATH_TYPE, 3, 3> Matrix3x3;
typedef Eigen::Matrix<MATH_TYPE, 3, 2> Matrix3x2;
typedef Eigen::Matrix<MATH_TYPE, 4, 4> Matrix4x4;
typedef Eigen::Matrix<MATH_TYPE, Eigen::Dynamic, Eigen::Dynamic> MatrixD;
typedef Eigen::Matrix<MATH_TYPE, 3, 3, Eigen::RowMajor> CvMatrix3x3;
typedef Eigen::Matrix<MATH_TYPE, 3, 4, Eigen::RowMajor> CvMatrix3x4;
typedef Eigen::Matrix<MATH_TYPE, 4, 4, Eigen::RowMajor> CvMatrix4x4;

// vectors
typedef Eigen::Matrix<MATH_TYPE, 1, 1> Vector1;
typedef Eigen::Matrix<MATH_TYPE, 2, 1> Vector2;
typedef Eigen::Matrix<MATH_TYPE, 3, 1> Vector3;
typedef Eigen::Matrix<MATH_TYPE, 4, 1> Vector4;
typedef Eigen::Matrix<MATH_TYPE, 5, 1> Vector5;
typedef Eigen::Matrix<MATH_TYPE, 6, 1> Vector6;
typedef Eigen::Matrix<MATH_TYPE, 7, 1> Vector7;
typedef Eigen::Matrix<MATH_TYPE, 8, 1> Vector8;
typedef Eigen::Matrix<MATH_TYPE, 9, 1> Vector9;
typedef Eigen::Matrix<MATH_TYPE, 10, 1> Vector10;
typedef Eigen::Matrix<MATH_TYPE, Eigen::Dynamic, 1> VectorD;

// arrays
typedef Eigen::Array<MATH_TYPE, 2, 1> Array2;
typedef Eigen::Array<MATH_TYPE, 3, 1> Array3;
typedef Eigen::Array<MATH_TYPE, 4, 1> Array4;
typedef Eigen::Array<MATH_TYPE, 5, 1> Array5;
typedef Eigen::Array<MATH_TYPE, 6, 1> Array6;
typedef Eigen::Array<MATH_TYPE, 7, 1> Array7;
typedef Eigen::Array<MATH_TYPE, 8, 1> Array8;
typedef Eigen::Array<MATH_TYPE, 9, 1> Array9;
typedef Eigen::Array<MATH_TYPE, 10, 1> Array10;
typedef Eigen::Array<MATH_TYPE, Eigen::Dynamic, 1> ArrayD;
// standard library containers
typedef std::vector<Vector2, Eigen::aligned_allocator<Vector2> > stdvectorV2;
typedef std::vector<Vector3, Eigen::aligned_allocator<Vector3> > stdvectorV3;

struct Ray {
  Ray(const Vector3& _position, const Vector3& _direction) : position(_position), direction(_direction.normalized()) {}
  Vector3 position;
  Vector3 direction;
  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
};

struct SnapshotStatistics {
  double mean, std, framerate;
};

struct PeakPoint {
  Vector3 point;
  int type;
  float wedge;
  float scanline;
};

struct Cluster {
  inline operator double() const { return point.z(); }
  Vector3 point;
  int label;
  int contourID;
  double accumWeight;
  Vector3 accumPos;
  FrameCounterType lastUpdateFrame;
};

typedef Eigen::aligned_allocator<PeakPoint> PeakPointAllocator;
typedef std::vector<PeakPoint, PeakPointAllocator> PeakPointVector;
typedef Eigen::aligned_allocator<Cluster> ClusterAllocator;
typedef std::vector<Cluster, ClusterAllocator> ClusterVector;

#endif //__DataTypes_h__
