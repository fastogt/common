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

#include <common/draw/point.h>

#if defined(OS_WIN)
#include <windows.h>
#elif defined(OS_IOS)
#include <CoreGraphics/CoreGraphics.h>
#elif defined(OS_MACOSX)
#include <ApplicationServices/ApplicationServices.h>
#endif

#include <common/convert2string.h>
#include <common/draw/point_conversions.h>
#include <common/draw/point_f.h>
#include <common/numerics/safe_conversions_impl.h>
#include <common/sprintf.h>

namespace common {
namespace draw {

#if defined(OS_WIN)
Point::Point(DWORD point) {
  POINTS points = MAKEPOINTS(point);
  x_ = points.x;
  y_ = points.y;
}

Point::Point(const POINT& point) : x_(point.x), y_(point.y) {}

Point& Point::operator=(const POINT& point) {
  x_ = point.x;
  y_ = point.y;
  return *this;
}
#elif defined(OS_MACOSX) || defined(OS_IOS)
Point::Point(const CGPoint& point) : x_(point.x), y_(point.y) {}
#endif

#if defined(OS_WIN)
POINT Point::ToPOINT() const {
  POINT p;
  p.x = x();
  p.y = y();
  return p;
}
#elif defined(OS_MACOSX) || defined(OS_IOS)
CGPoint Point::ToCGPoint() const {
  return CGPointMake(x(), y());
}
#endif

void Point::SetToMin(const Point& other) {
  x_ = x_ <= other.x_ ? x_ : other.x_;
  y_ = y_ <= other.y_ ? y_ : other.y_;
}

void Point::SetToMax(const Point& other) {
  x_ = x_ >= other.x_ ? x_ : other.x_;
  y_ = y_ >= other.y_ ? y_ : other.y_;
}

std::string Point::ToString() const {
  return ConvertToString(*this);
}

Point ScaleToCeiledPoint(const Point& point, float x_scale, float y_scale) {
  if (x_scale == 1.f && y_scale == 1.f)
    return point;
  return ToCeiledPoint(ScalePoint(PointF(point), x_scale, y_scale));
}

Point ScaleToCeiledPoint(const Point& point, float scale) {
  if (scale == 1.f)
    return point;
  return ToCeiledPoint(ScalePoint(PointF(point), scale, scale));
}

Point ScaleToFlooredPoint(const Point& point, float x_scale, float y_scale) {
  if (x_scale == 1.f && y_scale == 1.f)
    return point;
  return ToFlooredPoint(ScalePoint(PointF(point), x_scale, y_scale));
}

Point ScaleToFlooredPoint(const Point& point, float scale) {
  if (scale == 1.f)
    return point;
  return ToFlooredPoint(ScalePoint(PointF(point), scale, scale));
}

Point ScaleToRoundedPoint(const Point& point, float x_scale, float y_scale) {
  if (x_scale == 1.f && y_scale == 1.f)
    return point;
  return ToRoundedPoint(ScalePoint(PointF(point), x_scale, y_scale));
}

Point ScaleToRoundedPoint(const Point& point, float scale) {
  if (scale == 1.f)
    return point;
  return ToRoundedPoint(ScalePoint(PointF(point), scale, scale));
}

std::ostream& operator<<(std::ostream& out, const Point& point) {
  return out << point.ToString();
}

}  // namespace draw

std::string ConvertToString(const draw::Point& value) {
  return MemSPrintf("%d,%d", value.x(), value.y());
}

bool ConvertFromString(const std::string& from, draw::Point* out) {
  if (!out) {
    return false;
  }

  draw::Point res;
  size_t del = from.find_first_of(',');
  if (del != std::string::npos) {
    int lx;
    if (!ConvertFromString(from.substr(0, del), &lx)) {
      return false;
    }
    res.set_x(lx);

    int ly;
    if (!ConvertFromString(from.substr(del + 1), &ly)) {
      return false;
    }
    res.set_y(ly);
  }

  *out = res;
  return true;
}
}  // namespace common
