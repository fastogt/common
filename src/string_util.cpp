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

#include <common/string_util.h>

#include <algorithm>

#include <common/icu_utf.h>                      // for UChar32, CBU8_NEXT, etc
#include <common/patterns/singleton_pattern.h>   // for LazySingleton
#include <common/utf_string_conversion_utils.h>  // for IsValidCharacter, etc
#include <common/utf_string_conversions.h>       // for ASCIIToUTF16

namespace common {

namespace {

// Force the singleton used by EmptyString[16] to be a unique type. This
// prevents other code that might accidentally use Singleton<string> from
// getting our internal one.
struct EmptyStrings {
  EmptyStrings() {}
  const std::string s;
  const string16 s16;

  static EmptyStrings* GetInstance() { return &patterns::LazySingleton<EmptyStrings>::GetInstance(); }
};

// Used by ReplaceStringPlaceholders to track the position in the string of
// replaced parameters.
struct ReplacementOffset {
  ReplacementOffset(uintptr_t parameter, size_t offset) : parameter(parameter), offset(offset) {}

  // Index of the parameter.
  uintptr_t parameter;

  // Starting position in the string.
  size_t offset;
};

bool CompareParameter(const ReplacementOffset& elem1, const ReplacementOffset& elem2) {
  return elem1.parameter < elem2.parameter;
}

template <typename StringType>
StringType ToLowerASCIIImpl(BasicStringPiece<StringType> str) {
  StringType ret;
  ret.reserve(str.size());
  for (size_t i = 0; i < str.size(); i++)
    ret.push_back(ToLowerASCII(str[i]));
  return ret;
}

template <typename StringType>
StringType ToUpperASCIIImpl(BasicStringPiece<StringType> str) {
  StringType ret;
  ret.reserve(str.size());
  for (size_t i = 0; i < str.size(); i++)
    ret.push_back(ToUpperASCII(str[i]));
  return ret;
}

}  // namespace

std::string ToLowerASCII(StringPiece str) {
  return ToLowerASCIIImpl<std::string>(str);
}

string16 ToLowerASCII(StringPiece16 str) {
  return ToLowerASCIIImpl<string16>(str);
}

std::string ToUpperASCII(StringPiece str) {
  return ToUpperASCIIImpl<std::string>(str);
}

string16 ToUpperASCII(StringPiece16 str) {
  return ToUpperASCIIImpl<string16>(str);
}

// Overloaded function to append one string onto the end of another. Having a
// separate overload for |source| as both string and StringPiece allows for more
// efficient usage from functions templated to work with either type (avoiding a
// redundant call to the BasicStringPiece constructor in both cases).
template <typename string_type>
inline void AppendToString(string_type* target, const string_type& source) {
  target->append(source);
}
template <typename string_type>
inline void AppendToString(string_type* target, const BasicStringPiece<string_type>& source) {
  source.AppendToString(target);
}

template <typename char_type>
inline void AppendToVector(std::vector<char_type>* target, const std::vector<char_type>& source) {
  for (char_type c : source) {
    target->push_back(c);
  }
}

bool IsWprintfFormatPortable(const wchar_t* format) {
  for (const wchar_t* position = format; *position != '\0'; ++position) {
    if (*position == '%') {
      bool in_specification = true;
      bool modifier_l = false;
      while (in_specification) {
        // Eat up characters until reaching a known specifier.
        if (*++position == '\0') {
          // The format string ended in the middle of a specification.  Call
          // it portable because no unportable specifications were found.  The
          // string is equally broken on all platforms.
          return true;
        }

        if (*position == 'l') {
          // 'l' is the only thing that can save the 's' and 'c' specifiers.
          modifier_l = true;
        } else if (((*position == 's' || *position == 'c') && !modifier_l) || *position == 'S' || *position == 'C' ||
                   *position == 'F' || *position == 'D' || *position == 'O' || *position == 'U') {
          // Not portable.
          return false;
        }

        if (wcschr(L"diouxXeEfgGaAcspn%", *position)) {
          // Portable, keep scanning the rest of the format string.
          in_specification = false;
        }
      }
    }
  }

  return true;
}

const std::string& EmptyString() {
  return EmptyStrings::GetInstance()->s;
}

const string16& EmptyString16() {
  return EmptyStrings::GetInstance()->s16;
}

template <typename STR>
bool ReplaceCharsT(const STR& input, const STR& replace_chars, const STR& replace_with, STR* output) {
  bool removed = false;
  size_t replace_length = replace_with.length();

  *output = input;

  size_t found = output->find_first_of(replace_chars);
  while (found != STR::npos) {
    removed = true;
    output->replace(found, 1, replace_with);
    found = output->find_first_of(replace_chars, found + replace_length);
  }

  return removed;
}

bool ReplaceChars(const string16& input,
                  const StringPiece16& replace_chars,
                  const string16& replace_with,
                  string16* output) {
  return ReplaceCharsT(input, replace_chars.as_string(), replace_with, output);
}

bool ReplaceChars(const std::string& input,
                  const StringPiece& replace_chars,
                  const std::string& replace_with,
                  std::string* output) {
  return ReplaceCharsT(input, replace_chars.as_string(), replace_with, output);
}

bool RemoveChars(const string16& input, const StringPiece16& remove_chars, string16* output) {
  return ReplaceChars(input, remove_chars.as_string(), string16(), output);
}

bool RemoveChars(const std::string& input, const StringPiece& remove_chars, std::string* output) {
  return ReplaceChars(input, remove_chars.as_string(), std::string(), output);
}

template <typename STR>
TrimPositions TrimStringT(const STR& input, const STR& trim_chars, TrimPositions positions, STR* output) {
  // Find the edges of leading/trailing whitespace as desired.
  const size_t last_char = input.length() - 1;
  const size_t first_good_char = (positions & TRIM_LEADING) ? input.find_first_not_of(trim_chars) : 0;
  const size_t last_good_char = (positions & TRIM_TRAILING) ? input.find_last_not_of(trim_chars) : last_char;

  // When the string was all whitespace, report that we stripped off whitespace
  // from whichever position the caller was interested in.  For empty input, we
  // stripped no whitespace, but we still need to clear |output|.
  if (input.empty() || (first_good_char == STR::npos) || (last_good_char == STR::npos)) {
    bool input_was_empty = input.empty();  // in case output == &input
    output->clear();
    return input_was_empty ? TRIM_NONE : positions;
  }

  // Trim the whitespace.
  *output = input.substr(first_good_char, last_good_char - first_good_char + 1);

  // Return where we trimmed from.
  return static_cast<TrimPositions>(((first_good_char == 0) ? TRIM_NONE : TRIM_LEADING) |
                                    ((last_good_char == last_char) ? TRIM_NONE : TRIM_TRAILING));
}

bool TrimString(const string16& input, const StringPiece16& trim_chars, string16* output) {
  return TrimStringT(input, trim_chars.as_string(), TRIM_ALL, output) != TRIM_NONE;
}

bool TrimString(const std::string& input, const StringPiece& trim_chars, std::string* output) {
  return TrimStringT(input, trim_chars.as_string(), TRIM_ALL, output) != TRIM_NONE;
}

template <typename Str>
BasicStringPiece<Str> TrimStringPieceT(BasicStringPiece<Str> input,
                                       BasicStringPiece<Str> trim_chars,
                                       TrimPositions positions) {
  size_t begin = (positions & TRIM_LEADING) ? input.find_first_not_of(trim_chars) : 0;
  size_t end = (positions & TRIM_TRAILING) ? input.find_last_not_of(trim_chars) + 1 : input.size();
  return input.substr(begin, end - begin);
}

StringPiece16 TrimString(StringPiece16 input, StringPiece16 trim_chars, TrimPositions positions) {
  return TrimStringPieceT(input, trim_chars, positions);
}

StringPiece TrimString(StringPiece input, StringPiece trim_chars, TrimPositions positions) {
  return TrimStringPieceT(input, trim_chars, positions);
}

void TruncateUTF8ToByteSize(const std::string& input, const size_t byte_size, std::string* output) {
  if (!output) {
    return;
  }

  if (byte_size > input.length()) {
    *output = input;
    return;
  }
  DCHECK_LE(byte_size, static_cast<uint32_t>(std::numeric_limits<int32_t>::max()));
  // Note: This cast is necessary because CBU8_NEXT uses int32s.
  int32_t truncation_length = static_cast<int32_t>(byte_size);
  int32_t char_index = truncation_length - 1;
  const char* data = input.data();

  // Using CBU8, we will move backwards from the truncation point
  // to the beginning of the string looking for a valid UTF8
  // character.  Once a full UTF8 character is found, we will
  // truncate the string to the end of that character.
  while (char_index >= 0) {
    int32_t prev = char_index;
    base_icu::UChar32 code_point = 0;
    CBU8_NEXT(data, char_index, truncation_length, code_point);
    if (!IsValidCharacter(code_point) || !IsValidCodepoint(code_point)) {
      char_index = prev - 1;
    } else {
      break;
    }
  }

  if (char_index >= 0) {
    *output = input.substr(0, char_index);
  } else {
    output->clear();
  }
}

TrimPositions TrimWhitespace(const string16& input, TrimPositions positions, string16* output) {
  return TrimStringT(input, string16(kWhitespaceUTF16), positions, output);
}

TrimPositions TrimWhitespaceASCII(const std::string& input, TrimPositions positions, std::string* output) {
  return TrimStringT(input, std::string(kWhitespaceASCII), positions, output);
}

// This function is only for backward-compatibility.
// To be removed when all callers are updated.
TrimPositions TrimWhitespace(const std::string& input, TrimPositions positions, std::string* output) {
  return TrimWhitespaceASCII(input, positions, output);
}

template <typename STR>
STR CollapseWhitespaceT(const STR& text, bool trim_sequences_with_line_breaks) {
  STR result;
  result.resize(text.size());

  // Set flags to pretend we're already in a trimmed whitespace sequence, so we
  // will trim any leading whitespace.
  bool in_whitespace = true;
  bool already_trimmed = true;

  int chars_written = 0;
  for (typename STR::const_iterator i(text.begin()); i != text.end(); ++i) {
    if (IsWhitespace(*i)) {
      if (!in_whitespace) {
        // Reduce all whitespace sequences to a single space.
        in_whitespace = true;
        result[chars_written++] = L' ';
      }
      if (trim_sequences_with_line_breaks && !already_trimmed && ((*i == '\n') || (*i == '\r'))) {
        // Whitespace sequences containing CR or LF are eliminated entirely.
        already_trimmed = true;
        --chars_written;
      }
    } else {
      // Non-whitespace chracters are copied straight across.
      in_whitespace = false;
      already_trimmed = false;
      result[chars_written++] = *i;
    }
  }

  if (in_whitespace && !already_trimmed) {
    // Any trailing whitespace is eliminated.
    --chars_written;
  }

  result.resize(chars_written);
  return result;
}

string16 CollapseWhitespace(const string16& text, bool trim_sequences_with_line_breaks) {
  return CollapseWhitespaceT(text, trim_sequences_with_line_breaks);
}

std::string CollapseWhitespaceASCII(const std::string& text, bool trim_sequences_with_line_breaks) {
  return CollapseWhitespaceT(text, trim_sequences_with_line_breaks);
}

bool ContainsOnlyChars(const StringPiece& input, const StringPiece& characters) {
  return input.find_first_not_of(characters) == StringPiece::npos;
}

bool ContainsOnlyChars(const StringPiece16& input, const StringPiece16& characters) {
  return input.find_first_not_of(characters) == StringPiece16::npos;
}

template <class STR>
static bool DoIsStringASCII(const STR& str) {
  for (size_t i = 0; i < str.length(); i++) {
    typename ToUnsigned<typename STR::value_type>::Unsigned c = str[i];
    if (c > 0x7F) {
      return false;
    }
  }
  return true;
}

bool IsStringASCII(const StringPiece& str) {
  return DoIsStringASCII(str);
}

bool IsStringASCII(const string16& str) {
  return DoIsStringASCII(str);
}

bool IsStringUTF8(const std::string& str) {
  const char* src = str.data();
  int32_t src_len = static_cast<int32_t>(str.length());
  int32_t char_index = 0;

  while (char_index < src_len) {
    int32_t code_point;
    CBU8_NEXT(src, char_index, src_len, code_point);
    if (!IsValidCharacter(code_point)) {
      return false;
    }
  }
  return true;
}

template <typename Str>
static inline bool DoLowerCaseEqualsASCII(BasicStringPiece<Str> str, StringPiece lowercase_ascii) {
  if (str.size() != lowercase_ascii.size())
    return false;
  for (size_t i = 0; i < str.size(); i++) {
    if (ToLowerASCII(str[i]) != lowercase_ascii[i])
      return false;
  }
  return true;
}

bool LowerCaseEqualsASCII(StringPiece str, StringPiece lowercase_ascii) {
  return DoLowerCaseEqualsASCII<std::string>(str, lowercase_ascii);
}

bool LowerCaseEqualsASCII(StringPiece16 str, StringPiece lowercase_ascii) {
  return DoLowerCaseEqualsASCII<string16>(str, lowercase_ascii);
}

bool EqualsASCII(const string16& a, const StringPiece& b) {
  if (a.length() != b.length()) {
    return false;
  }
  return std::equal(b.begin(), b.end(), a.begin());
}

bool StartsWithASCII(const std::string& str, const std::string& search, bool case_sensitive) {
  if (case_sensitive) {
    return str.compare(0, search.length(), search) == 0;
  }

  return strncasecmp(str.c_str(), search.c_str(), search.length()) == 0;
}

template <typename STR>
bool StartsWithT(const STR& str, const STR& search, bool case_sensitive) {
  if (search.size() > str.size()) {
    return false;
  }

  if (case_sensitive) {
    for (size_t i = 0; i < search.size(); ++i) {
      if (str[i] != search[i]) {
        return false;
      }
    }
    return true;
  }

  return std::equal(search.begin(), search.end(), str.begin(), CaseInsensitiveCompare<typename STR::value_type>());
}

bool EqualsASCII(const std::string& a, const std::string& b, bool case_sensitive) {
  return StartsWithT(a, b, case_sensitive);
}

bool EqualsASCII(const std::vector<unsigned char>& a, const std::vector<unsigned char>& b, bool case_sensitive) {
  return StartsWithT(a, b, case_sensitive);
}

bool EqualsASCII(const std::vector<char>& a, const std::vector<char>& b, bool case_sensitive) {
  return StartsWithT(a, b, case_sensitive);
}

template <typename STR>
bool FullEqualsASCIIT(const STR& str, const STR& search, bool case_sensitive) {
  if (search.size() != str.size()) {
    return false;
  }

  if (case_sensitive) {
    return str == search;
  }

  return std::equal(search.begin(), search.end(), str.begin(), CaseInsensitiveCompare<typename STR::value_type>());
}

bool FullEqualsASCII(const std::string& a, const std::string& b, bool case_sensitive) {
  return FullEqualsASCIIT(a, b, case_sensitive);
}

bool FullEqualsASCII(const std::vector<unsigned char>& a, const std::vector<unsigned char>& b, bool case_sensitive) {
  return FullEqualsASCIIT(a, b, case_sensitive);
}

bool FullEqualsASCII(const std::vector<char>& a, const std::vector<char>& b, bool case_sensitive) {
  return FullEqualsASCIIT(a, b, case_sensitive);
}

bool StartsWith(const string16& str, const string16& search, bool case_sensitive) {
  return StartsWithT(str, search, case_sensitive);
}

template <typename STR>
bool EndsWithT(const STR& str, const STR& search, bool case_sensitive) {
  size_t str_length = str.length();
  size_t search_length = search.length();
  if (search_length > str_length) {
    return false;
  }
  if (case_sensitive) {
    return str.compare(str_length - search_length, search_length, search) == 0;
  }
  return std::equal(search.begin(), search.end(), str.begin() + (str_length - search_length),
                    CaseInsensitiveCompare<typename STR::value_type>());
}

bool EndsWith(const std::string& str, const std::string& search, bool case_sensitive) {
  return EndsWithT(str, search, case_sensitive);
}

bool EndsWith(const string16& str, const string16& search, bool case_sensitive) {
  return EndsWithT(str, search, case_sensitive);
}

static const char* const kByteStringsUnlocalized[] = {" B", " kB", " MB", " GB", " TB", " PB"};

string16 FormatBytesUnlocalized(int64_t bytes) {
  double unit_amount = static_cast<double>(bytes);
  size_t dimension = 0;
  const int kKilo = 1024;
  while (unit_amount >= kKilo && dimension < arraysize(kByteStringsUnlocalized) - 1) {
    unit_amount /= kKilo;
    dimension++;
  }

  char buf[64];
  if (bytes != 0 && dimension > 0 && unit_amount < 100) {
    snprintf(buf, arraysize(buf), "%.1lf%s", unit_amount, kByteStringsUnlocalized[dimension]);
  } else {
    snprintf(buf, arraysize(buf), "%.0lf%s", unit_amount, kByteStringsUnlocalized[dimension]);
  }

  return ASCIIToUTF16(buf);
}

template <class StringType>
void DoReplaceSubstringsAfterOffset(StringType* str,
                                    size_t start_offset,
                                    const StringType& find_this,
                                    const StringType& replace_with,
                                    bool replace_all) {
  if ((start_offset == StringType::npos) || (start_offset >= str->length())) {
    return;
  }

  DCHECK(!find_this.empty());
  for (size_t offs(str->find(find_this, start_offset)); offs != StringType::npos; offs = str->find(find_this, offs)) {
    str->replace(offs, find_this.length(), replace_with);
    offs += replace_with.length();

    if (!replace_all) {
      break;
    }
  }
}

void ReplaceFirstSubstringAfterOffset(string16* str,
                                      size_t start_offset,
                                      const string16& find_this,
                                      const string16& replace_with) {
  DoReplaceSubstringsAfterOffset(str, start_offset, find_this, replace_with, false);  // replace first instance
}

void ReplaceFirstSubstringAfterOffset(std::string* str,
                                      size_t start_offset,
                                      const std::string& find_this,
                                      const std::string& replace_with) {
  DoReplaceSubstringsAfterOffset(str, start_offset, find_this, replace_with, false);  // replace first instance
}

void ReplaceSubstringsAfterOffset(string16* str,
                                  size_t start_offset,
                                  const string16& find_this,
                                  const string16& replace_with) {
  DoReplaceSubstringsAfterOffset(str, start_offset, find_this, replace_with, true);  // replace all instances
}

void ReplaceSubstringsAfterOffset(std::string* str,
                                  size_t start_offset,
                                  const std::string& find_this,
                                  const std::string& replace_with) {
  DoReplaceSubstringsAfterOffset(str, start_offset, find_this, replace_with, true);  // replace all instances
}

template <typename STR>
static size_t TokenizeT(const STR& str, const STR& delimiters, std::vector<STR>* tokens) {
  tokens->clear();

  size_t start = str.find_first_not_of(delimiters);
  while (start != STR::npos) {
    size_t end = str.find_first_of(delimiters, start + 1);
    if (end == STR::npos) {
      tokens->push_back(str.substr(start));
      break;
    } else {
      tokens->push_back(str.substr(start, end - start));
      start = str.find_first_not_of(delimiters, end + 1);
    }
  }

  return tokens->size();
}

size_t Tokenize(const string16& str, const string16& delimiters, std::vector<string16>* tokens) {
  return TokenizeT(str, delimiters, tokens);
}

size_t Tokenize(const std::string& str, const std::string& delimiters, std::vector<std::string>* tokens) {
  return TokenizeT(str, delimiters, tokens);
}

size_t Tokenize(const StringPiece& str, const StringPiece& delimiters, std::vector<StringPiece>* tokens) {
  return TokenizeT(str, delimiters, tokens);
}

// Generic version for all JoinString overloads. |list_type| must be a sequence
// (std::vector or std::initializer_list) of strings/StringPieces (std::string,
// string16, StringPiece or StringPiece16). |string_type| is either std::string
// or string16.
template <typename list_type, typename string_type>
static string_type JoinStringT(const list_type& parts, BasicStringPiece<string_type> sep) {
  if (parts.size() == 0)
    return string_type();
  // Pre-allocate the eventual size of the string. Start with the size of all of
  // the separators (note that this *assumes* parts.size() > 0).
  size_t total_size = (parts.size() - 1) * sep.size();
  for (const auto& part : parts)
    total_size += part.size();
  string_type result;
  result.reserve(total_size);
  auto iter = parts.begin();
  DCHECK(iter != parts.end());
  AppendToString(&result, *iter);
  ++iter;
  for (; iter != parts.end(); ++iter) {
    sep.AppendToString(&result);
    // Using the overloaded AppendToString allows this template function to work
    // on both strings and StringPieces without creating an intermediate
    // StringPiece object.
    AppendToString(&result, *iter);
  }
  // Sanity-check that we pre-allocated correctly.
  DCHECK_EQ(total_size, result.size());
  return result;
}

template <typename list_type, typename string_type>
static string_type JoinStringTEX(const list_type& parts, string_type sep) {
  if (parts.size() == 0)
    return string_type();
  // Pre-allocate the eventual size of the string. Start with the size of all of
  // the separators (note that this *assumes* parts.size() > 0).
  size_t total_size = (parts.size() - 1) * 1;
  for (const auto& part : parts)
    total_size += part.size();
  string_type result;
  result.reserve(total_size);
  auto iter = parts.begin();
  DCHECK(iter != parts.end());
  AppendToVector(&result, *iter);
  ++iter;

  for (; iter != parts.end(); ++iter) {
    AppendToVector(&result, sep);
    // Using the overloaded AppendToString allows this template function to work
    // on both strings and StringPieces without creating an intermediate
    // StringPiece object.
    AppendToVector(&result, *iter);
  }
  // Sanity-check that we pre-allocated correctly.
  DCHECK_EQ(total_size, result.size());
  return result;
}

std::string JoinString(const std::vector<std::string>& parts, StringPiece separator) {
  return JoinStringT(parts, separator);
}
string16 JoinString(const std::vector<string16>& parts, StringPiece16 separator) {
  return JoinStringT(parts, separator);
}
std::string JoinString(const std::vector<StringPiece>& parts, StringPiece separator) {
  return JoinStringT(parts, separator);
}
string16 JoinString(const std::vector<StringPiece16>& parts, StringPiece16 separator) {
  return JoinStringT(parts, separator);
}

std::string JoinString(std::initializer_list<StringPiece> parts, StringPiece separator) {
  return JoinStringT(parts, separator);
}
string16 JoinString(std::initializer_list<StringPiece16> parts, StringPiece16 separator) {
  return JoinStringT(parts, separator);
}

template <class FormatStringType, class OutStringType>
OutStringType DoReplaceStringPlaceholders(const FormatStringType& format_string,
                                          const std::vector<OutStringType>& subst,
                                          std::vector<size_t>* offsets) {
  size_t substitutions = subst.size();

  size_t sub_length = 0;
  for (typename std::vector<OutStringType>::const_iterator iter = subst.begin(); iter != subst.end(); ++iter) {
    sub_length += iter->length();
  }

  OutStringType formatted;
  formatted.reserve(format_string.length() + sub_length);

  std::vector<ReplacementOffset> r_offsets;
  for (typename FormatStringType::const_iterator i = format_string.begin(); i != format_string.end(); ++i) {
    if ('$' == *i) {
      if (i + 1 != format_string.end()) {
        ++i;
        // DCHECK('$' == *i || '1' <= *i) << "Invalid placeholder: " << *i;
        if ('$' == *i) {
          while (i != format_string.end() && '$' == *i) {
            formatted.push_back('$');
            ++i;
          }
          --i;
        } else {
          uintptr_t index = 0;
          while (i != format_string.end() && '0' <= *i && *i <= '9') {
            index *= 10;
            index += *i - '0';
            ++i;
          }
          --i;
          index -= 1;
          if (offsets) {
            ReplacementOffset r_offset(index, static_cast<int>(formatted.size()));
            r_offsets.insert(std::lower_bound(r_offsets.begin(), r_offsets.end(), r_offset, &CompareParameter),
                             r_offset);
          }
          if (index < substitutions) {
            formatted.append(subst.at(index));
          }
        }
      }
    } else {
      formatted.push_back(*i);
    }
  }
  if (offsets) {
    for (std::vector<ReplacementOffset>::const_iterator i = r_offsets.begin(); i != r_offsets.end(); ++i) {
      offsets->push_back(i->offset);
    }
  }
  return formatted;
}

string16 ReplaceStringPlaceholders(const string16& format_string,
                                   const std::vector<string16>& subst,
                                   std::vector<size_t>* offsets) {
  return DoReplaceStringPlaceholders(format_string, subst, offsets);
}

std::string ReplaceStringPlaceholders(const StringPiece& format_string,
                                      const std::vector<std::string>& subst,
                                      std::vector<size_t>* offsets) {
  return DoReplaceStringPlaceholders(format_string, subst, offsets);
}

string16 ReplaceStringPlaceholders(const string16& format_string, const string16& a, size_t* offset) {
  std::vector<size_t> offsets;
  std::vector<string16> subst;
  subst.push_back(a);
  string16 result = ReplaceStringPlaceholders(format_string, subst, &offsets);

  DCHECK_EQ(1U, offsets.size());
  if (offset) {
    *offset = offsets[0];
  }
  return result;
}

static bool IsWildcard(base_icu::UChar32 character) {
  return character == '*' || character == '?';
}

// Move the strings pointers to the point where they start to differ.
template <typename CHAR, typename NEXT>
static void EatSameChars(const CHAR** pattern,
                         const CHAR* pattern_end,
                         const CHAR** string,
                         const CHAR* string_end,
                         NEXT next) {
  const CHAR* escape = nullptr;
  while (*pattern != pattern_end && *string != string_end) {
    if (!escape && IsWildcard(**pattern)) {
      // We don't want to match wildcard here, except if it's escaped.
      return;
    }

    // Check if the escapement char is found. If so, skip it and move to the
    // next character.
    if (!escape && **pattern == '\\') {
      escape = *pattern;
      next(pattern, pattern_end);
      continue;
    }

    // Check if the chars match, if so, increment the ptrs.
    const CHAR* pattern_next = *pattern;
    const CHAR* string_next = *string;
    base_icu::UChar32 pattern_char = next(&pattern_next, pattern_end);
    if (pattern_char == next(&string_next, string_end) && pattern_char != CBU_SENTINEL) {
      *pattern = pattern_next;
      *string = string_next;
    } else {
      // Uh oh, it did not match, we are done. If the last char was an
      // escapement, that means that it was an error to advance the ptr here,
      // let's put it back where it was. This also mean that the MatchPattern
      // function will return false because if we can't match an escape char
      // here, then no one will.
      if (escape) {
        *pattern = escape;
      }
      return;
    }

    escape = nullptr;
  }
}

template <typename CHAR, typename NEXT>
static void EatWildcard(const CHAR** pattern, const CHAR* end, NEXT next) {
  while (*pattern != end) {
    if (!IsWildcard(**pattern)) {
      return;
    }
    next(pattern, end);
  }
}

template <typename CHAR, typename NEXT>
static bool MatchPatternT(const CHAR* eval,
                          const CHAR* eval_end,
                          const CHAR* pattern,
                          const CHAR* pattern_end,
                          int depth,
                          NEXT next) {
  const int kMaxDepth = 16;
  if (depth > kMaxDepth) {
    return false;
  }

  // Eat all the matching chars.
  EatSameChars(&pattern, pattern_end, &eval, eval_end, next);

  // If the string is empty, then the pattern must be empty too, or contains
  // only wildcards.
  if (eval == eval_end) {
    EatWildcard(&pattern, pattern_end, next);
    return pattern == pattern_end;
  }

  // Pattern is empty but not string, this is not a match.
  if (pattern == pattern_end) {
    return false;
  }

  // If this is a question mark, then we need to compare the rest with
  // the current string or the string with one character eaten.
  const CHAR* next_pattern = pattern;
  next(&next_pattern, pattern_end);
  if (pattern[0] == '?') {
    if (MatchPatternT(eval, eval_end, next_pattern, pattern_end, depth + 1, next)) {
      return true;
    }
    const CHAR* next_eval = eval;
    next(&next_eval, eval_end);
    if (MatchPatternT(next_eval, eval_end, next_pattern, pattern_end, depth + 1, next)) {
      return true;
    }
  }

  // This is a *, try to match all the possible substrings with the remainder
  // of the pattern.
  if (pattern[0] == '*') {
    // Collapse duplicate wild cards (********** into *) so that the
    // method does not recurse unnecessarily. http://crbug.com/52839
    EatWildcard(&next_pattern, pattern_end, next);

    while (eval != eval_end) {
      if (MatchPatternT(eval, eval_end, next_pattern, pattern_end, depth + 1, next)) {
        return true;
      }
      eval++;
    }

    // We reached the end of the string, let see if the pattern contains only
    // wildcards.
    if (eval == eval_end) {
      EatWildcard(&pattern, pattern_end, next);
      if (pattern != pattern_end) {
        return false;
      }
      return true;
    }
  }

  return false;
}

struct NextCharUTF8 {
  base_icu::UChar32 operator()(const char** p, const char* end) const {
    base_icu::UChar32 c;
    int offset = 0;
    CBU8_NEXT(*p, offset, end - *p, c);
    *p += offset;
    return c;
  }
};

struct NextCharUTF16 {
  base_icu::UChar32 operator()(const char16** p, const char16* end) const {
    base_icu::UChar32 c;
    int offset = 0;
    CBU16_NEXT(*p, offset, end - *p, c);
    *p += offset;
    return c;
  }
};

bool MatchPattern(const StringPiece& eval, const StringPiece& pattern) {
  return MatchPatternT(eval.data(), eval.data() + eval.size(), pattern.data(), pattern.data() + pattern.size(), 0,
                       NextCharUTF8());
}

bool MatchPattern(const string16& eval, const string16& pattern) {
  return MatchPatternT(eval.c_str(), eval.c_str() + eval.size(), pattern.c_str(), pattern.c_str() + pattern.size(), 0,
                       NextCharUTF16());
}

// The following code is compatible with the OpenBSD lcpy interface.  See:
//   http://www.gratisoft.us/todd/papers/strlcpy.html
//   ftp://ftp.openbsd.org/pub/OpenBSD/src/lib/libc/string/{wcs,str}lcpy.c

namespace {

template <typename CHAR>
size_t lcpyT(CHAR* dst, const CHAR* src, size_t dst_size) {
  for (size_t i = 0; i < dst_size; ++i) {
    if ((dst[i] = src[i]) == 0) {  // We hit and copied the terminating NULL.
      return i;
    }
  }

  // We were left off at dst_size.  We over copied 1 byte.  Null terminate.
  if (dst_size != 0) {
    dst[dst_size - 1] = 0;
  }

  // Count the rest of the |src|, and return it's length in characters.
  while (src[dst_size]) {
    ++dst_size;
  }
  return dst_size;
}

}  // namespace

size_t strlcpy(char* dst, const char* src, size_t dst_size) {
  return lcpyT<char>(dst, src, dst_size);
}
size_t wcslcpy(wchar_t* dst, const wchar_t* src, size_t dst_size) {
  return lcpyT<wchar_t>(dst, src, dst_size);
}

}  // namespace common
