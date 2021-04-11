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

#include <common/macros.h>

namespace common {

template <typename T, T Tmin, T Tmax>
class BoundedValue {
 public:
  typedef T value_type;
  enum { min_value = Tmin, max_value = Tmax };
  typedef BoundedValue<value_type, min_value, max_value> self_type;

  static constexpr T value(T val) { return stable_value<T, Tmin, Tmax>::value(val); }

  // runtime checking constructor:
  explicit BoundedValue(T runtime_value) : val_(value(runtime_value)) {}

  // compile-time checked constructors:
  BoundedValue(const self_type& other) : val_(other) {}
  BoundedValue(self_type&& other) : val_(other) {}

  template <typename otherT, value_type other_min, value_type other_max>
  BoundedValue(const BoundedValue<otherT, other_min, other_max>& other)
      : val_(other)  // will just fail if T, otherT not convertible
  {
    COMPILE_ASSERT(other_min >= Tmin, "conversion disallowed from BoundedValue with lower min");
    COMPILE_ASSERT(other_max <= Tmax, "conversion disallowed from BoundedValue with higher max");
  }

  // compile-time checked assignments:
  BoundedValue& operator=(const self_type& other) {
    val_ = other.val_;
    return *this;
  }

  template <typename otherT, value_type other_min, value_type other_max>
  BoundedValue& operator=(const BoundedValue<otherT, other_min, other_max>& other) {
    COMPILE_ASSERT(other_min >= Tmin, "conversion disallowed from BoundedValue with lower min");
    COMPILE_ASSERT(other_max <= Tmax, "conversion disallowed from BoundedValue with higher max");
    val_ = other;  // will just fail if T, otherT not convertible
    return *this;
  }

  // run-time checked assignment:
  BoundedValue& operator=(const T& val) {
    val_ = value(val);
    return *this;
  }

  operator T() const { return val_; }

 private:
  value_type val_;
};

}  // namespace common
