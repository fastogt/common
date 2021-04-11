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
#include <common/draw/vector2d_f.h>

namespace common {
namespace draw {

// A floating version of gfx::Point.
class PointF {
 public:
  constexpr PointF() : x_(0.f), y_(0.f) {}
  constexpr PointF(float x, float y) : x_(x), y_(y) {}

  constexpr explicit PointF(const Point& p) : PointF(static_cast<float>(p.x()), static_cast<float>(p.y())) {}

  constexpr float x() const { return x_; }
  constexpr float y() const { return y_; }
  void set_x(float x) { x_ = x; }
  void set_y(float y) { y_ = y; }

  void SetPoint(float x, float y) {
    x_ = x;
    y_ = y;
  }

  void Offset(float delta_x, float delta_y) {
    x_ += delta_x;
    y_ += delta_y;
  }

  void operator+=(const Vector2dF& vector) {
    x_ += vector.x();
    y_ += vector.y();
  }

  void operator-=(const Vector2dF& vector) {
    x_ -= vector.x();
    y_ -= vector.y();
  }

  void SetToMin(const PointF& other);
  void SetToMax(const PointF& other);

  bool IsOrigin() const { return x_ == 0 && y_ == 0; }

  Vector2dF OffsetFromOrigin() const { return Vector2dF(x_, y_); }

  // A point is less than another point if its y-value is closer
  // to the origin. If the y-values are the same, then point with
  // the x-value closer to the origin is considered less than the
  // other.
  // This comparison is required to use PointF in sets, or sorted
  // vectors.
  bool operator<(const PointF& rhs) const { return std::tie(y_, x_) < std::tie(rhs.y_, rhs.x_); }

  void Scale(float scale) { Scale(scale, scale); }

  void Scale(float x_scale, float y_scale) { SetPoint(x() * x_scale, y() * y_scale); }

  // Returns a string representation of point.
  std::string ToString() const;

 private:
  float x_;
  float y_;
};

inline bool operator==(const PointF& lhs, const PointF& rhs) {
  return lhs.x() == rhs.x() && lhs.y() == rhs.y();
}

inline bool operator!=(const PointF& lhs, const PointF& rhs) {
  return !(lhs == rhs);
}

inline PointF operator+(const PointF& lhs, const Vector2dF& rhs) {
  PointF result(lhs);
  result += rhs;
  return result;
}

inline PointF operator-(const PointF& lhs, const Vector2dF& rhs) {
  PointF result(lhs);
  result -= rhs;
  return result;
}

inline Vector2dF operator-(const PointF& lhs, const PointF& rhs) {
  return Vector2dF(lhs.x() - rhs.x(), lhs.y() - rhs.y());
}

inline PointF PointAtOffsetFromOrigin(const Vector2dF& offset_from_origin) {
  return PointF(offset_from_origin.x(), offset_from_origin.y());
}

PointF ScalePoint(const PointF& p, float x_scale, float y_scale);

inline PointF ScalePoint(const PointF& p, float scale) {
  return ScalePoint(p, scale, scale);
}

std::ostream& operator<<(std::ostream& out, const PointF& point);
}  // namespace draw

std::string ConvertToString(const draw::PointF& value);
bool ConvertFromString(const std::string& from, draw::PointF* out);

}  // namespace common
