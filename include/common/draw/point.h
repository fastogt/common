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

#include <iosfwd>
#include <string>
#include <tuple>

#include <common/draw/vector2d.h>
#include <common/macros.h>
#include <common/numerics/clamped_math.h>

#if defined(OS_WIN)
typedef unsigned long DWORD;
typedef struct tagPOINT POINT;
#elif defined(OS_MACOSX) || defined(OS_IOS)
typedef struct CGPoint CGPoint;
#endif

namespace common {
namespace draw {

// A point has an x and y coordinate.
class Point {
 public:
  constexpr Point() : x_(0), y_(0) {}
  constexpr Point(int x, int y) : x_(x), y_(y) {}
#if defined(OS_WIN)
  // |point| is a DWORD value that contains a coordinate.  The x-coordinate is
  // the low-order short and the y-coordinate is the high-order short.  This
  // value is commonly acquired from GetMessagePos/GetCursorPos.
  explicit Point(DWORD point);
  explicit Point(const POINT& point);
  Point& operator=(const POINT& point);
#elif defined(OS_MACOSX) || defined(OS_IOS)
  explicit Point(const CGPoint& point);
#endif

#if defined(OS_WIN)
  POINT ToPOINT() const;
#elif defined(OS_MACOSX) || defined(OS_IOS)
  CGPoint ToCGPoint() const;
#endif

  constexpr int x() const { return x_; }
  constexpr int y() const { return y_; }
  void set_x(int x) { x_ = x; }
  void set_y(int y) { y_ = y; }

  void SetPoint(int x, int y) {
    x_ = x;
    y_ = y;
  }

  void Offset(int delta_x, int delta_y) {
    x_ = common::ClampAdd(x_, delta_x);
    y_ = common::ClampAdd(y_, delta_y);
  }

  void operator+=(const Vector2d& vector) {
    x_ = common::ClampAdd(x_, vector.x());
    y_ = common::ClampAdd(y_, vector.y());
  }

  void operator-=(const Vector2d& vector) {
    x_ = common::ClampSub(x_, vector.x());
    y_ = common::ClampSub(y_, vector.y());
  }

  void SetToMin(const Point& other);
  void SetToMax(const Point& other);

  bool IsOrigin() const { return x_ == 0 && y_ == 0; }

  Vector2d OffsetFromOrigin() const { return Vector2d(x_, y_); }

  // A point is less than another point if its y-value is closer
  // to the origin. If the y-values are the same, then point with
  // the x-value closer to the origin is considered less than the
  // other.
  // This comparison is required to use Point in sets, or sorted
  // vectors.
  bool operator<(const Point& rhs) const { return std::tie(y_, x_) < std::tie(rhs.y_, rhs.x_); }

  // Returns a string representation of point.
  std::string ToString() const;

 private:
  int x_;
  int y_;
};

inline bool operator==(const Point& lhs, const Point& rhs) {
  return lhs.x() == rhs.x() && lhs.y() == rhs.y();
}

inline bool operator!=(const Point& lhs, const Point& rhs) {
  return !(lhs == rhs);
}

inline Point operator+(const Point& lhs, const Vector2d& rhs) {
  Point result(lhs);
  result += rhs;
  return result;
}

inline Point operator-(const Point& lhs, const Vector2d& rhs) {
  Point result(lhs);
  result -= rhs;
  return result;
}

inline Vector2d operator-(const Point& lhs, const Point& rhs) {
  return Vector2d(common::ClampSub(lhs.x(), rhs.x()), common::ClampSub(lhs.y(), rhs.y()));
}

inline Point PointAtOffsetFromOrigin(const Vector2d& offset_from_origin) {
  return Point(offset_from_origin.x(), offset_from_origin.y());
}

// This is declared here for use in gtest-based unit tests but is defined in
// the //ui/gfx:test_support target. Depend on that to use this in your unit
// test. This should not be used in production code - call ToString() instead.
void PrintTo(const Point& point, ::std::ostream* os);

// Helper methods to scale a gfx::Point to a new gfx::Point.
Point ScaleToCeiledPoint(const Point& point, float x_scale, float y_scale);
Point ScaleToCeiledPoint(const Point& point, float x_scale);
Point ScaleToFlooredPoint(const Point& point, float x_scale, float y_scale);
Point ScaleToFlooredPoint(const Point& point, float x_scale);
Point ScaleToRoundedPoint(const Point& point, float x_scale, float y_scale);
Point ScaleToRoundedPoint(const Point& point, float x_scale);

std::ostream& operator<<(std::ostream& out, const Point& point);
}  // namespace draw

std::string ConvertToString(const draw::Point& value);
bool ConvertFromString(const std::string& from, draw::Point* out);

}  // namespace common
