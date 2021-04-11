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

#include <stdarg.h>  // for va_list, va_end, va_start

#include <string>
#include <vector>

#include <common/string_piece.h>  // for StringPiece, StringPiece16

namespace common {

// C standard-library functions like "strncasecmp" and "snprintf" that aren't
// cross-platform are provided as "common::strncasecmp", and their prototypes
// are listed below.  These functions are then implemented as inline calls
// to the platform-specific equivalents in the platform-specific headers.

// Compares the two strings s1 and s2 without regard to case using
// the current locale; returns 0 if they are equal, 1 if s1 > s2, and -1 if
// s2 > s1 according to a lexicographic comparison.
int strcasecmp(const char* s1, const char* s2);

// Compares up to count characters of s1 and s2 without regard to case using
// the current locale; returns 0 if they are equal, 1 if s1 > s2, and -1 if
// s2 > s1 according to a lexicographic comparison.
int strncasecmp(const char* s1, const char* s2, size_t count);

const char* strcasestr(const char* s, const char* find);

// Same as strncmp but for char16 strings.
int strncmp16(const char16* s1, const char16* s2, size_t count);

// Wrapper for vsnprintf that always null-terminates and always returns the
// number of characters that would be in an untruncated formatted
// string, even when truncation occurs.
int vsnprintf(char* buffer, size_t size, const char* format, va_list arguments) PRINTF_FORMAT(3, 0);

/* Maximum chars of output to write in MAXLEN.  */
int vasprintf(char** ptr, const char* format, va_list ap);
inline int vasprintf(char** ptr, const char* format, ...) PRINTF_FORMAT(2, 3);
inline int vasprintf(char** ptr, const char* format, ...) {
  va_list arguments;
  va_start(arguments, format);
  int result = vasprintf(ptr, format, arguments);
  va_end(arguments);
  return result;
}

// Some of these implementations need to be inlined.

// We separate the declaration from the implementation of this inline
// function just so the PRINTF_FORMAT works.
inline int snprintf(char* buffer, size_t size, const char* format, ...) PRINTF_FORMAT(3, 4);
inline int snprintf(char* buffer, size_t size, const char* format, ...) {
  va_list arguments;
  va_start(arguments, format);
  int result = vsnprintf(buffer, size, format, arguments);
  va_end(arguments);
  return result;
}

// BSD-style safe and consistent string copy functions.
// Copies |src| to |dst|, where |dst_size| is the total allocated size of |dst|.
// Copies at most |dst_size|-1 characters, and always NULL terminates |dst|, as
// long as |dst_size| is not 0.  Returns the length of |src| in characters.
// If the return value is >= dst_size, then the output was truncated.
// NOTE: All sizes are in number of characters, NOT in bytes.
size_t strlcpy(char* dst, const char* src, size_t dst_size);
size_t wcslcpy(wchar_t* dst, const wchar_t* src, size_t dst_size);

// Scan a wprintf format string to determine whether it's portable across a
// variety of systems.  This function only checks that the conversion
// specifiers used by the format string are supported and have the same meaning
// on a variety of systems.  It doesn't check for other errors that might occur
// within a format string.
//
// Nonportable conversion specifiers for wprintf are:
//  - 's' and 'c' without an 'l' length modifier.  %s and %c operate on char
//     data on all systems except Windows, which treat them as wchar_t data.
//     Use %ls and %lc for wchar_t data instead.
//  - 'S' and 'C', which operate on wchar_t data on all systems except Windows,
//     which treat them as char data.  Use %ls and %lc for wchar_t data
//     instead.
//  - 'F', which is not identified by Windows wprintf documentation.
//  - 'D', 'O', and 'U', which are deprecated and not available on all systems.
//     Use %ld, %lo, and %lu instead.
//
// Note that there is no portable conversion specifier for char data when
// working with wprintf.
//
// This function is intended to be called from common::vswprintf.
bool IsWprintfFormatPortable(const wchar_t* format);

// ASCII-specific tolower.  The standard library's tolower is locale sensitive,
// so we don't want to use it here.
inline char ToLowerASCII(char c) {
  return (c >= 'A' && c <= 'Z') ? (c + ('a' - 'A')) : c;
}
inline char16 ToLowerASCII(char16 c) {
  return (c >= 'A' && c <= 'Z') ? (c + ('a' - 'A')) : c;
}

// ASCII-specific toupper.  The standard library's toupper is locale sensitive,
// so we don't want to use it here.
inline char ToUpperASCII(char c) {
  return (c >= 'a' && c <= 'z') ? (c + ('A' - 'a')) : c;
}
inline char16 ToUpperASCII(char16 c) {
  return (c >= 'a' && c <= 'z') ? (c + ('A' - 'a')) : c;
}

// Converts the given string to it's ASCII-lowercase equivalent.
std::string ToLowerASCII(StringPiece str);
string16 ToLowerASCII(StringPiece16 str);

// Converts the given string to it's ASCII-uppercase equivalent.
std::string ToUpperASCII(StringPiece str);
string16 ToUpperASCII(StringPiece16 str);

// Function objects to aid in comparing/searching strings.

template <typename Char>
struct CaseInsensitiveCompare {
 public:
  bool operator()(Char x, Char y) const {
    // TODO(darin): Do we really want to do locale sensitive comparisons here?
    // See http://crbug.com/24917
    return tolower(x) == tolower(y);
  }
};

template <typename Char>
struct CaseInsensitiveCompareASCII {
 public:
  bool operator()(Char x, Char y) const { return ToLowerASCII(x) == ToLowerASCII(y); }
};

// These threadsafe functions return references to globally unique empty
// strings.
//
// It is likely faster to construct a new empty string object (just a few
// instructions to set the length to 0) than to get the empty string singleton
// returned by these functions (which requires threadsafe singleton access).
//
// Therefore, DO NOT USE THESE AS A GENERAL-PURPOSE SUBSTITUTE FOR DEFAULT
// CONSTRUCTORS. There is only one case where you should use these: functions
// which need to return a string by reference (e.g. as a class member
// accessor), and don't have an empty string to use (e.g. in an error case).
// These should not be used as initializers, function arguments, or return
// values for functions which return by value or outparam.
const std::string& EmptyString();
const string16& EmptyString16();

// Contains the set of characters representing whitespace in the corresponding
// encoding. Null-terminated.
extern const wchar_t kWhitespaceWide[];
extern const char16 kWhitespaceUTF16[];
extern const char kWhitespaceASCII[];

// Null-terminated string representing the UTF-8 byte order mark.
extern const char kUtf8ByteOrderMark[];

// Removes characters in |remove_chars| from anywhere in |input|.  Returns true
// if any characters were removed.  |remove_chars| must be null-terminated.
// NOTE: Safe to use the same variable for both |input| and |output|.
bool RemoveChars(const string16& input, const StringPiece16& remove_chars, string16* output);
bool RemoveChars(const std::string& input, const StringPiece& remove_chars, std::string* output);

// Replaces characters in |replace_chars| from anywhere in |input| with
// |replace_with|.  Each character in |replace_chars| will be replaced with
// the |replace_with| string.  Returns true if any characters were replaced.
// |replace_chars| must be null-terminated.
// NOTE: Safe to use the same variable for both |input| and |output|.
bool ReplaceChars(const string16& input,
                  const StringPiece16& replace_chars,
                  const string16& replace_with,
                  string16* output);
bool ReplaceChars(const std::string& input,
                  const StringPiece& replace_chars,
                  const std::string& replace_with,
                  std::string* output);

// Removes characters in |trim_chars| from the beginning and end of |input|.
// |trim_chars| must be null-terminated.
// NOTE: Safe to use the same variable for both |input| and |output|.
bool TrimString(const string16& input, const StringPiece16& trim_chars, string16* output);
bool TrimString(const std::string& input, const StringPiece& trim_chars, std::string* output);

// Trims any whitespace from either end of the input string.  Returns where
// whitespace was found.
// The non-wide version has two functions:
// * TrimWhitespaceASCII()
//   This function is for ASCII strings and only looks for ASCII whitespace;
// Please choose the best one according to your usage.
// NOTE: Safe to use the same variable for both input and output.
enum TrimPositions {
  TRIM_NONE = 0,
  TRIM_LEADING = 1 << 0,
  TRIM_TRAILING = 1 << 1,
  TRIM_ALL = TRIM_LEADING | TRIM_TRAILING,
};

// StringPiece versions of the above. The returned pieces refer to the original
// buffer.
StringPiece16 TrimString(StringPiece16 input, StringPiece16 trim_chars, TrimPositions positions);
StringPiece TrimString(StringPiece input, StringPiece trim_chars, TrimPositions positions);

// Truncates a string to the nearest UTF-8 character that will leave
// the string less than or equal to the specified byte size.
void TruncateUTF8ToByteSize(const std::string& input, const size_t byte_size, std::string* output);

TrimPositions TrimWhitespace(const string16& input, TrimPositions positions, string16* output);
TrimPositions TrimWhitespaceASCII(const std::string& input, TrimPositions positions, std::string* output);

// Deprecated. This function is only for backward compatibility and calls
// TrimWhitespaceASCII().
TrimPositions TrimWhitespace(const std::string& input, TrimPositions positions, std::string* output);

// Searches  for CR or LF characters.  Removes all contiguous whitespace
// strings that contain them.  This is useful when trying to deal with text
// copied from terminals.
// Returns |text|, with the following three transformations:
// (1) Leading and trailing whitespace is trimmed.
// (2) If |trim_sequences_with_line_breaks| is true, any other whitespace
//     sequences containing a CR or LF are trimmed.
// (3) All other whitespace sequences are converted to single spaces.
string16 CollapseWhitespace(const string16& text, bool trim_sequences_with_line_breaks);
std::string CollapseWhitespaceASCII(const std::string& text, bool trim_sequences_with_line_breaks);

// Returns true if |input| is empty or contains only characters found in
// |characters|.
bool ContainsOnlyChars(const StringPiece& input, const StringPiece& characters);
bool ContainsOnlyChars(const StringPiece16& input, const StringPiece16& characters);

// Returns true if the specified string matches the criteria. How can a wide
// string be 8-bit or UTF8? It contains only characters that are < 256 (in the
// first case) or characters that use only 8-bits and whose 8-bit
// representation looks like a UTF-8 string (the second case).
//
// Note that IsStringUTF8 checks not only if the input is structurally
// valid but also if it doesn't contain any non-character codepoint
// (e.g. U+FFFE). It's done on purpose because all the existing callers want
// to have the maximum 'discriminating' power from other encodings. If
// there's a use case for just checking the structural validity, we have to
// add a new function for that.
bool IsStringUTF8(const std::string& str);
bool IsStringASCII(const StringPiece& str);
bool IsStringASCII(const string16& str);

// Converts the elements of the given string.  This version uses a pointer to
// clearly differentiate it from the non-pointer variant.
template <class str>
inline void StringToLowerASCII(str* s) {
  for (typename str::iterator i = s->begin(); i != s->end(); ++i) {
    *i = ToLowerASCII(*i);
  }
}

template <class str>
inline str StringToLowerASCII(const str& s) {
  // for std::string and std::wstring
  str output(s);
  StringToLowerASCII(&output);
  return output;
}

}  // namespace common

#if defined(OS_WIN)
#include <common/string_util_win.h>
#elif defined(OS_POSIX)
#include <common/string_util_posix.h>
#else
#error Define string operations appropriately for your platform
#endif

namespace common {

// Converts the elements of the given string.  This version uses a pointer to
// clearly differentiate it from the non-pointer variant.
template <class str>
inline void StringToUpperASCII(str* s) {
  for (typename str::iterator i = s->begin(); i != s->end(); ++i)
    *i = ToUpperASCII(*i);
}

template <class str>
inline str StringToUpperASCII(const str& s) {
  // for std::string and std::wstring
  str output(s);
  StringToUpperASCII(&output);
  return output;
}

// Compare the lower-case form of the given string against the given
// previously-lower-cased ASCII string (typically a constant).
bool LowerCaseEqualsASCII(StringPiece str, StringPiece lowecase_ascii);
bool LowerCaseEqualsASCII(StringPiece16 str, StringPiece lowecase_ascii);

// Performs a case-sensitive string compare. The behavior is undefined if both
// strings are not ASCII.
bool EqualsASCII(const string16& a, const StringPiece& b);
bool EqualsASCII(const std::string& a, const std::string& b, bool case_sensitive);
bool EqualsASCII(const std::vector<unsigned char>& a, const std::vector<unsigned char>& b, bool case_sensitive);
bool EqualsASCII(const std::vector<char>& a, const std::vector<char>& b, bool case_sensitive);

bool FullEqualsASCII(const std::string& a, const std::string& b, bool case_sensitive);
bool FullEqualsASCII(const std::vector<unsigned char>& a, const std::vector<unsigned char>& b, bool case_sensitive);
bool FullEqualsASCII(const std::vector<char>& a, const std::vector<char>& b, bool case_sensitive);

// Returns true if str starts with search, or false otherwise.
bool StartsWithASCII(const std::string& str, const std::string& search, bool case_sensitive);
bool StartsWith(const string16& str, const string16& search, bool case_sensitive);

// Returns true if str ends with search, or false otherwise.
bool EndsWith(const std::string& str, const std::string& search, bool case_sensitive);
bool EndsWith(const string16& str, const string16& search, bool case_sensitive);

// Determines the type of ASCII character, independent of locale (the C
// library versions will change based on locale).
template <typename Char>
inline bool IsAsciiWhitespace(Char c) {
  return c == ' ' || c == '\r' || c == '\n' || c == '\t';
}
template <typename Char>
inline bool IsAsciiAlpha(Char c) {
  return ((c >= 'A') && (c <= 'Z')) || ((c >= 'a') && (c <= 'z'));
}
template <typename Char>
inline bool IsAsciiDigit(Char c) {
  return c >= '0' && c <= '9';
}

template <typename Char>
inline bool IsHexDigit(Char c) {
  return (c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f');
}

template <typename Char>
inline Char HexDigitToInt(Char c) {
  DCHECK(IsHexDigit(c));
  if (c >= '0' && c <= '9') {
    return c - '0';
  }
  if (c >= 'A' && c <= 'F') {
    return c - 'A' + 10;
  }
  if (c >= 'a' && c <= 'f') {
    return c - 'a' + 10;
  }
  return 0;
}

// Returns true if it's a whitespace character.
inline bool IsWhitespace(wchar_t c) {
  return wcschr(kWhitespaceWide, c) != nullptr;
}

// Return a byte string in human-readable format with a unit suffix. Not
// appropriate for use in any UI; use of FormatBytes and friends in ui/base is
// highly recommended instead. TODO(avi): Figure out how to get callers to use
// FormatBytes instead; remove this.
string16 FormatBytesUnlocalized(int64_t bytes);

// Starting at |start_offset| (usually 0), replace the first instance of
// |find_this| with |replace_with|.
void ReplaceFirstSubstringAfterOffset(string16* str,
                                      size_t start_offset,
                                      const string16& find_this,
                                      const string16& replace_with);
void ReplaceFirstSubstringAfterOffset(std::string* str,
                                      size_t start_offset,
                                      const std::string& find_this,
                                      const std::string& replace_with);

// Starting at |start_offset| (usually 0), look through |str| and replace all
// instances of |find_this| with |replace_with|.
//
// This does entire substrings; use std::replace in <algorithm> for single
// characters, for example:
//   std::replace(str.begin(), str.end(), 'a', 'b');
void ReplaceSubstringsAfterOffset(string16* str,
                                  size_t start_offset,
                                  const string16& find_this,
                                  const string16& replace_with);
void ReplaceSubstringsAfterOffset(std::string* str,
                                  size_t start_offset,
                                  const std::string& find_this,
                                  const std::string& replace_with);

// Reserves enough memory in |str| to accommodate |length_with_null| characters,
// sets the size of |str| to |length_with_null - 1| characters, and returns a
// pointer to the underlying contiguous array of characters.  This is typically
// used when calling a function that writes results into a character array, but
// the caller wants the data to be managed by a string-like object.  It is
// convenient in that is can be used inline in the call, and fast in that it
// avoids copying the results of the call from a char* into a string.
//
// |length_with_null| must be at least 2, since otherwise the underlying string
// would have size 0, and trying to access &((*str)[0]) in that case can result
// in a number of problems.
//
// Internally, this takes linear time because the resize() call 0-fills the
// underlying array for potentially all
// (|length_with_null - 1| * sizeof(string_type::value_type)) bytes.  Ideally we
// could avoid this aspect of the resize() call, as we expect the caller to
// immediately write over this memory, but there is no other way to set the size
// of the string, and not doing that will mean people who access |str| rather
// than str.c_str() will get back a string of whatever size |str| had on entry
// to this function (probably 0).
template <class string_type>
inline typename string_type::value_type* WriteInto(string_type* str, size_t length_with_null) {
  DCHECK_GT(length_with_null, 1u);
  str->reserve(length_with_null);
  str->resize(length_with_null - 1);
  return &((*str)[0]);
}

//-----------------------------------------------------------------------------

// Splits a string into its fields delimited by any of the characters in
// |delimiters|.  Each field is added to the |tokens| vector.  Returns the
// number of tokens found.
size_t Tokenize(const string16& str, const string16& delimiters, std::vector<string16>* tokens);
size_t Tokenize(const std::string& str, const std::string& delimiters, std::vector<std::string>* tokens);
size_t Tokenize(const StringPiece& str, const StringPiece& delimiters, std::vector<StringPiece>* tokens);

template <typename CharT, typename T>
inline size_t Tokenize(const std::vector<CharT>& str, const std::vector<CharT>& delimiters, std::vector<T>* tokens) {
  if (!tokens) {
    return 0;
  }
  tokens->clear();

  std::vector<CharT> chunk;
  for (size_t i = 0; i < str.size(); ++i) {
    bool is_last = i == str.size() - 1;
    CharT c = str[i];
    bool found = false;
    for (size_t j = 0; j < delimiters.size(); ++j) {
      if (c == delimiters[j]) {
        found = true;
        break;
      }
    }

    if (!found) {
      chunk.push_back(c);
    }

    if (found || is_last) {  // if founded or last symbol
      if (!chunk.empty()) {
        tokens->push_back(chunk);
      }
      chunk.clear();
    }
  }

  return tokens->size();
}

// Does the opposite of SplitString()/SplitStringPiece(). Joins a vector or list
// of strings into a single string, inserting |separator| (which may be empty)
// in between all elements.
//
// If possible, callers should build a vector of StringPieces and use the
// StringPiece variant, so that they do not create unnecessary copies of
// strings. For example, instead of using SplitString, modifying the vector,
// then using JoinString, use SplitStringPiece followed by JoinString so that no
// copies of those strings are created until the final join operation.
std::string JoinString(const std::vector<std::string>& parts, StringPiece separator);
string16 JoinString(const std::vector<string16>& parts, StringPiece16 separator);
std::string JoinString(const std::vector<StringPiece>& parts, StringPiece separator);
string16 JoinString(const std::vector<StringPiece16>& parts, StringPiece16 separator);

// Explicit initializer_list overloads are required to break ambiguity when used
// with a literal initializer list (otherwise the compiler would not be able to
// decide between the string and StringPiece overloads).
std::string JoinString(std::initializer_list<StringPiece> parts, StringPiece separator);
string16 JoinString(std::initializer_list<StringPiece16> parts, StringPiece16 separator);

// Replace $1-$2-$3..$9 in the format string with |a|-|b|-|c|..|i| respectively.
// Additionally, any number of consecutive '$' characters is replaced by that
// number less one. Eg $$->$, $$$->$$, etc. The offsets parameter here can be
// NULL. This only allows you to use up to nine replacements.
string16 ReplaceStringPlaceholders(const string16& format_string,
                                   const std::vector<string16>& subst,
                                   std::vector<size_t>* offsets);

std::string ReplaceStringPlaceholders(const StringPiece& format_string,
                                      const std::vector<std::string>& subst,
                                      std::vector<size_t>* offsets);

// Single-string shortcut for ReplaceStringHolders. |offset| may be NULL.
string16 ReplaceStringPlaceholders(const string16& format_string, const string16& a, size_t* offset);

// Returns true if the string passed in matches the pattern. The pattern
// string can contain wildcards like * and ?
// The backslash character (\) is an escape character for * and ?
// We limit the patterns to having a max of 16 * or ? characters.
// ? matches 0 or 1 character, while * matches 0 or more characters.
bool MatchPattern(const StringPiece& string, const StringPiece& pattern);
bool MatchPattern(const string16& string, const string16& pattern);

// Hack to convert any char-like type to its unsigned counterpart.
// For example, it will convert char, signed char and unsigned char to unsigned
// char.
template <typename T>
struct ToUnsigned {
  typedef T Unsigned;
};

template <>
struct ToUnsigned<char> {
  typedef unsigned char Unsigned;
};
template <>
struct ToUnsigned<signed char> {
  typedef unsigned char Unsigned;
};
template <>
struct ToUnsigned<wchar_t> {
#if defined(WCHAR_T_IS_UTF16)
  typedef unsigned short Unsigned;
#elif defined(UNICODE)
  typedef uint32_t Unsigned;
#endif
};
template <>
struct ToUnsigned<short> {
  typedef unsigned short Unsigned;
};

}  // namespace common
