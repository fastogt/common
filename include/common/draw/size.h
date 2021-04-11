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

#include <common/numerics/checked_math.h>

#if defined(OS_WIN)
typedef struct tagSIZE SIZE;
#elif defined(OS_MACOSX) || defined(OS_IOS)
typedef struct CGSize CGSize;
#endif

namespace common {
namespace draw {

// A size has width and height values.
class Size {
 public:
  constexpr Size() : width_(0), height_(0) {}
  constexpr Size(int width, int height) : width_(std::max(0, width)), height_(std::max(0, height)) {}
#if defined(OS_MACOSX) || defined(OS_IOS)
  explicit Size(const CGSize& s);
#endif

#if defined(OS_MACOSX) || defined(OS_IOS)
  Size& operator=(const CGSize& s);
#endif

  void operator+=(const Size& size);

  void operator-=(const Size& size);

#if defined(OS_WIN)
  SIZE ToSIZE() const;
#elif defined(OS_MACOSX) || defined(OS_IOS)
  CGSize ToCGSize() const;
#endif

  constexpr int width() const { return width_; }
  constexpr int height() const { return height_; }

  void set_width(int width) { width_ = std::max(0, width); }
  void set_height(int height) { height_ = std::max(0, height); }

  // This call will CHECK if the area of this size would overflow int.
  int GetArea() const;
  // Returns a checked numeric representation of the area.
  common::CheckedNumeric<int> GetCheckedArea() const;

  void SetSize(int width, int height) {
    set_width(width);
    set_height(height);
  }

  void Enlarge(int grow_width, int grow_height);

  void SetToMin(const Size& other);
  void SetToMax(const Size& other);

  bool IsEmpty() const { return !width() || !height(); }

  std::string ToString() const;

 private:
  int width_;
  int height_;
};

inline bool operator==(const Size& lhs, const Size& rhs) {
  return lhs.width() == rhs.width() && lhs.height() == rhs.height();
}

inline bool operator!=(const Size& lhs, const Size& rhs) {
  return !(lhs == rhs);
}

inline Size operator+(Size lhs, const Size& rhs) {
  lhs += rhs;
  return lhs;
}

inline Size operator-(Size lhs, const Size& rhs) {
  lhs -= rhs;
  return lhs;
}

// Helper methods to scale a gfx::Size to a new gfx::Size.
Size ScaleToCeiledSize(const Size& size, float x_scale, float y_scale);
Size ScaleToCeiledSize(const Size& size, float scale);
Size ScaleToFlooredSize(const Size& size, float x_scale, float y_scale);
Size ScaleToFlooredSize(const Size& size, float scale);
Size ScaleToRoundedSize(const Size& size, float x_scale, float y_scale);
Size ScaleToRoundedSize(const Size& size, float scale);

std::ostream& operator<<(std::ostream& out, const Size& size);
}  // namespace draw

std::string ConvertToString(const draw::Size& value);
bool ConvertFromString(const std::string& from, draw::Size* out);
}  // namespace common
