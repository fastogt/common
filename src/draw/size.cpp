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

#include <common/draw/size.h>

#if defined(OS_WIN)
#include <windows.h>
#elif defined(OS_IOS)
#include <CoreGraphics/CoreGraphics.h>
#elif defined(OS_MACOSX)
#include <ApplicationServices/ApplicationServices.h>
#endif

#include <common/convert2string.h>
#include <common/draw/size_conversions.h>
#include <common/draw/size_f.h>
#include <common/numerics/clamped_math.h>
#include <common/numerics/safe_conversions_impl.h>
#include <common/sprintf.h>

namespace common {
namespace draw {

#if defined(OS_MACOSX) || defined(OS_IOS)
Size::Size(const CGSize& s) : width_(s.width < 0 ? 0 : s.width), height_(s.height < 0 ? 0 : s.height) {}

Size& Size::operator=(const CGSize& s) {
  set_width(s.width);
  set_height(s.height);
  return *this;
}
#endif

void Size::operator+=(const Size& size) {
  Enlarge(size.width(), size.height());
}

void Size::operator-=(const Size& size) {
  Enlarge(-size.width(), -size.height());
}

#if defined(OS_WIN)
SIZE Size::ToSIZE() const {
  SIZE s;
  s.cx = width();
  s.cy = height();
  return s;
}
#elif defined(OS_MACOSX) || defined(OS_IOS)
CGSize Size::ToCGSize() const {
  return CGSizeMake(width(), height());
}
#endif

int Size::GetArea() const {
  return GetCheckedArea().ValueOrDie();
}

common::CheckedNumeric<int> Size::GetCheckedArea() const {
  common::CheckedNumeric<int> checked_area = width();
  checked_area *= height();
  return checked_area;
}

void Size::Enlarge(int grow_width, int grow_height) {
  SetSize(common::ClampAdd(width(), grow_width), common::ClampAdd(height(), grow_height));
}

void Size::SetToMin(const Size& other) {
  width_ = width() <= other.width() ? width() : other.width();
  height_ = height() <= other.height() ? height() : other.height();
}

void Size::SetToMax(const Size& other) {
  width_ = width() >= other.width() ? width() : other.width();
  height_ = height() >= other.height() ? height() : other.height();
}

std::string Size::ToString() const {
  return common::ConvertToString(*this);
}

Size ScaleToCeiledSize(const Size& size, float x_scale, float y_scale) {
  if (x_scale == 1.f && y_scale == 1.f)
    return size;
  return ToCeiledSize(ScaleSize(SizeF(size), x_scale, y_scale));
}

Size ScaleToCeiledSize(const Size& size, float scale) {
  if (scale == 1.f)
    return size;
  return ToCeiledSize(ScaleSize(SizeF(size), scale, scale));
}

Size ScaleToFlooredSize(const Size& size, float x_scale, float y_scale) {
  if (x_scale == 1.f && y_scale == 1.f)
    return size;
  return ToFlooredSize(ScaleSize(SizeF(size), x_scale, y_scale));
}

Size ScaleToFlooredSize(const Size& size, float scale) {
  if (scale == 1.f)
    return size;
  return ToFlooredSize(ScaleSize(SizeF(size), scale, scale));
}

Size ScaleToRoundedSize(const Size& size, float x_scale, float y_scale) {
  if (x_scale == 1.f && y_scale == 1.f)
    return size;
  return ToRoundedSize(ScaleSize(SizeF(size), x_scale, y_scale));
}

Size ScaleToRoundedSize(const Size& size, float scale) {
  if (scale == 1.f)
    return size;
  return ToRoundedSize(ScaleSize(SizeF(size), scale, scale));
}

std::ostream& operator<<(std::ostream& out, const Size& size) {
  return out << size.ToString();
}

}  // namespace draw

std::string ConvertToString(const draw::Size& value) {
  return MemSPrintf("%dx%d", value.width(), value.height());
}

bool ConvertFromString(const std::string& from, draw::Size* out) {
  if (!out) {
    return false;
  }

  draw::Size res;
  size_t del = from.find_first_of('x');
  if (del != std::string::npos) {
    int lwidth;
    if (!ConvertFromString(from.substr(0, del), &lwidth)) {
      return false;
    }
    res.set_width(lwidth);

    int lheight;
    if (!ConvertFromString(from.substr(del + 1), &lheight)) {
      return false;
    }
    res.set_height(lheight);
  }

  *out = res;
  return true;
}
}  // namespace common
