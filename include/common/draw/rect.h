/*  Copyright (C) 2014-2021 FastoGT. All right reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following disclaimer
    in the documentation and/or other materials provided with the
    distribution.
        * Neither the name of FastoGT. nor the names of its
    contributors may be used to endorse or promote products derived from
    this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
    A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include <common/draw/point.h>
#include <common/draw/size.h>
#include <common/draw/vector2d.h>
#include <common/numerics/checked_math.h>

#if defined(OS_WIN)
typedef struct tagRECT RECT;
#elif defined(OS_MACOSX) || defined(OS_IOS)
typedef struct CGRect CGRect;
#endif

namespace common {
namespace draw {

class Rect {
 public:
  constexpr Rect() = default;
  constexpr Rect(int width, int height) : size_(width, height) {}
  constexpr Rect(int x, int y, int width, int height)
      : origin_(x, y), size_(GetClampedValue(x, width), GetClampedValue(y, height)) {}
  constexpr explicit Rect(const Size& size) : size_(size) {}
  constexpr Rect(const Point& origin, const Size& size)
      : origin_(origin), size_(GetClampedValue(origin.x(), size.width()), GetClampedValue(origin.y(), size.height())) {}

#if defined(OS_WIN)
  explicit Rect(const RECT& r);
#elif defined(OS_MACOSX) || defined(OS_IOS)
  explicit Rect(const CGRect& r);
#endif

#if defined(OS_WIN)
  // Construct an equivalent Win32 RECT object.
  RECT ToRECT() const;
#elif defined(OS_MACOSX) || defined(OS_IOS)
  // Construct an equivalent CoreGraphics object.
  CGRect ToCGRect() const;
#endif

  constexpr int x() const { return origin_.x(); }
  // Sets the X position while preserving the width.
  void set_x(int x) {
    origin_.set_x(x);
    size_.set_width(GetClampedValue(x, width()));
  }

  constexpr int y() const { return origin_.y(); }
  // Sets the Y position while preserving the height.
  void set_y(int y) {
    origin_.set_y(y);
    size_.set_height(GetClampedValue(y, height()));
  }

  constexpr int width() const { return size_.width(); }
  void set_width(int width) { size_.set_width(GetClampedValue(x(), width)); }

  constexpr int height() const { return size_.height(); }
  void set_height(int height) { size_.set_height(GetClampedValue(y(), height)); }

  constexpr const Point& origin() const { return origin_; }
  void set_origin(const Point& origin) {
    origin_ = origin;
    // Ensure that width and height remain valid.
    set_width(width());
    set_height(height());
  }

  constexpr const Size& size() const { return size_; }
  void set_size(const Size& size) {
    set_width(size.width());
    set_height(size.height());
  }

  constexpr int right() const { return x() + width(); }
  constexpr int bottom() const { return y() + height(); }

  constexpr Point top_right() const { return Point(right(), y()); }
  constexpr Point bottom_left() const { return Point(x(), bottom()); }
  constexpr Point bottom_right() const { return Point(right(), bottom()); }

  constexpr Point left_center() const { return Point(x(), y() + height() / 2); }
  constexpr Point top_center() const { return Point(x() + width() / 2, y()); }
  constexpr Point right_center() const { return Point(right(), y() + height() / 2); }
  constexpr Point bottom_center() const { return Point(x() + width() / 2, bottom()); }

  Vector2d OffsetFromOrigin() const { return Vector2d(x(), y()); }

  void SetRect(int x, int y, int width, int height) {
    origin_.SetPoint(x, y);
    // Ensure that width and height remain valid.
    set_width(width);
    set_height(height);
  }

  // Use in place of SetRect() when you know the edges of the rectangle instead
  // of the dimensions, rather than trying to determine the width/height
  // yourself. This safely handles cases where the width/height would overflow.
  void SetByBounds(int left, int top, int right, int bottom);

  // Shrink the rectangle by a horizontal and vertical distance on all sides.
  void Inset(int horizontal, int vertical) { Inset(horizontal, vertical, horizontal, vertical); }

  // Shrink the rectangle by the specified amount on each side.
  void Inset(int left, int top, int right, int bottom);

  // Move the rectangle by a horizontal and vertical distance.
  void Offset(int horizontal, int vertical);
  void Offset(const Vector2d& distance) { Offset(distance.x(), distance.y()); }
  void operator+=(const Vector2d& offset);
  void operator-=(const Vector2d& offset);

  // Returns true if the area of the rectangle is zero.
  bool IsEmpty() const { return size_.IsEmpty(); }

  // A rect is less than another rect if its origin is less than
  // the other rect's origin. If the origins are equal, then the
  // shortest rect is less than the other. If the origin and the
  // height are equal, then the narrowest rect is less than.
  // This comparison is required to use Rects in sets, or sorted
  // vectors.
  bool operator<(const Rect& other) const;

  // Returns true if the point identified by point_x and point_y falls inside
  // this rectangle.  The point (x, y) is inside the rectangle, but the
  // point (x + width, y + height) is not.
  bool Contains(int point_x, int point_y) const;

  // Returns true if the specified point is contained by this rectangle.
  bool Contains(const Point& point) const { return Contains(point.x(), point.y()); }

  // Returns true if this rectangle contains the specified rectangle.
  bool Contains(const Rect& rect) const;

  // Returns true if this rectangle intersects the specified rectangle.
  // An empty rectangle doesn't intersect any rectangle.
  bool Intersects(const Rect& rect) const;

  // Computes the intersection of this rectangle with the given rectangle.
  void Intersect(const Rect& rect);

  // Computes the union of this rectangle with the given rectangle.  The union
  // is the smallest rectangle containing both rectangles.
  void Union(const Rect& rect);

  // Computes the rectangle resulting from subtracting |rect| from |*this|,
  // i.e. the bounding rect of |Region(*this) - Region(rect)|.
  void Subtract(const Rect& rect);

  // Fits as much of the receiving rectangle into the supplied rectangle as
  // possible, becoming the result. For example, if the receiver had
  // a x-location of 2 and a width of 4, and the supplied rectangle had
  // an x-location of 0 with a width of 5, the returned rectangle would have
  // an x-location of 1 with a width of 4.
  void AdjustToFit(const Rect& rect);

  // Returns the center of this rectangle.
  Point CenterPoint() const;

  // Becomes a rectangle that has the same center point but with a size capped
  // at given |size|.
  void ClampToCenteredSize(const Size& size);

  // Transpose x and y axis.
  void Transpose();

  // Splits |this| in two halves, |left_half| and |right_half|.
  void SplitVertically(Rect* left_half, Rect* right_half) const;

  // Returns true if this rectangle shares an entire edge (i.e., same width or
  // same height) with the given rectangle, and the rectangles do not overlap.
  bool SharesEdgeWith(const Rect& rect) const;

  // Returns the manhattan distance from the rect to the point. If the point is
  // inside the rect, returns 0.
  int ManhattanDistanceToPoint(const Point& point) const;

  // Returns the manhattan distance between the contents of this rect and the
  // contents of the given rect. That is, if the intersection of the two rects
  // is non-empty then the function returns 0. If the rects share a side, it
  // returns the smallest non-zero value appropriate for int.
  int ManhattanInternalDistance(const Rect& rect) const;

  std::string ToString() const;

  bool ApproximatelyEqual(const Rect& rect, int tolerance) const;

 private:
  Point origin_;
  Size size_;

  // Returns true iff a+b would overflow max int.
  static constexpr bool AddWouldOverflow(int a, int b) {
    // In this function, GCC tries to make optimizations that would only work if
    // max - a wouldn't overflow but it isn't smart enough to notice that a > 0.
    // So cast everything to unsigned to avoid this.  As it is guaranteed that
    // max - a and b are both already positive, the cast is a noop.
    //
    // This is intended to be: a > 0 && max - a < b
    return a > 0 && b > 0 && static_cast<unsigned>(std::numeric_limits<int>::max() - a) < static_cast<unsigned>(b);
  }

  // Clamp the size to avoid integer overflow in bottom() and right().
  // This returns the width given an origin and a width.
  // TODO(enne): this should probably use common::ClampAdd, but that
  // function is not a constexpr.
  static constexpr int GetClampedValue(int origin, int size) {
    return AddWouldOverflow(origin, size) ? std::numeric_limits<int>::max() - origin : size;
  }
};

inline bool operator==(const Rect& lhs, const Rect& rhs) {
  return lhs.origin() == rhs.origin() && lhs.size() == rhs.size();
}

inline bool operator!=(const Rect& lhs, const Rect& rhs) {
  return !(lhs == rhs);
}

Rect operator+(const Rect& lhs, const Vector2d& rhs);
Rect operator-(const Rect& lhs, const Vector2d& rhs);

inline Rect operator+(const Vector2d& lhs, const Rect& rhs) {
  return rhs + lhs;
}

Rect IntersectRects(const Rect& a, const Rect& b);
Rect UnionRects(const Rect& a, const Rect& b);
Rect SubtractRects(const Rect& a, const Rect& b);

// Constructs a rectangle with |p1| and |p2| as opposite corners.
//
// This could also be thought of as "the smallest rect that contains both
// points", except that we consider points on the right/bottom edges of the
// rect to be outside the rect.  So technically one or both points will not be
// contained within the rect, because they will appear on one of these edges.
Rect BoundingRect(const Point& p1, const Point& p2);

// Scales the rect and returns the enclosing rect.  Use this only the inputs are
// known to not overflow.  Use ScaleToEnclosingRectSafe if the inputs are
// unknown and need to use saturated math.
inline Rect ScaleToEnclosingRect(const Rect& rect, float x_scale, float y_scale) {
  if (x_scale == 1.f && y_scale == 1.f)
    return rect;
  // These next functions cast instead of using e.g. ToFlooredInt() because we
  // haven't checked to ensure that the clamping behavior of the helper
  // functions doesn't degrade performance, and callers shouldn't be passing
  // values that cause overflow anyway.
  DCHECK(common::IsValueInRangeForNumericType<int>(std::floor(rect.x() * x_scale)));
  DCHECK(common::IsValueInRangeForNumericType<int>(std::floor(rect.y() * y_scale)));
  DCHECK(common::IsValueInRangeForNumericType<int>(std::ceil(rect.right() * x_scale)));
  DCHECK(common::IsValueInRangeForNumericType<int>(std::ceil(rect.bottom() * y_scale)));
  int x = static_cast<int>(std::floor(rect.x() * x_scale));
  int y = static_cast<int>(std::floor(rect.y() * y_scale));
  int r = rect.width() == 0 ? x : static_cast<int>(std::ceil(rect.right() * x_scale));
  int b = rect.height() == 0 ? y : static_cast<int>(std::ceil(rect.bottom() * y_scale));
  return Rect(x, y, r - x, b - y);
}

inline Rect ScaleToEnclosingRect(const Rect& rect, float scale) {
  return ScaleToEnclosingRect(rect, scale, scale);
}

// ScaleToEnclosingRect but clamping instead of asserting if the resulting rect
// would overflow.
inline Rect ScaleToEnclosingRectSafe(const Rect& rect, float x_scale, float y_scale) {
  if (x_scale == 1.f && y_scale == 1.f)
    return rect;
  int x = common::saturated_cast<int>(std::floor(rect.x() * x_scale));
  int y = common::saturated_cast<int>(std::floor(rect.y() * y_scale));
  int w = common::saturated_cast<int>(std::ceil(rect.width() * x_scale));
  int h = common::saturated_cast<int>(std::ceil(rect.height() * y_scale));
  return Rect(x, y, w, h);
}

inline Rect ScaleToEnclosingRectSafe(const Rect& rect, float scale) {
  return ScaleToEnclosingRectSafe(rect, scale, scale);
}

inline Rect ScaleToEnclosedRect(const Rect& rect, float x_scale, float y_scale) {
  if (x_scale == 1.f && y_scale == 1.f)
    return rect;
  DCHECK(common::IsValueInRangeForNumericType<int>(std::ceil(rect.x() * x_scale)));
  DCHECK(common::IsValueInRangeForNumericType<int>(std::ceil(rect.y() * y_scale)));
  DCHECK(common::IsValueInRangeForNumericType<int>(std::floor(rect.right() * x_scale)));
  DCHECK(common::IsValueInRangeForNumericType<int>(std::floor(rect.bottom() * y_scale)));
  int x = static_cast<int>(std::ceil(rect.x() * x_scale));
  int y = static_cast<int>(std::ceil(rect.y() * y_scale));
  int r = rect.width() == 0 ? x : static_cast<int>(std::floor(rect.right() * x_scale));
  int b = rect.height() == 0 ? y : static_cast<int>(std::floor(rect.bottom() * y_scale));
  return Rect(x, y, r - x, b - y);
}

inline Rect ScaleToEnclosedRect(const Rect& rect, float scale) {
  return ScaleToEnclosedRect(rect, scale, scale);
}

// Scales |rect| by scaling its four corner points. If the corner points lie on
// non-integral coordinate after scaling, their values are rounded to the
// nearest integer.
// This is helpful during layout when relative positions of multiple gfx::Rect
// in a given coordinate space needs to be same after scaling as it was before
// scaling. ie. this gives a lossless relative positioning of rects.
inline Rect ScaleToRoundedRect(const Rect& rect, float x_scale, float y_scale) {
  if (x_scale == 1.f && y_scale == 1.f)
    return rect;

  DCHECK(common::IsValueInRangeForNumericType<int>(std::round(rect.x() * x_scale)));
  DCHECK(common::IsValueInRangeForNumericType<int>(std::round(rect.y() * y_scale)));
  DCHECK(common::IsValueInRangeForNumericType<int>(std::round(rect.right() * x_scale)));
  DCHECK(common::IsValueInRangeForNumericType<int>(std::round(rect.bottom() * y_scale)));

  int x = static_cast<int>(std::round(rect.x() * x_scale));
  int y = static_cast<int>(std::round(rect.y() * y_scale));
  int r = rect.width() == 0 ? x : static_cast<int>(std::round(rect.right() * x_scale));
  int b = rect.height() == 0 ? y : static_cast<int>(std::round(rect.bottom() * y_scale));

  return Rect(x, y, r - x, b - y);
}

inline Rect ScaleToRoundedRect(const Rect& rect, float scale) {
  return ScaleToRoundedRect(rect, scale, scale);
}

}  // namespace draw
}  // namespace common
