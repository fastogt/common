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

#include <common/draw/vector2d_f.h>

#include <cmath>

#include <common/sprintf.h>

namespace common {
namespace draw {

std::string Vector2dF::ToString() const {
  return common::MemSPrintf("[%f %f]", x_, y_);
}

bool Vector2dF::IsZero() const {
  return x_ == 0 && y_ == 0;
}

void Vector2dF::Add(const Vector2dF& other) {
  x_ += other.x_;
  y_ += other.y_;
}

void Vector2dF::Subtract(const Vector2dF& other) {
  x_ -= other.x_;
  y_ -= other.y_;
}

double Vector2dF::LengthSquared() const {
  return static_cast<double>(x_) * x_ + static_cast<double>(y_) * y_;
}

float Vector2dF::Length() const {
  return static_cast<float>(std::sqrt(LengthSquared()));
}

void Vector2dF::Scale(float x_scale, float y_scale) {
  x_ *= x_scale;
  y_ *= y_scale;
}

double CrossProduct(const Vector2dF& lhs, const Vector2dF& rhs) {
  return static_cast<double>(lhs.x()) * rhs.y() - static_cast<double>(lhs.y()) * rhs.x();
}

double DotProduct(const Vector2dF& lhs, const Vector2dF& rhs) {
  return static_cast<double>(lhs.x()) * rhs.x() + static_cast<double>(lhs.y()) * rhs.y();
}

Vector2dF ScaleVector2d(const Vector2dF& v, float x_scale, float y_scale) {
  Vector2dF scaled_v(v);
  scaled_v.Scale(x_scale, y_scale);
  return scaled_v;
}

}  // namespace draw
}  // namespace common
