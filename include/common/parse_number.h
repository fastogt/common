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

#include <common/string_piece.h>

// This file contains utility functions for parsing numbers, in the context of
// network protocols.
//
// Q: Doesn't //base already provide these in string_number_conversions.h, with
//    functions like common::StringToInt()?
//
// A: Yes, and those functions are used under the hood by these implementations.
//
//    However using the common::StringTo*() has historically led to subtle bugs
//    in the context of parsing network protocols:
//
//      * Permitting a leading '+'
//      * Incorrectly classifying overflow/underflow from a parsing failure
//      * Allowing negative numbers for non-negative fields
//
//   This API tries to avoid these problems by picking sensible defaults for
//   //net code. For more details see crbug.com/596523.

namespace common {

// Format to use when parsing integers.
enum class ParseIntFormat {
  // Accepts non-negative base 10 integers of the form:
  //
  //    1*DIGIT
  //
  // This construction is used in a variety of IETF standards, such as RFC 7230
  // (HTTP).
  //
  // When attempting to parse a negative number using this format, the failure
  // will be FAILED_PARSE since it violated the expected format (and not
  // FAILED_UNDERFLOW).
  //
  // Also note that inputs need not be in minimal encoding: "0003" is valid and
  // equivalent to "3".
  NON_NEGATIVE,

  // Accept optionally negative base 10 integers of the form:
  //
  //    ["-"] 1*DIGIT
  //
  // In other words, this accepts the same things as NON_NEGATIVE, and
  // additionally recognizes those numbers prefixed with a '-'.
  //
  // Note that by this defintion "-0" IS a valid input.
  OPTIONALLY_NEGATIVE
};

// The specific reason why a ParseInt*() function failed.
enum class ParseIntError {
  // The parsed number couldn't fit into the provided output type because it was
  // too high.
  FAILED_OVERFLOW,

  // The parsed number couldn't fit into the provided output type because it was
  // too low.
  FAILED_UNDERFLOW,

  // The number failed to be parsed because it wasn't a valid decimal number (as
  // determined by the policy).
  FAILED_PARSE,
};

// The ParseInt*() functions parse a string representing a number.
//
// The format of the strings that are accepted is controlled by the |format|
// parameter. This allows rejecting negative numbers.
//
// These functions return true on success, and fill |*output| with the result.
//
// On failure, it is guaranteed that |*output| was not modified. If
// |optional_error| was non-null, then it is filled with the reason for the
// failure.
bool ParseInt32(const StringPiece& input,
                ParseIntFormat format,
                int32_t* output,
                ParseIntError* optional_error = nullptr) WARN_UNUSED_RESULT;

bool ParseInt64(const StringPiece& input,
                ParseIntFormat format,
                int64_t* output,
                ParseIntError* optional_error = nullptr) WARN_UNUSED_RESULT;

// The ParseUint*() functions parse a string representing a number.
//
// These are equivalent to calling ParseInt*() with a format string of
// ParseIntFormat::NON_NEGATIVE and unsigned output types.
bool ParseUint32(const StringPiece& input,
                 uint32_t* output,
                 ParseIntError* optional_error = nullptr) WARN_UNUSED_RESULT;

bool ParseUint64(const StringPiece& input,
                 uint64_t* output,
                 ParseIntError* optional_error = nullptr) WARN_UNUSED_RESULT;

}  // namespace common
