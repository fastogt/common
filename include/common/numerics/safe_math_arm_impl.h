/*  Copyright (C) 2014-2022 FastoGT. All right reserved.

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

#include <cassert>
#include <limits>
#include <type_traits>

#include <common/numerics/safe_conversions.h>

namespace common {
namespace internal {
template <typename T, typename U>
struct CheckedMulFastAsmOp {
  static const bool is_supported = FastIntegerArithmeticPromotion<T, U>::is_contained;
  // The following is much more efficient than the Clang and GCC builtins for
  // performing overflow-checked multiplication when a twice wider type is
  // available. The below compiles down to 2-3 instructions, depending on the
  // width of the types in use.
  // As an example, an int32_t multiply compiles to:
  //    smull   r0, r1, r0, r1
  //    cmp     r1, r1, asr #31
  // And an int16_t multiply compiles to:
  //    smulbb  r1, r1, r0
  //    asr     r2, r1, #16
  //    cmp     r2, r1, asr #15
  template <typename V>
  __attribute__((always_inline)) static bool Do(T x, U y, V* result) {
    using Promotion = typename FastIntegerArithmeticPromotion<T, U>::type;
    Promotion presult;
    presult = static_cast<Promotion>(x) * static_cast<Promotion>(y);
    if (!IsValueInRangeForNumericType<V>(presult))
      return false;
    *result = static_cast<V>(presult);
    return true;
  }
};
template <typename T, typename U>
struct ClampedAddFastAsmOp {
  static const bool is_supported = BigEnoughPromotion<T, U>::is_contained &&
                                   IsTypeInRangeForNumericType<int32_t, typename BigEnoughPromotion<T, U>::type>::value;
  template <typename V>
  __attribute__((always_inline)) static V Do(T x, U y) {
    // This will get promoted to an int, so let the compiler do whatever is
    // clever and rely on the saturated cast to bounds check.
    if (IsIntegerArithmeticSafe<int, T, U>::value)
      return saturated_cast<V>(x + y);
    int32_t result;
    int32_t x_i32 = checked_cast<int32_t>(x);
    int32_t y_i32 = checked_cast<int32_t>(y);
    asm("qadd %[result], %[first], %[second]" : [result] "=r"(result) : [first] "r"(x_i32), [second] "r"(y_i32));
    return saturated_cast<V>(result);
  }
};
template <typename T, typename U>
struct ClampedSubFastAsmOp {
  static const bool is_supported = BigEnoughPromotion<T, U>::is_contained &&
                                   IsTypeInRangeForNumericType<int32_t, typename BigEnoughPromotion<T, U>::type>::value;
  template <typename V>
  __attribute__((always_inline)) static V Do(T x, U y) {
    // This will get promoted to an int, so let the compiler do whatever is
    // clever and rely on the saturated cast to bounds check.
    if (IsIntegerArithmeticSafe<int, T, U>::value)
      return saturated_cast<V>(x - y);
    int32_t result;
    int32_t x_i32 = checked_cast<int32_t>(x);
    int32_t y_i32 = checked_cast<int32_t>(y);
    asm("qsub %[result], %[first], %[second]" : [result] "=r"(result) : [first] "r"(x_i32), [second] "r"(y_i32));
    return saturated_cast<V>(result);
  }
};
template <typename T, typename U>
struct ClampedMulFastAsmOp {
  static const bool is_supported = CheckedMulFastAsmOp<T, U>::is_supported;
  template <typename V>
  __attribute__((always_inline)) static V Do(T x, U y) {
    // Use the CheckedMulFastAsmOp for full-width 32-bit values, because
    // it's fewer instructions than promoting and then saturating.
    if (!IsIntegerArithmeticSafe<int32_t, T, U>::value && !IsIntegerArithmeticSafe<uint32_t, T, U>::value) {
      V result;
      return CheckedMulFastAsmOp<T, U>::Do(x, y, &result) ? result
                                                          : CommonMaxOrMin<V>(IsValueNegative(x) ^ IsValueNegative(y));
    }
    assert((FastIntegerArithmeticPromotion<T, U>::is_contained));
    using Promotion = typename FastIntegerArithmeticPromotion<T, U>::type;
    return saturated_cast<V>(static_cast<Promotion>(x) * static_cast<Promotion>(y));
  }
};
}  // namespace internal
}  // namespace common
