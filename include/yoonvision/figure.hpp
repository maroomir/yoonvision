#ifndef YOONVISION_FIGURE_HPP_
#define YOONVISION_FIGURE_HPP_

namespace yoonvision {
enum class Direction2D {
  kNone = -1,
  kCenter,
  kTopLeft,
  kTop,
  kTopRight,
  kRight,
  kBottomRight,
  kBottom,
  kBottomLeft,
  kLeft,
};

template <typename T>
struct Point2d {
  T x;
  T y;
};

template <typename T>
struct Point3d {
  T x;
  T y;
  T z;
};

using Point2dI = Point2d<int>;
using Point2dF = Point2d<float>;
using Point3dI = Point3d<int>;
using Point3dF = Point3d<float>;

}  // namespace yoonvision

#endif  // YOONVISION_FIGURE_HPP_
