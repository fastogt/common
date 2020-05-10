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

#include <common/convert2string.h>

#include <math.h>

#include <algorithm>
#include <limits>

#include <common/portable_endian.h>
#include <common/sprintf.h>
#include <common/string_number_conversions.h>
#include <common/string_util.h>
#include <common/utf_string_conversions.h>  // for UTF16ToUTF8

namespace common {

namespace {

// Utility to convert a character to a digit in a given base
template <typename CHAR, int BASE, bool BASE_LTE_10>
class BaseCharToDigit {};

// Faster specialization for bases <= 10
template <typename CHAR, int BASE>
class BaseCharToDigit<CHAR, BASE, true> {
 public:
  static bool Convert(CHAR c, uint8_t* digit) {
    if (c >= '0' && c < '0' + BASE) {
      *digit = c - '0';
      return true;
    }
    return false;
  }
};

// Specialization for bases where 10 < base <= 36
template <typename CHAR, int BASE>
class BaseCharToDigit<CHAR, BASE, false> {
 public:
  static bool Convert(CHAR c, uint8_t* digit) {
    if (c >= '0' && c <= '9') {
      *digit = c - '0';
    } else if (c >= 'a' && c < 'a' + BASE - 10) {
      *digit = c - 'a' + 10;
    } else if (c >= 'A' && c < 'A' + BASE - 10) {
      *digit = c - 'A' + 10;
    } else {
      return false;
    }
    return true;
  }
};

template <int BASE, typename CHAR>
bool CharToDigit(CHAR c, uint8_t* digit) {
  return BaseCharToDigit<CHAR, BASE, BASE <= 10>::Convert(c, digit);
}

// There is an IsWhitespace for wchars defined in string_util.h, but it is
// locale independent, whereas the functions we are replacing were
// locale-dependent. TBD what is desired, but for the moment let's not introduce
// a change in behaviour.
template <typename CHAR>
class WhitespaceHelper {};

template <>
class WhitespaceHelper<char> {
 public:
  static bool Invoke(char c) { return 0 != isspace(static_cast<unsigned char>(c)); }
};

template <>
class WhitespaceHelper<unsigned char> {
 public:
  static bool Invoke(unsigned char c) { return 0 != isspace(c); }
};

template <>
class WhitespaceHelper<char16> {
 public:
  static bool Invoke(char16 c) { return 0 != iswspace(c); }
};

template <typename CHAR>
bool LocalIsWhitespace(CHAR c) {
  return WhitespaceHelper<CHAR>::Invoke(c);
}

// IteratorRangeToNumberTraits should provide:
//  - a typedef for iterator_type, the iterator type used as input.
//  - a typedef for value_type, the target numeric type.
//  - static functions min, max (returning the minimum and maximum permitted
//    values)
//  - constant kBase, the base in which to interpret the input
template <typename IteratorRangeToNumberTraits>
class IteratorRangeToNumber {
 public:
  typedef IteratorRangeToNumberTraits traits;
  typedef typename traits::iterator_type const_iterator;
  typedef typename traits::value_type value_type;

  // Generalized iterator-range-to-number conversion.
  //
  static bool Invoke(const_iterator begin, const_iterator end, value_type* output) {
    bool valid = true;

    while (begin != end && LocalIsWhitespace(*begin)) {
      valid = false;
      ++begin;
    }

    if (begin != end && *begin == '-') {
      if (!std::numeric_limits<value_type>::is_signed) {
        valid = false;
      } else if (!Negative::Invoke(begin + 1, end, output)) {
        valid = false;
      }
    } else {
      if (begin != end && *begin == '+') {
        ++begin;
      }
      if (!Positive::Invoke(begin, end, output)) {
        valid = false;
      }
    }

    return valid;
  }

 private:
  // Sign provides:
  //  - a static function, CheckBounds, that determines whether the next digit
  //    causes an overflow/underflow
  //  - a static function, Increment, that appends the next digit appropriately
  //    according to the sign of the number being parsed.
  template <typename Sign>
  class Base {
   public:
    static bool Invoke(const_iterator begin, const_iterator end, typename traits::value_type* output) {
      *output = 0;

      if (begin == end) {
        return false;
      }

      // Note: no performance difference was found when using template
      // specialization to remove this check in bases other than 16
      if (traits::kBase == 16 && end - begin > 2 && *begin == '0' && (*(begin + 1) == 'x' || *(begin + 1) == 'X')) {
        begin += 2;
      }

      for (const_iterator current = begin; current != end; ++current) {
        uint8_t new_digit = 0;

        if (!CharToDigit<traits::kBase>(*current, &new_digit)) {
          return false;
        }

        if (current != begin) {
          if (!Sign::CheckBounds(output, new_digit)) {
            return false;
          }
          *output *= traits::kBase;
        }

        Sign::Increment(new_digit, output);
      }
      return true;
    }
  };

  class Positive : public Base<Positive> {
   public:
    static bool CheckBounds(value_type* output, uint8_t new_digit) {
      if (*output > static_cast<value_type>(traits::max() / traits::kBase) ||
          (*output == static_cast<value_type>(traits::max() / traits::kBase) &&
           new_digit > traits::max() % traits::kBase)) {
        *output = traits::max();
        return false;
      }
      return true;
    }
    static void Increment(uint8_t increment, value_type* output) { *output += increment; }
  };

  class Negative : public Base<Negative> {
   public:
    static bool CheckBounds(value_type* output, uint8_t new_digit) {
      if (*output < traits::min() / traits::kBase ||
          (*output == traits::min() / traits::kBase && new_digit > 0 - traits::min() % traits::kBase)) {
        *output = traits::min();
        return false;
      }
      return true;
    }
    static void Increment(uint8_t increment, value_type* output) { *output -= increment; }
  };
};

template <typename ITERATOR, typename VALUE, int BASE>
class BaseIteratorRangeToNumberTraits {
 public:
  typedef ITERATOR iterator_type;
  typedef VALUE value_type;
  static value_type min() { return std::numeric_limits<value_type>::min(); }
  static value_type max() { return std::numeric_limits<value_type>::max(); }
  static const int kBase = BASE;
};

template <typename ITERATOR>
class BaseHexIteratorRangeToIntTraits : public BaseIteratorRangeToNumberTraits<ITERATOR, int32_t, 16> {};

template <typename ITERATOR>
class BaseHexIteratorRangeToUIntTraits : public BaseIteratorRangeToNumberTraits<ITERATOR, uint32_t, 16> {};

template <typename ITERATOR>
class BaseHexIteratorRangeToInt64Traits : public BaseIteratorRangeToNumberTraits<ITERATOR, int64_t, 16> {};

template <typename ITERATOR>
class BaseHexIteratorRangeToUInt64Traits : public BaseIteratorRangeToNumberTraits<ITERATOR, uint64_t, 16> {};

typedef BaseHexIteratorRangeToIntTraits<StringPiece::const_iterator> HexIteratorRangeToIntTraits;

typedef BaseHexIteratorRangeToUIntTraits<StringPiece::const_iterator> HexIteratorRangeToUIntTraits;

typedef BaseHexIteratorRangeToInt64Traits<StringPiece::const_iterator> HexIteratorRangeToInt64Traits;

typedef BaseHexIteratorRangeToUInt64Traits<StringPiece::const_iterator> HexIteratorRangeToUInt64Traits;

template <typename VALUE, int BASE>
class StringPieceToNumberTraits : public BaseIteratorRangeToNumberTraits<StringPiece::const_iterator, VALUE, BASE> {};

template <typename VALUE>
bool DoStringToInt(const StringPiece& input, VALUE* output) {
  return IteratorRangeToNumber<StringPieceToNumberTraits<VALUE, 10>>::Invoke(input.begin(), input.end(), output);
}

template <typename VALUE, int BASE>
class StringPiece16ToNumberTraits : public BaseIteratorRangeToNumberTraits<StringPiece16::const_iterator, VALUE, BASE> {
};

template <typename VALUE>
bool DoString16ToInt(const StringPiece16& input, VALUE* output) {
  return IteratorRangeToNumber<StringPiece16ToNumberTraits<VALUE, 10>>::Invoke(input.begin(), input.end(), output);
}

template <typename BUFFER, typename VALUE, int BASE>
class BytesToNumberTraits : public BaseIteratorRangeToNumberTraits<typename BUFFER::const_iterator, VALUE, BASE> {};

template <typename BUFFER, typename VALUE>
bool DoBytesToInt(const BUFFER& input, VALUE* output) {
  return IteratorRangeToNumber<BytesToNumberTraits<BUFFER, VALUE, 10>>::Invoke(input.begin(), input.end(), output);
}

template <typename STR>
bool UnicodeStringToBytesT(const STR& input, std::vector<uint16_t>* output) {
  DCHECK_EQ(output->size(), 0u);
  size_t count = input.size();
  if (count == 0 || (count % 4) != 0) {
    return false;
  }

  for (size_t i = 0; i < count / 4; ++i) {
    uint8_t c0 = 0;  // most significant 4 bits
    uint8_t c1 = 0;  // least significant 4 bits
    uint8_t c2 = 0;  // most significant 4 bits
    uint8_t c3 = 0;  // least significant 4 bits
    if (!CharToDigit<16>(input[i * 4], &c0) || !CharToDigit<16>(input[i * 4 + 1], &c1) ||
        !CharToDigit<16>(input[i * 4 + 2], &c2) || !CharToDigit<16>(input[i * 4 + 3], &c3)) {
      return false;
    }

    uint16_t val = (c0 << 12) | (c1 << 8) | (c2 << 4) | c3;
    output->push_back(val);
  }
  return true;
}

template <typename STR>
bool UUnicodeStringToBytesT(const STR& input, std::vector<uint16_t>* output) {
  DCHECK_EQ(output->size(), 0u);
  size_t count = input.size();
  if (count == 0 || (count % 6) != 0) {
    return false;
  }

  for (size_t i = 0; i < count / 6; ++i) {
    uint8_t c0 = 0;  // most significant 4 bits
    uint8_t c1 = 0;  // least significant 4 bits
    uint8_t c2 = 0;  // most significant 4 bits
    uint8_t c3 = 0;  // least significant 4 bits
    if (!CharToDigit<16>(input[i * 6 + 2], &c0) || !CharToDigit<16>(input[i * 6 + 3], &c1) ||
        !CharToDigit<16>(input[i * 6 + 4], &c2) || !CharToDigit<16>(input[i * 6 + 5], &c3)) {
      return false;
    }

    uint16_t val = (c0 << 12) | (c1 << 8) | (c2 << 4) | c3;
    output->push_back(val);
  }
  return true;
}

template <typename T, typename U>
bool do_unicode_decode(const T& input, U* out) {
  if (!out) {
    return false;
  }

  std::vector<uint16_t> vec;
  bool res = UnicodeStringToBytesT(input, &vec);
  if (!res) {
    return false;
  }

  *out = U(vec.begin(), vec.end());
  return true;
}

template <typename T, typename U>
bool do_uunicode_decode(const T& input, U* out) {
  if (!out) {
    return false;
  }

  std::vector<uint16_t> vec;
  bool res = UUnicodeStringToBytesT(input, &vec);
  if (!res) {
    return false;
  }

  *out = U(vec.begin(), vec.end());
  return true;
}

template <typename T, typename U>
bool do_hex_encode(const T& input, bool is_lower, U* out) {
  if (!out) {
    return false;
  }

  static const char uHexChars[] = "0123456789ABCDEF";
  static const char lHexChars[] = "0123456789abcdef";

  typedef typename T::value_type value_type;
  const typename T::size_type size = input.size();
  U decoded;
  decoded.resize(size * 2);

  for (size_t i = 0; i < size; ++i) {
    value_type b = input[i];
    if (is_lower) {
      decoded[(i * 2)] = lHexChars[(b >> 4) & 0xf];
      decoded[(i * 2) + 1] = lHexChars[b & 0xf];
    } else {
      decoded[(i * 2)] = uHexChars[(b >> 4) & 0xf];
      decoded[(i * 2) + 1] = uHexChars[b & 0xf];
    }
  }

  *out = decoded;
  return true;
}

template <typename T, typename U>
bool do_xhex_encode(const T& input, bool is_lower, U* out) {
  if (!out) {
    return false;
  }

  static const char uHexChars[] = "0123456789ABCDEF";
  static const char lHexChars[] = "0123456789abcdef";

  typedef typename T::value_type value_type;
  const typename T::size_type size = input.size();
  U decoded;
  decoded.resize(size * 4);

  for (size_t i = 0; i < size; ++i) {
    value_type b = input[i];
    decoded[(i * 4)] = '\\';
    if (is_lower) {
      decoded[(i * 4) + 1] = 'x';
      decoded[(i * 4) + 2] = lHexChars[(b >> 4) & 0xf];
      decoded[(i * 4) + 3] = lHexChars[b & 0xf];
    } else {
      decoded[(i * 4) + 1] = 'X';
      decoded[(i * 4) + 2] = uHexChars[(b >> 4) & 0xf];
      decoded[(i * 4) + 3] = uHexChars[b & 0xf];
    }
  }

  *out = decoded;
  return true;
}

template <typename STR>
bool HexStringToBytesT(const STR& input, std::vector<uint8_t>* output) {
  DCHECK_EQ(output->size(), 0u);
  size_t count = input.size();
  if (count == 0 || (count % 2) != 0) {
    return false;
  }
  for (uintptr_t i = 0; i < count / 2; ++i) {
    uint8_t msb = 0;  // most significant 4 bits
    uint8_t lsb = 0;  // least significant 4 bits
    if (!CharToDigit<16>(input[i * 2], &msb) || !CharToDigit<16>(input[i * 2 + 1], &lsb)) {
      return false;
    }
    output->push_back((msb << 4) | lsb);
  }
  return true;
}

template <typename STR>
bool XHexStringToBytesT(const STR& input, std::vector<uint8_t>* output) {
  DCHECK_EQ(output->size(), 0u);
  size_t count = input.size();
  if (count == 0 || (count % 4) != 0) {
    return false;
  }
  for (uintptr_t i = 0; i < count / 4; ++i) {
    uint8_t msb = 0;  // most significant 4 bits
    uint8_t lsb = 0;  // least significant 4 bits
    if (!CharToDigit<16>(input[i * 4 + 2], &msb) || !CharToDigit<16>(input[i * 4 + 3], &lsb)) {
      return false;
    }
    output->push_back((msb << 4) | lsb);
  }
  return true;
}

template <typename T, typename U>
bool do_hex_decode(const T& input, U* out) {
  if (!out) {
    return false;
  }

  std::vector<uint8_t> vec;
  bool res = HexStringToBytesT(input, &vec);
  if (!res) {
    return false;
  }

  *out = U(vec.begin(), vec.end());
  return true;
}

template <typename T, typename U>
bool do_xhex_decode(const T& input, U* out) {
  if (!out) {
    return false;
  }

  std::vector<uint8_t> vec;
  bool res = XHexStringToBytesT(input, &vec);
  if (!res) {
    return false;
  }

  *out = U(vec.begin(), vec.end());
  return true;
}

template <typename T, typename U>
bool do_unicode_encode(const T& input, bool is_lower, U* out) {
  if (!out) {
    return false;
  }

  static const char uHexChars[] = "0123456789ABCDEF";
  static const char lHexChars[] = "0123456789abcdef";

  typedef typename T::value_type value_type;
  const typename T::size_type size = input.size();
  U decoded;
  decoded.resize(size * 4);

  for (size_t i = 0; i < size; ++i) {
    value_type b = htobe16(input[i]);
    uint8_t msb = b;       // most significant 4 bits
    uint8_t lsb = b >> 8;  // least significant 4 bits
    if (is_lower) {
      decoded[(i * 4)] = lHexChars[(msb >> 4) & 0xf];
      decoded[(i * 4) + 1] = lHexChars[(msb & 0xf) & 0xf];
      decoded[(i * 4) + 2] = lHexChars[(lsb >> 4) & 0xf];
      decoded[(i * 4) + 3] = lHexChars[lsb & 0xf];
    } else {
      decoded[(i * 4)] = uHexChars[(msb >> 4) & 0xf];
      decoded[(i * 4) + 1] = uHexChars[(msb & 0xf) & 0xf];
      decoded[(i * 4) + 2] = uHexChars[(lsb >> 4) & 0xf];
      decoded[(i * 4) + 3] = uHexChars[lsb & 0xf];
    }
  }

  *out = decoded;
  return true;
}

template <typename T, typename U>
bool do_uunicode_encode(const T& input, bool is_lower, U* out) {
  if (!out) {
    return false;
  }

  static const char uHexChars[] = "0123456789ABCDEF";
  static const char lHexChars[] = "0123456789abcdef";

  typedef typename T::value_type value_type;
  const typename T::size_type size = input.size();
  U decoded;
  decoded.resize(size * 6);

  for (size_t i = 0; i < size; ++i) {
    value_type b = htobe16(input[i]);
    uint8_t msb = b;       // most significant 4 bits
    uint8_t lsb = b >> 8;  // least significant 4 bits
    decoded[(i * 6)] = '\\';
    if (is_lower) {
      decoded[(i * 6) + 1] = 'u';
      decoded[(i * 6) + 2] = lHexChars[(msb >> 4) & 0xf];
      decoded[(i * 6) + 3] = lHexChars[(msb & 0xf) & 0xf];
      decoded[(i * 6) + 4] = lHexChars[(lsb >> 4) & 0xf];
      decoded[(i * 6) + 5] = lHexChars[lsb & 0xf];
    } else {
      decoded[(i * 6) + 1] = 'U';
      decoded[(i * 6) + 2] = uHexChars[(msb >> 4) & 0xf];
      decoded[(i * 6) + 3] = uHexChars[(msb & 0xf) & 0xf];
      decoded[(i * 6) + 4] = uHexChars[(lsb >> 4) & 0xf];
      decoded[(i * 6) + 5] = uHexChars[lsb & 0xf];
    }
  }

  *out = decoded;
  return true;
}

template <typename Buffer>
Buffer ConvertToBytesT(bool value) {
  if (value) {
    return {'t', 'r', 'u', 'e'};
  }
  return {'f', 'a', 'l', 's', 'e'};
}

template <typename Buffer>
Buffer ConvertToBytesT(char value) {
  const std::string str = ConvertToString(value);  // more readable
  return Buffer(str.begin(), str.end());
}

template <typename Buffer>
Buffer ConvertToBytesT(unsigned char value) {
  const std::string str = ConvertToString(value);  // more readable
  return Buffer(str.begin(), str.end());
}

template <typename Buffer>
Buffer ConvertToBytesT(short value) {
  const std::string str = ConvertToString(value);  // more readable
  return Buffer(str.begin(), str.end());
}

template <typename Buffer>
Buffer ConvertToBytesT(unsigned short value) {
  const std::string str = ConvertToString(value);  // more readable
  return Buffer(str.begin(), str.end());
}

template <typename Buffer>
Buffer ConvertToBytesT(int value) {
  const std::string str = ConvertToString(value);  // more readable
  return Buffer(str.begin(), str.end());
}

template <typename Buffer>
Buffer ConvertToBytesT(unsigned int value) {
  const std::string str = ConvertToString(value);  // more readable
  return Buffer(str.begin(), str.end());
}

template <typename Buffer>
Buffer ConvertToBytesT(long value) {
  const std::string str = ConvertToString(value);  // more readable
  return Buffer(str.begin(), str.end());
}

template <typename Buffer>
Buffer ConvertToBytesT(unsigned long value) {
  const std::string str = ConvertToString(value);  // more readable
  return Buffer(str.begin(), str.end());
}

template <typename Buffer>
Buffer ConvertToBytesT(long long value) {
  const std::string str = ConvertToString(value);  // more readable
  return Buffer(str.begin(), str.end());
}

template <typename Buffer>
Buffer ConvertToBytesT(unsigned long long value) {
  const std::string str = ConvertToString(value);  // more readable
  return Buffer(str.begin(), str.end());
}

template <typename Buffer>
Buffer ConvertToBytesT(float value, int prec) {
  const std::string str = ConvertToString(value, prec);  // more readable
  return Buffer(str.begin(), str.end());
}

template <typename Buffer>
Buffer ConvertToBytesT(double value, int prec) {
  const std::string str = ConvertToString(value, prec);  // more readable
  return Buffer(str.begin(), str.end());
}

}  // namespace

std::string EscapedText(const std::string& str) {
  if (!str.empty() && str[str.length() - 1] != '\n') {
    return str + "\n";
  }

  return str;
}

string16 ConvertToString16(const char* from) {
  return ConvertToString16(std::string(from));
}

string16 ConvertToString16(const std::string& from) {
#if defined(WCHAR_T_IS_UTF16)
  return UTF8ToWide(from);
#elif defined(WCHAR_T_IS_UTF32)
  return UTF8ToUTF16(from);
#endif
}

string16 ConvertToString16(const StringPiece& from) {
#if defined(WCHAR_T_IS_UTF16)
  return UTF8ToWide(from);
#elif defined(WCHAR_T_IS_UTF32)
  return UTF8ToUTF16(from);
#endif
}

template <typename ch>
string16 ConvertToString16(const ByteArray<ch>& from) {
  std::string str = ConvertToString(from);
  return ConvertToString16(str);
}

string16 ConvertToString16(const StringPiece16& from) {
  return from.as_string();
}

string16 ConvertToString16(bool value) {
  return value ? ASCIIToUTF16("true") : ASCIIToUTF16("false");
}

string16 ConvertToString16(char value) {
  return NumberToString16(value);
}

string16 ConvertToString16(unsigned char value) {
  return NumberToString16(value);
}

string16 ConvertToString16(short value) {
  return NumberToString16(value);
}

string16 ConvertToString16(unsigned short value) {
  return NumberToString16(value);
}

string16 ConvertToString16(int value) {
  return NumberToString16(value);
}

string16 ConvertToString16(unsigned int value) {
  return NumberToString16(value);
}

string16 ConvertToString16(long value) {
  return NumberToString16(value);
}

string16 ConvertToString16(unsigned long value) {
  return NumberToString16(value);
}

string16 ConvertToString16(long long value) {
  return NumberToString16(value);
}

string16 ConvertToString16(unsigned long long value) {
  return NumberToString16(value);
}

string16 ConvertToString16(float value) {
  std::string str = ConvertToString(value);
  return ConvertToString16(str);
}

string16 ConvertToString16(double value) {
  std::string str = ConvertToString(value);
  return ConvertToString16(str);
}

//
bool ConvertFromString16(const string16& from, StringPiece* out) {
  if (!out) {
    return false;
  }
#if defined(WCHAR_T_IS_UTF16)
  *out = WideToUTF8(from);
#elif defined(WCHAR_T_IS_UTF32)
  *out = UTF16ToUTF8(from);
#endif
  return true;
}

bool ConvertFromString16(const string16& from, std::string* out) {
  if (!out) {
    return false;
  }
#if defined(WCHAR_T_IS_UTF16)
  *out = WideToUTF8(from);
#elif defined(WCHAR_T_IS_UTF32)
  *out = UTF16ToUTF8(from);
#endif
  return true;
}

template <typename ch>
bool ConvertFromString16(const string16& from, ByteArray<ch>* out) {
  if (!out) {
    return false;
  }

  std::string ascii;
  bool res = ConvertFromString16(from, &ascii);
  if (!res) {
    return false;
  }

  *out = ByteArray<ch>(ascii.begin(), ascii.end());
  return true;
}

bool ConvertFromString16(const string16& from, bool* out) {
  if (!out) {
    return false;
  }

  string16 copy = from;
  std::transform(copy.begin(), copy.end(), copy.begin(), ::tolower);
  *out = copy == ASCIIToUTF16("true");
  return true;
}

bool ConvertFromString16(const string16& from, char* out) {
  if (!out) {
    return false;
  }

  char loutput = 0;
  bool res = DoString16ToInt(from, &loutput);
  if (!res) {
    return false;
  }

  *out = loutput;
  return true;
}

bool ConvertFromString16(const string16& from, unsigned char* out) {
  if (!out) {
    return false;
  }

  unsigned char loutput = 0;
  bool res = DoString16ToInt(from, &loutput);
  if (!res) {
    return false;
  }

  *out = loutput;
  return true;
}

bool ConvertFromString16(const string16& from, short* out) {
  if (!out) {
    return false;
  }

  short loutput = 0;
  bool res = DoString16ToInt(from, &loutput);
  if (!res) {
    return false;
  }

  *out = loutput;
  return true;
}

bool ConvertFromString16(const string16& from, unsigned short* out) {
  if (!out) {
    return false;
  }

  unsigned short loutput = 0;
  bool res = DoString16ToInt(from, &loutput);
  if (!res) {
    return false;
  }

  *out = loutput;
  return true;
}

bool ConvertFromString16(const string16& from, int* out) {
  if (!out) {
    return false;
  }

  int loutput = 0;
  bool res = DoString16ToInt(from, &loutput);
  if (!res) {
    return false;
  }

  *out = loutput;
  return true;
}

bool ConvertFromString16(const string16& from, unsigned int* out) {
  if (!out) {
    return false;
  }

  unsigned int loutput = 0;
  bool res = DoString16ToInt(from, &loutput);
  if (!res) {
    return false;
  }

  *out = loutput;
  return true;
}

bool ConvertFromString16(const string16& from, long* out) {
  if (!out) {
    return false;
  }

  long loutput = 0;
  bool res = DoString16ToInt(from, &loutput);
  if (!res) {
    return false;
  }

  *out = loutput;
  return true;
}

bool ConvertFromString16(const string16& from, unsigned long* out) {
  if (!out) {
    return false;
  }

  unsigned long loutput = 0;
  bool res = DoString16ToInt(from, &loutput);
  if (!res) {
    return false;
  }

  *out = loutput;
  return true;
}

bool ConvertFromString16(const string16& from, long long* out) {
  if (!out) {
    return false;
  }

  long long loutput = 0;
  bool res = DoString16ToInt(from, &loutput);
  if (!res) {
    return false;
  }

  *out = loutput;
  return true;
}

bool ConvertFromString16(const string16& from, unsigned long long* out) {
  if (!out) {
    return false;
  }

  unsigned long long loutput = 0;
  bool res = DoString16ToInt(from, &loutput);
  if (!res) {
    return false;
  }

  *out = loutput;
  return true;
}

bool ConvertFromString16(const string16& from, float* out) {
  if (!out) {
    return false;
  }

  std::string ascii;
  if (!ConvertFromString16(from, &ascii)) {
    return false;
  }

  return ConvertFromString(ascii, out);
}

bool ConvertFromString16(const string16& from, double* out) {
  if (!out) {
    return false;
  }

  std::string ascii;
  if (!ConvertFromString16(from, &ascii)) {
    return false;
  }

  return ConvertFromString(ascii, out);
}

bool ConvertFromString16(const string16& from, StringPiece16* out) {
  if (!out) {
    return false;
  }

  *out = from;
  return true;
}

// std string

std::string ConvertToString(const char* from) {
  return from;
}

template <typename ch>
std::string ConvertToString(const ByteArray<ch>& from) {
  return std::string(from.begin(), from.end());
}

std::string ConvertToString(const string16& from) {
  std::string ascii;
  if (!ConvertFromString16(from, &ascii)) {
    return std::string();
  }

  return ascii;
}

std::string ConvertToString(const StringPiece16& from) {
  std::string ascii;
  string16 s16 = ConvertToString16(from);
  if (!ConvertFromString16(s16, &ascii)) {
    return std::string();
  }

  return ascii;
}

std::string ConvertToString(const StringPiece& from) {
  return from.as_string();
}

std::string ConvertToString(bool value) {
  return value ? "true" : "false";
}

std::string ConvertToString(char value) {
  return NumberToString(value);
}

std::string ConvertToString(unsigned char value) {
  return NumberToString(value);
}

std::string ConvertToString(short value) {
  return NumberToString(value);
}

std::string ConvertToString(unsigned short value) {
  return NumberToString(value);
}

std::string ConvertToString(int value) {
  return NumberToString(value);
}

std::string ConvertToString(unsigned int value) {
  return NumberToString(value);
}

std::string ConvertToString(long value) {
  return NumberToString(value);
}

std::string ConvertToString(unsigned long value) {
  return NumberToString(value);
}

std::string ConvertToString(long long value) {
  return NumberToString(value);
}

std::string ConvertToString(unsigned long long value) {
  return NumberToString(value);
}

std::string ConvertToString(float value, int prec) {
  return NumberToString(value, prec);
}

std::string ConvertToString(double value, int prec) {
  return NumberToString(value, prec);
}

bool ConvertFromString(const std::string& from, string16* out) {
  if (!out) {
    return false;
  }

  *out = ConvertToString16(from);
  return true;
}

#if defined(WCHAR_T_IS_UTF16)
#else
bool ConvertFromString(const std::string& from, std::wstring* out) {
  if (!out) {
    return false;
  }

  *out = UTF8ToWide(from);
  return true;
}
#endif

template <typename ch>
bool ConvertFromString(const std::string& from, ByteArray<ch>* out) {
  if (!out || from.empty()) {
    return false;
  }

  const char* cstr = from.c_str();
  *out = ByteArray<ch>(cstr, cstr + from.size());
  return true;
}

bool ConvertFromString(const std::string& from, bool* out) {
  if (!out) {
    return false;
  }

  std::string copy = from;
  std::transform(copy.begin(), copy.end(), copy.begin(), ::tolower);
  *out = copy == "true";
  return true;
}

bool ConvertFromString(const std::string& from, char* out) {
  if (!out) {
    return false;
  }

  char loutput = 0;
  bool res = DoStringToInt(from, &loutput);
  if (!res) {
    return false;
  }

  *out = loutput;
  return true;
}

bool ConvertFromString(const std::string& from, unsigned char* out) {
  if (!out) {
    return false;
  }

  unsigned char loutput = 0;
  bool res = DoStringToInt(from, &loutput);
  if (!res) {
    return false;
  }

  *out = loutput;
  return true;
}

bool ConvertFromString(const std::string& from, short* out) {
  if (!out) {
    return false;
  }

  short loutput = 0;
  bool res = DoStringToInt(from, &loutput);
  if (!res) {
    return false;
  }

  *out = loutput;
  return true;
}

bool ConvertFromString(const std::string& from, unsigned short* out) {
  if (!out) {
    return false;
  }

  unsigned short loutput = 0;
  bool res = DoStringToInt(from, &loutput);
  if (!res) {
    return false;
  }

  *out = loutput;
  return true;
}

bool ConvertFromString(const std::string& from, int* out) {
  if (!out) {
    return false;
  }

  int loutput = 0;
  bool res = DoStringToInt(from, &loutput);
  if (!res) {
    return false;
  }

  *out = loutput;
  return true;
}

bool ConvertFromString(const std::string& from, unsigned int* out) {
  if (!out) {
    return false;
  }

  unsigned int loutput = 0;
  bool res = DoStringToInt(from, &loutput);
  if (!res) {
    return false;
  }

  *out = loutput;
  return true;
}

bool ConvertFromString(const std::string& from, long* out) {
  if (!out) {
    return false;
  }

  long loutput = 0;
  bool res = DoStringToInt(from, &loutput);
  if (!res) {
    return false;
  }

  *out = loutput;
  return true;
}

bool ConvertFromString(const std::string& from, unsigned long* out) {
  if (!out) {
    return false;
  }

  unsigned long loutput = 0;
  bool res = DoStringToInt(from, &loutput);
  if (!res) {
    return false;
  }

  *out = loutput;
  return true;
}

bool ConvertFromString(const std::string& from, long long* out) {
  if (!out) {
    return false;
  }

  long long loutput = 0;
  bool res = DoStringToInt(from, &loutput);
  if (!res) {
    return false;
  }

  *out = loutput;
  return true;
}

bool ConvertFromString(const std::string& from, unsigned long long* out) {
  if (!out) {
    return false;
  }

  unsigned long long loutput = 0;
  bool res = DoStringToInt(from, &loutput);
  if (!res) {
    return false;
  }

  *out = loutput;
  return true;
}

bool ConvertFromString(const std::string& from, float* out) {
  if (!out) {
    return false;
  }

  if (from == PLUS_INF || from == PPLUS_INF) {
    *out = std::numeric_limits<float>::infinity();
    return true;
  }

  if (from == MINUS_INF) {
    *out = -std::numeric_limits<float>::infinity();
    return true;
  }

  size_t end_number_pos;
  float d = std::stof(from.c_str(), &end_number_pos);
  if (end_number_pos != from.size()) {
    return false;
  }
  *out = d;
  return true;
}

bool ConvertFromString(const std::string& from, double* out) {
  if (!out) {
    return false;
  }

  if (from == PLUS_INF || from == PPLUS_INF) {
    *out = std::numeric_limits<double>::infinity();
    return true;
  }

  if (from == MINUS_INF) {
    *out = -std::numeric_limits<double>::infinity();
    return true;
  }

  size_t end_number_pos;
  double d = std::stod(from.c_str(), &end_number_pos);
  if (end_number_pos != from.size()) {
    return false;
  }
  *out = d;
  return true;
}

bool ConvertFromString(const std::string& from, StringPiece* out) {
  if (!out) {
    return false;
  }

  *out = from;
  return true;
}

// buffer_t

template <typename Buffer>
Buffer ConvertToBytesT(const std::string& from) {
  Buffer buff;
  if (!ConvertFromString(from, &buff)) {
    return Buffer();
  }

  return buff;
}

template <typename Buffer>
Buffer ConvertToBytesT(const string16& from) {
  Buffer buff;
  if (!ConvertFromString16(from, &buff)) {
    return Buffer();
  }

  return buff;
}

buffer_t ConvertToBytes(const char* from) {
  return MAKE_BUFFER_SIZE(from, strlen(from));
}

buffer_t ConvertToBytes(const std::string& from) {
  return ConvertToBytesT<buffer_t>(from);
}

buffer_t ConvertToBytes(const string16& from) {
  return ConvertToBytesT<buffer_t>(from);
}

buffer_t ConvertToBytes(const char_buffer_t& from) {
  return buffer_t(from.begin(), from.end());
}

buffer_t ConvertToBytes(bool value) {
  return ConvertToBytesT<buffer_t>(value);
}

buffer_t ConvertToBytes(char value) {
  return ConvertToBytesT<buffer_t>(value);
}

buffer_t ConvertToBytes(unsigned char value) {
  return ConvertToBytesT<buffer_t>(value);
}

buffer_t ConvertToBytes(short value) {
  return ConvertToBytesT<buffer_t>(value);
}

buffer_t ConvertToBytes(unsigned short value) {
  return ConvertToBytesT<buffer_t>(value);
}

buffer_t ConvertToBytes(int value) {
  return ConvertToBytesT<buffer_t>(value);
}

buffer_t ConvertToBytes(unsigned int value) {
  return ConvertToBytesT<buffer_t>(value);
}

buffer_t ConvertToBytes(long value) {
  return ConvertToBytesT<buffer_t>(value);
}

buffer_t ConvertToBytes(unsigned long value) {
  return ConvertToBytesT<buffer_t>(value);
}

buffer_t ConvertToBytes(long long value) {
  return ConvertToBytesT<buffer_t>(value);
}

buffer_t ConvertToBytes(unsigned long long value) {
  return ConvertToBytesT<buffer_t>(value);
}

buffer_t ConvertToBytes(float value, int prec) {
  return ConvertToBytesT<buffer_t>(value, prec);
}

buffer_t ConvertToBytes(double value, int prec) {
  return ConvertToBytesT<buffer_t>(value, prec);
}

char_buffer_t ConvertToCharBytes(const char* from) {
  return MAKE_CHAR_BUFFER_SIZE(from, strlen(from));
}

char_buffer_t ConvertToCharBytes(const std::string& from) {
  return ConvertToBytesT<char_buffer_t>(from);
}

char_buffer_t ConvertToCharBytes(const string16& from) {
  return ConvertToBytesT<char_buffer_t>(from);
}

char_buffer_t ConvertToCharBytes(const buffer_t& from) {
  return char_buffer_t(from.begin(), from.end());
}

char_buffer_t ConvertToCharBytes(bool value) {
  return ConvertToBytesT<char_buffer_t>(value);
}

char_buffer_t ConvertToCharBytes(char value) {
  return ConvertToBytesT<char_buffer_t>(value);
}

char_buffer_t ConvertToCharBytes(unsigned char value) {
  return ConvertToBytesT<char_buffer_t>(value);
}

char_buffer_t ConvertToCharBytes(short value) {
  return ConvertToBytesT<char_buffer_t>(value);
}

char_buffer_t ConvertToCharBytes(unsigned short value) {
  return ConvertToBytesT<char_buffer_t>(value);
}

char_buffer_t ConvertToCharBytes(int value) {
  return ConvertToBytesT<char_buffer_t>(value);
}

char_buffer_t ConvertToCharBytes(unsigned int value) {
  return ConvertToBytesT<char_buffer_t>(value);
}

char_buffer_t ConvertToCharBytes(long value) {
  return ConvertToBytesT<char_buffer_t>(value);
}

char_buffer_t ConvertToCharBytes(unsigned long value) {
  return ConvertToBytesT<char_buffer_t>(value);
}

char_buffer_t ConvertToCharBytes(long long value) {
  return ConvertToBytesT<char_buffer_t>(value);
}

char_buffer_t ConvertToCharBytes(unsigned long long value) {
  return ConvertToBytesT<char_buffer_t>(value);
}

char_buffer_t ConvertToCharBytes(float value, int prec) {
  return ConvertToBytesT<char_buffer_t>(value, prec);
}

char_buffer_t ConvertToCharBytes(double value, int prec) {
  return ConvertToBytesT<char_buffer_t>(value, prec);
}

template <typename ch>
bool ConvertFromBytes(const ByteArray<ch>& from, string16* out) {
  if (!out) {
    return false;
  }

  const ch* cstr = from.data();
  *out = string16(cstr, cstr + from.size());
  return true;
}

template <typename ch>
bool ConvertFromBytes(const ByteArray<ch>& from, std::string* out) {
  if (!out) {
    return false;
  }

  *out = from.as_string();
  return true;
}

template <typename ch>
bool ConvertFromBytes(const ByteArray<ch>& from, bool* out) {
  if (!out) {
    return false;
  }

  ByteArray<ch> copy = from;
  std::transform(copy.begin(), copy.end(), copy.begin(), ::tolower);
  bool res = copy == ByteArray<ch>{'t', 'r', 'u', 'e'};
  *out = res;
  return true;
}

template <typename ch>
bool ConvertFromBytes(const ByteArray<ch>& from, char* out) {
  if (!out) {
    return false;
  }

  char loutput = 0;
  bool res = DoBytesToInt(from, &loutput);
  if (!res) {
    return false;
  }

  *out = loutput;
  return true;
}

template <typename ch>
bool ConvertFromBytes(const ByteArray<ch>& from, unsigned char* out) {
  if (!out) {
    return false;
  }

  unsigned char loutput = 0;
  bool res = DoBytesToInt(from, &loutput);
  if (!res) {
    return false;
  }

  *out = loutput;
  return true;
}

template <typename ch>
bool ConvertFromBytes(const ByteArray<ch>& from, short* out) {
  if (!out) {
    return false;
  }

  short loutput = 0;
  bool res = DoBytesToInt(from, &loutput);
  if (!res) {
    return false;
  }

  *out = loutput;
  return true;
}

template <typename ch>
bool ConvertFromBytes(const ByteArray<ch>& from, unsigned short* out) {
  if (!out) {
    return false;
  }

  unsigned short loutput = 0;
  bool res = DoBytesToInt(from, &loutput);
  if (!res) {
    return false;
  }

  *out = loutput;
  return true;
}

template <typename ch>
bool ConvertFromBytes(const ByteArray<ch>& from, int* out) {
  if (!out) {
    return false;
  }

  int loutput = 0;
  bool res = DoBytesToInt(from, &loutput);
  if (!res) {
    return false;
  }

  *out = loutput;
  return true;
}

template <typename ch>
bool ConvertFromBytes(const ByteArray<ch>& from, unsigned int* out) {
  if (!out) {
    return false;
  }

  unsigned int loutput = 0;
  bool res = DoBytesToInt(from, &loutput);
  if (!res) {
    return false;
  }

  *out = loutput;
  return true;
}

template <typename ch>
bool ConvertFromBytes(const ByteArray<ch>& from, long* out) {
  if (!out) {
    return false;
  }

  long loutput = 0;
  bool res = DoBytesToInt(from, &loutput);
  if (!res) {
    return false;
  }

  *out = loutput;
  return true;
}

template <typename ch>
bool ConvertFromBytes(const ByteArray<ch>& from, unsigned long* out) {
  if (!out) {
    return false;
  }

  unsigned long loutput = 0;
  bool res = DoBytesToInt(from, &loutput);
  if (!res) {
    return false;
  }

  *out = loutput;
  return true;
}

template <typename ch>
bool ConvertFromBytes(const ByteArray<ch>& from, long long* out) {
  if (!out) {
    return false;
  }

  long long loutput = 0;
  bool res = DoBytesToInt(from, &loutput);
  if (!res) {
    return false;
  }

  *out = loutput;
  return true;
}

template <typename ch>
bool ConvertFromBytes(const ByteArray<ch>& from, unsigned long long* out) {
  if (!out) {
    return false;
  }

  unsigned long long loutput = 0;
  bool res = DoBytesToInt(from, &loutput);
  if (!res) {
    return false;
  }

  *out = loutput;
  return true;
}

template <typename ch>
bool ConvertFromBytes(const ByteArray<ch>& from, float* out) {
  return ConvertFromString(ConvertToString(from), out);  // more readable
}

template <typename ch>
bool ConvertFromBytes(const ByteArray<ch>& from, double* out) {
  return ConvertFromString(ConvertToString(from), out);  // more readable
}

//

namespace utils {
namespace hex {

bool encode(const char_buffer_t& input, bool is_lower, char_buffer_t* out) {
  return do_hex_encode(input, is_lower, out);
}

bool encode(const StringPiece& input, bool is_lower, char_buffer_t* out) {
  return do_hex_encode(input, is_lower, out);
}

bool decode(const char_buffer_t& input, char_buffer_t* out) {
  return do_hex_decode(input, out);
}

bool decode(const StringPiece& input, char_buffer_t* out) {
  return do_hex_decode(input, out);
}

bool encode(const StringPiece& input, bool is_lower, std::string* out) {
  return do_hex_encode(input, is_lower, out);
}

bool encode(const char_buffer_t& input, bool is_lower, std::string* out) {
  return do_hex_encode(input, is_lower, out);
}

}  // namespace hex

namespace xhex {

bool encode(const StringPiece& input, bool is_lower, char_buffer_t* out) {
  return do_xhex_encode(input, is_lower, out);
}

bool decode(const StringPiece& input, char_buffer_t* out) {
  return do_xhex_decode(input, out);
}

bool encode(const char_buffer_t& input, bool is_lower, char_buffer_t* out) {
  return do_xhex_encode(input, is_lower, out);
}

bool decode(const char_buffer_t& input, char_buffer_t* out) {
  return do_xhex_decode(input, out);
}

bool encode(const StringPiece& input, bool is_lower, std::string* out) {
  return do_xhex_encode(input, is_lower, out);
}

bool encode(const char_buffer_t& input, bool is_lower, std::string* out) {
  return do_xhex_encode(input, is_lower, out);
}

}  // namespace xhex

namespace unicode {
bool encode(const StringPiece16& input, bool is_lower, char_buffer_t* out) {
  return do_unicode_encode(input, is_lower, out);
}

bool decode(const StringPiece& input, string16* out) {
  return do_unicode_decode(input, out);
}
}  // namespace unicode

namespace uunicode {
bool encode(const StringPiece16& input, bool is_lower, char_buffer_t* out) {
  return do_uunicode_encode(input, is_lower, out);
}

bool decode(const StringPiece& input, string16* out) {
  return do_uunicode_decode(input, out);
}
}  // namespace uunicode
}  // namespace utils

bool HexStringToInt(const StringPiece& input, int32_t* output) {
  return IteratorRangeToNumber<HexIteratorRangeToIntTraits>::Invoke(input.begin(), input.end(), output);
}

bool HexStringToUInt(const StringPiece& input, uint32_t* output) {
  return IteratorRangeToNumber<HexIteratorRangeToUIntTraits>::Invoke(input.begin(), input.end(), output);
}

bool HexStringToInt64(const StringPiece& input, int64_t* output) {
  return IteratorRangeToNumber<HexIteratorRangeToInt64Traits>::Invoke(input.begin(), input.end(), output);
}

bool HexStringToUInt64(const StringPiece& input, uint64_t* output) {
  return IteratorRangeToNumber<HexIteratorRangeToUInt64Traits>::Invoke(input.begin(), input.end(), output);
}

bool HexStringToBytes(const std::string& input, std::vector<uint8_t>* output) {
  return HexStringToBytesT(input, output);
}

std::string ConvertVersionNumberTo3DotString(uint32_t number) {
  char buffer[16] = {0};
  uint8_t major = PROJECT_GET_MAJOR_VERSION(number);
  uint8_t minor = PROJECT_GET_MINOR_VERSION(number);
  uint8_t patch = PROJECT_GET_PATCH_VERSION(number);
  uint8_t tweak = PROJECT_GET_TWEAK_VERSION(number);
  SNPrintf(buffer, sizeof(buffer), "%u.%u.%u.%u", major, minor, patch, tweak);
  return buffer;
}

std::string ConvertVersionNumberTo2DotString(uint32_t number) {
  char buffer[16] = {0};
  uint8_t major = PROJECT_GET_MAJOR_VERSION(number);
  uint8_t minor = PROJECT_GET_MINOR_VERSION(number);
  uint8_t patch = PROJECT_GET_PATCH_VERSION(number);
  SNPrintf(buffer, sizeof(buffer), "%u.%u.%u", major, minor, patch);
  return buffer;
}

uint32_t ConvertVersionNumberFromString(const std::string& version) {
  if (version.empty()) {
    return 0;
  }

  uint8_t major = 0;
  uint8_t minor = 0;
  uint8_t patch = 0;
  uint8_t tweak = 0;

  std::vector<std::string> numbers;
  size_t count_del = Tokenize(version, ".", &numbers);
  for (size_t i = 0; i < count_del; ++i) {
    std::string number_str = numbers[i];
    if (i == 0) {
      bool res = ConvertFromString(number_str, &major);
      UNUSED(res);
    } else if (i == 1) {
      bool res = ConvertFromString(number_str, &minor);
      UNUSED(res);
    } else if (i == 2) {
      bool res = ConvertFromString(number_str, &patch);
      UNUSED(res);
    } else if (i == 3) {
      bool res = ConvertFromString(number_str, &tweak);
      UNUSED(res);
    }
  }

  return PROJECT_VERSION_GENERATE_FULL(major, minor, patch, tweak);
}

// explicit instance

template string16 ConvertToString16(const ByteArray<char>& from);
template string16 ConvertToString16(const ByteArray<unsigned char>& from);

template std::string ConvertToString(const ByteArray<char>& from);
template std::string ConvertToString(const ByteArray<unsigned char>& from);

template bool ConvertFromString(const std::string& from, ByteArray<char>* out);
template bool ConvertFromString(const std::string& from, ByteArray<unsigned char>* out);

template bool ConvertFromBytes(const ByteArray<char>& from, string16* out);
template bool ConvertFromBytes(const ByteArray<char>& from, std::string* out);
template bool ConvertFromBytes(const ByteArray<char>& from, bool* out);
template bool ConvertFromBytes(const ByteArray<char>& from, char* out);
template bool ConvertFromBytes(const ByteArray<char>& from, unsigned char* out);
template bool ConvertFromBytes(const ByteArray<char>& from, short* out);
template bool ConvertFromBytes(const ByteArray<char>& from, unsigned short* out);
template bool ConvertFromBytes(const ByteArray<char>& from, int* out);
template bool ConvertFromBytes(const ByteArray<char>& from, unsigned int* out);
template bool ConvertFromBytes(const ByteArray<char>& from, long* out);
template bool ConvertFromBytes(const ByteArray<char>& from, unsigned long* out);
template bool ConvertFromBytes(const ByteArray<char>& from, long long* out);
template bool ConvertFromBytes(const ByteArray<char>& from, unsigned long long* out);
template bool ConvertFromBytes(const ByteArray<char>& from, float* out);
template bool ConvertFromBytes(const ByteArray<char>& from, double* out);

template bool ConvertFromBytes(const ByteArray<unsigned char>& from, string16* out);
template bool ConvertFromBytes(const ByteArray<unsigned char>& from, std::string* out);
template bool ConvertFromBytes(const ByteArray<unsigned char>& from, bool* out);
template bool ConvertFromBytes(const ByteArray<unsigned char>& from, char* out);
template bool ConvertFromBytes(const ByteArray<unsigned char>& from, unsigned char* out);
template bool ConvertFromBytes(const ByteArray<unsigned char>& from, short* out);
template bool ConvertFromBytes(const ByteArray<unsigned char>& from, unsigned short* out);
template bool ConvertFromBytes(const ByteArray<unsigned char>& from, int* out);
template bool ConvertFromBytes(const ByteArray<unsigned char>& from, unsigned int* out);
template bool ConvertFromBytes(const ByteArray<unsigned char>& from, long* out);
template bool ConvertFromBytes(const ByteArray<unsigned char>& from, unsigned long* out);
template bool ConvertFromBytes(const ByteArray<unsigned char>& from, long long* out);
template bool ConvertFromBytes(const ByteArray<unsigned char>& from, unsigned long long* out);
template bool ConvertFromBytes(const ByteArray<unsigned char>& from, float* out);
template bool ConvertFromBytes(const ByteArray<unsigned char>& from, double* out);

}  // namespace common
