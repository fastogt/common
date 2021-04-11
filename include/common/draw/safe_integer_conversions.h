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

#include <common/numerics/checked_math.h>

namespace common {
namespace draw {

inline int ToFlooredInt(float value) {
  return common::saturated_cast<int>(std::floor(value));
}

inline int ToCeiledInt(float value) {
  return common::saturated_cast<int>(std::ceil(value));
}

inline int ToFlooredInt(double value) {
  return common::saturated_cast<int>(std::floor(value));
}

inline int ToCeiledInt(double value) {
  return common::saturated_cast<int>(std::ceil(value));
}

inline int ToRoundedInt(float value) {
  float rounded;
  if (value >= 0.0f)
    rounded = std::floor(value + 0.5f);
  else
    rounded = std::ceil(value - 0.5f);
  return common::saturated_cast<int>(rounded);
}

inline int ToRoundedInt(double value) {
  double rounded;
  if (value >= 0.0)
    rounded = std::floor(value + 0.5);
  else
    rounded = std::ceil(value - 0.5);
  return common::saturated_cast<int>(rounded);
}

inline bool IsExpressibleAsInt(float value) {
  if (value != value)
    return false;  // no int NaN.
  if (value > std::numeric_limits<int>::max())
    return false;
  if (value < std::numeric_limits<int>::min())
    return false;
  return true;
}

}  // namespace draw
}  // namespace common
