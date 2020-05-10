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

#include <common/parse_number.h>

#include <common/string_number_conversions.h>
#include <common/string_util.h>

namespace common {

namespace {

// The string to number conversion functions in //base include the type in the
// name (like StringToInt64()). The following wrapper methods create a
// consistent interface to StringToXXX() that calls the appropriate //base
// version. This simplifies writing generic code with a template.

bool StringToNumber(const StringPiece& input, int32_t* output) {
  // This assumes ints are 32-bits (will fail compile if that ever changes).
  return StringToInt(input, output);
}

bool StringToNumber(const StringPiece& input, uint32_t* output) {
  // This assumes ints are 32-bits (will fail compile if that ever changes).
  return StringToUint(input, output);
}

bool StringToNumber(const StringPiece& input, int64_t* output) {
  return StringToInt64(input, output);
}

bool StringToNumber(const StringPiece& input, uint64_t* output) {
  return StringToUint64(input, output);
}

bool SetError(ParseIntError error, ParseIntError* optional_error) {
  if (optional_error)
    *optional_error = error;
  return false;
}

template <typename T>
bool ParseIntHelper(const StringPiece& input, ParseIntFormat format, T* output, ParseIntError* optional_error) {
  // Check that the input matches the format before calling StringToNumber().
  // Numbers must start with either a digit or a negative sign.
  if (input.empty())
    return SetError(ParseIntError::FAILED_PARSE, optional_error);

  bool starts_with_negative = input[0] == '-';
  bool starts_with_digit = IsAsciiDigit(input[0]);

  if (!starts_with_digit) {
    if (format == ParseIntFormat::NON_NEGATIVE || !starts_with_negative)
      return SetError(ParseIntError::FAILED_PARSE, optional_error);
  }

  // Dispatch to the appropriate flavor of StringToXXX() by calling one of
  // the type-specific overloads.
  T result;
  if (StringToNumber(input, &result)) {
    *output = result;
    return true;
  }

  // Optimization: If the error is not going to be inspected, don't bother
  // calculating it.
  if (!optional_error)
    return false;

  // Set an error that distinguishes between parsing/underflow/overflow errors.
  //
  // Note that the output set by StringToXXX() on failure cannot be used
  // as it has ambiguity with parse errors.

  // Strip any leading negative sign off the number.
  StringPiece numeric_portion = starts_with_negative ? input.substr(1) : input;

  // Test if |numeric_portion| is a valid non-negative integer.
  if (!numeric_portion.empty() && numeric_portion.find_first_not_of("0123456789") == std::string::npos) {
    // If it was, the failure must have been due to underflow/overflow.
    return SetError(starts_with_negative ? ParseIntError::FAILED_UNDERFLOW : ParseIntError::FAILED_OVERFLOW,
                    optional_error);
  }

  // Otherwise it was a mundane parsing error.
  return SetError(ParseIntError::FAILED_PARSE, optional_error);
}

}  // namespace

bool ParseInt32(const StringPiece& input, ParseIntFormat format, int32_t* output, ParseIntError* optional_error) {
  return ParseIntHelper(input, format, output, optional_error);
}

bool ParseInt64(const StringPiece& input, ParseIntFormat format, int64_t* output, ParseIntError* optional_error) {
  return ParseIntHelper(input, format, output, optional_error);
}

bool ParseUint32(const StringPiece& input, uint32_t* output, ParseIntError* optional_error) {
  return ParseIntHelper(input, ParseIntFormat::NON_NEGATIVE, output, optional_error);
}

bool ParseUint64(const StringPiece& input, uint64_t* output, ParseIntError* optional_error) {
  return ParseIntHelper(input, ParseIntFormat::NON_NEGATIVE, output, optional_error);
}

}  // namespace common
