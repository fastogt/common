/*  Copyright (C) 2014-2020 FastoGT. All right reserved.

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

#include <common/draw/types.h>

#include <common/convert2string.h>
#include <common/sprintf.h>

namespace common {
namespace draw {

Point::Point() : x(0), y(0) {}

Point::Point(int x, int y) : x(x), y(y) {}

bool Point::Equals(const Point& pt) const {
  return x == pt.x && y == pt.y;
}

Size::Size() : width(-1), height(-1) {}

Size::Size(int width, int height) : width(width), height(height) {}

bool Size::IsValid() const {
  return IsValidSize(width, height);
}

bool Size::Equals(const Size& sz) const {
  return width == sz.width && height == sz.height;
}

bool IsValidSize(int width, int height) {
  return width >= 0 && height >= 0;
}

}  // namespace draw

std::string ConvertToString(const draw::Point& value) {
  return MemSPrintf("%d,%d", value.x, value.y);
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
    res.x = lx;

    int ly;
    if (!ConvertFromString(from.substr(del + 1), &ly)) {
      return false;
    }
    res.y = ly;
  }

  *out = res;
  return true;
}

std::string ConvertToString(const draw::Size& value) {
  return MemSPrintf("%dx%d", value.width, value.height);
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
    res.width = lwidth;

    int lheight;
    if (!ConvertFromString(from.substr(del + 1), &lheight)) {
      return false;
    }
    res.height = lheight;
  }

  *out = res;
  return true;
}
}  // namespace common
