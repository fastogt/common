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

#include <limits>
#include <string>

#include <common/draw/size.h>

namespace common {
namespace draw {

// A floating version of gfx::Size.
class SizeF {
 public:
  constexpr SizeF() : width_(0.f), height_(0.f) {}
  constexpr SizeF(float width, float height) : width_(clamp(width)), height_(clamp(height)) {}

  constexpr explicit SizeF(const Size& size)
      : SizeF(static_cast<float>(size.width()), static_cast<float>(size.height())) {}

  constexpr float width() const { return width_; }
  constexpr float height() const { return height_; }

  void set_width(float width) { width_ = clamp(width); }
  void set_height(float height) { height_ = clamp(height); }

  float GetArea() const;

  void SetSize(float width, float height) {
    set_width(width);
    set_height(height);
  }

  void Enlarge(float grow_width, float grow_height);

  void SetToMin(const SizeF& other);
  void SetToMax(const SizeF& other);

  bool IsEmpty() const { return !width() || !height(); }

  void Scale(float scale) { Scale(scale, scale); }

  void Scale(float x_scale, float y_scale) { SetSize(width() * x_scale, height() * y_scale); }

  std::string ToString() const;

 private:
  static constexpr float kTrivial = 8.f * std::numeric_limits<float>::epsilon();

  static constexpr float clamp(float f) { return f > kTrivial ? f : 0.f; }

  float width_;
  float height_;
};

inline bool operator==(const SizeF& lhs, const SizeF& rhs) {
  return lhs.width() == rhs.width() && lhs.height() == rhs.height();
}

inline bool operator!=(const SizeF& lhs, const SizeF& rhs) {
  return !(lhs == rhs);
}

SizeF ScaleSize(const SizeF& p, float x_scale, float y_scale);

inline SizeF ScaleSize(const SizeF& p, float scale) {
  return ScaleSize(p, scale, scale);
}

std::ostream& operator<<(std::ostream& out, const SizeF& size);
}  // namespace draw

std::string ConvertToString(const draw::SizeF& value);
bool ConvertFromString(const std::string& from, draw::SizeF* out);
}  // namespace common
