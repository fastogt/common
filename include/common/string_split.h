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

#include <string>
#include <utility>
#include <vector>

#include <common/string16.h>
#include <common/string_piece.h>

namespace common {

enum WhitespaceHandling {
  KEEP_WHITESPACE,
  TRIM_WHITESPACE,
};

enum SplitResult {
  // Strictly return all results.
  //
  // If the input is ",," and the separator is ',' this will return a
  // vector of three empty strings.
  SPLIT_WANT_ALL,

  // Only nonempty results will be added to the results. Multiple separators
  // will be coalesced. Separators at the beginning and end of the input will
  // be ignored. With TRIM_WHITESPACE, whitespace-only results will be dropped.
  //
  // If the input is ",," and the separator is ',', this will return an empty
  // vector.
  SPLIT_WANT_NONEMPTY,
};

// Split the given string on ANY of the given separators, returning copies of
// the result.
//
// Note this is inverse of JoinString() defined in string_util.h.
//
// To split on either commas or semicolons, keeping all whitespace:
//
//   std::vector<std::string> tokens = common::SplitString(
//       input, ", WARN_UNUSED_RESULT;", common::KEEP_WHITESPACE,
//       common::SPLIT_WANT_ALL) WARN_UNUSED_RESULT;
std::vector<std::string> SplitString(StringPiece input,
                                     StringPiece separators,
                                     WhitespaceHandling whitespace,
                                     SplitResult result_type) WARN_UNUSED_RESULT;
std::vector<string16> SplitString(StringPiece16 input,
                                  StringPiece16 separators,
                                  WhitespaceHandling whitespace,
                                  SplitResult result_type) WARN_UNUSED_RESULT;

// Like SplitString above except it returns a vector of StringPieces which
// reference the original buffer without copying. Although you have to be
// careful to keep the original string unmodified, this provides an efficient
// way to iterate through tokens in a string.
//
// Note this is inverse of JoinString() defined in string_util.h.
//
// To iterate through all whitespace-separated tokens in an input string:
//
//   for (const auto& cur :
//        common::SplitStringPiece(input, common::kWhitespaceASCII,
//                               common::KEEP_WHITESPACE,
//                               common::SPLIT_WANT_NONEMPTY)) {
//     ...
std::vector<StringPiece> SplitStringPiece(StringPiece input,
                                          StringPiece separators,
                                          WhitespaceHandling whitespace,
                                          SplitResult result_type) WARN_UNUSED_RESULT;
std::vector<StringPiece16> SplitStringPiece(StringPiece16 input,
                                            StringPiece16 separators,
                                            WhitespaceHandling whitespace,
                                            SplitResult result_type) WARN_UNUSED_RESULT;

using StringPairs = std::vector<std::pair<std::string, std::string>>;

// Splits |line| into key value pairs according to the given delimiters and
// removes whitespace leading each key and trailing each value. Returns true
// only if each pair has a non-empty key and value. |key_value_pairs| will
// include ("","") pairs for entries without |key_value_delimiter|.
bool SplitStringIntoKeyValuePairs(StringPiece input,
                                  char key_value_delimiter,
                                  char key_value_pair_delimiter,
                                  StringPairs* key_value_pairs);

// Similar to SplitStringIntoKeyValuePairs, but use a substring
// |key_value_pair_delimiter| instead of a single char.
bool SplitStringIntoKeyValuePairsUsingSubstr(StringPiece input,
                                             char key_value_delimiter,
                                             StringPiece key_value_pair_delimiter,
                                             StringPairs* key_value_pairs);

// Similar to SplitString, but use a substring delimiter instead of a list of
// characters that are all possible delimiters.
std::vector<string16> SplitStringUsingSubstr(StringPiece16 input,
                                             StringPiece16 delimiter,
                                             WhitespaceHandling whitespace,
                                             SplitResult result_type) WARN_UNUSED_RESULT;
std::vector<std::string> SplitStringUsingSubstr(StringPiece input,
                                                StringPiece delimiter,
                                                WhitespaceHandling whitespace,
                                                SplitResult result_type) WARN_UNUSED_RESULT;

// Like SplitStringUsingSubstr above except it returns a vector of StringPieces
// which reference the original buffer without copying. Although you have to be
// careful to keep the original string unmodified, this provides an efficient
// way to iterate through tokens in a string.
//
// To iterate through all newline-separated tokens in an input string:
//
//   for (const auto& cur :
//        common::SplitStringUsingSubstr(input, "\r\n",
//                                     common::KEEP_WHITESPACE,
//                                     common::SPLIT_WANT_NONEMPTY)) {
//     ...
std::vector<StringPiece16> SplitStringPieceUsingSubstr(StringPiece16 input,
                                                       StringPiece16 delimiter,
                                                       WhitespaceHandling whitespace,
                                                       SplitResult result_type) WARN_UNUSED_RESULT;
std::vector<StringPiece> SplitStringPieceUsingSubstr(StringPiece input,
                                                     StringPiece delimiter,
                                                     WhitespaceHandling whitespace,
                                                     SplitResult result_type) WARN_UNUSED_RESULT;

#if defined(OS_WIN) && defined(BASE_STRING16_IS_STD_U16STRING)
std::vector<std::wstring> SplitString(WStringPiece input,
                                      WStringPiece separators,
                                      WhitespaceHandling whitespace,
                                      SplitResult result_type) WARN_UNUSED_RESULT;

std::vector<WStringPiece> SplitStringPiece(WStringPiece input,
                                           WStringPiece separators,
                                           WhitespaceHandling whitespace,
                                           SplitResult result_type) WARN_UNUSED_RESULT;

std::vector<std::wstring> SplitStringUsingSubstr(WStringPiece input,
                                                 WStringPiece delimiter,
                                                 WhitespaceHandling whitespace,
                                                 SplitResult result_type) WARN_UNUSED_RESULT;

std::vector<WStringPiece> SplitStringPieceUsingSubstr(WStringPiece input,
                                                      WStringPiece delimiter,
                                                      WhitespaceHandling whitespace,
                                                      SplitResult result_type) WARN_UNUSED_RESULT;
#endif

}  // namespace common
