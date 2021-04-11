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

#include <common/draw/size_f.h>

#include <common/convert2string.h>
#include <common/numerics/clamped_math.h>
#include <common/sprintf.h>

namespace common {
namespace draw {

float SizeF::GetArea() const {
  return width() * height();
}

void SizeF::Enlarge(float grow_width, float grow_height) {
  SetSize(width() + grow_width, height() + grow_height);
}

void SizeF::SetToMin(const SizeF& other) {
  width_ = width() <= other.width() ? width() : other.width();
  height_ = height() <= other.height() ? height() : other.height();
}

void SizeF::SetToMax(const SizeF& other) {
  width_ = width() >= other.width() ? width() : other.width();
  height_ = height() >= other.height() ? height() : other.height();
}

std::string SizeF::ToString() const {
  return ConvertToString(*this);
}

SizeF ScaleSize(const SizeF& s, float x_scale, float y_scale) {
  SizeF scaled_s(s);
  scaled_s.Scale(x_scale, y_scale);
  return scaled_s;
}

std::ostream& operator<<(std::ostream& out, const SizeF& size) {
  return out << size.ToString();
}

}  // namespace draw

std::string ConvertToString(const draw::SizeF& value) {
  return MemSPrintf("%fx%f", value.width(), value.height());
}

bool ConvertFromString(const std::string& from, draw::SizeF* out) {
  if (!out) {
    return false;
  }

  draw::SizeF res;
  size_t del = from.find_first_of('x');
  if (del != std::string::npos) {
    float lwidth;
    if (!ConvertFromString(from.substr(0, del), &lwidth)) {
      return false;
    }
    res.set_width(lwidth);

    float lheight;
    if (!ConvertFromString(from.substr(del + 1), &lheight)) {
      return false;
    }
    res.set_height(lheight);
  }

  *out = res;
  return true;
}
}  // namespace common
