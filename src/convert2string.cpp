/*  Copyright (C) 2014-2018 FastoGT. All right reserved.

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

#include <algorithm>
#include <limits>

#include <common/sprintf.h>
#include <common/utf_string_conversions.h>  // for UTF16ToUTF8

namespace common {

namespace {

template <typename STR, typename INT, typename UINT, bool NEG>
struct IntToStringT {
  // This is to avoid a compiler warning about unary minus on unsigned type.
  // For example, say you had the following code:
  //   template <typename INT>
  //   INT abs(INT value) { return value < 0 ? -value : value; }
  // Even though if INT is unsigned, it's impossible for value < 0, so the
  // unary minus will never be taken, the compiler will still generate a
  // warning.  We do a little specialization dance...
  template <typename INT2, typename UINT2, bool NEG2>
  struct ToUnsignedT {};

  template <typename INT2, typename UINT2>
  struct ToUnsignedT<INT2, UINT2, false> {
    static UINT2 ToUnsigned(INT2 value) { return static_cast<UINT2>(value); }
  };

  template <typename INT2, typename UINT2>
  struct ToUnsignedT<INT2, UINT2, true> {
    static UINT2 ToUnsigned(INT2 value) { return static_cast<UINT2>(value < 0 ? -value : value); }
  };

  // This set of templates is very similar to the above templates, but
  // for testing whether an integer is negative.
  template <typename INT2, bool NEG2>
  struct TestNegT {};
  template <typename INT2>
  struct TestNegT<INT2, false> {
    static bool TestNeg(INT2 value) {
      // value is unsigned, and can never be negative.
      UNUSED(value);

      return false;
    }
  };
  template <typename INT2>
  struct TestNegT<INT2, true> {
    static bool TestNeg(INT2 value) { return value < 0; }
  };

  static STR IntToString(INT value) {
    // log10(2) ~= 0.3 bytes needed per bit or per byte log10(2**8) ~= 2.4.
    // So round up to allocate 3 output characters per byte, plus 1 for '-'.
    const int kOutputBufSize = 3 * sizeof(INT) + 1;

    // Allocate the whole string right away, we will right back to front, and
    // then return the substr of what we ended up using.
    STR outbuf(kOutputBufSize, 0);

    bool is_neg = TestNegT<INT, NEG>::TestNeg(value);
    // Even though is_neg will never be true when INT is parameterized as
    // unsigned, even the presence of the unary operation causes a warning.
    UINT res = ToUnsignedT<INT, UINT, NEG>::ToUnsigned(value);

    typename STR::iterator it(outbuf.end());
    do {
      --it;
      DCHECK(it != outbuf.begin());
      *it = static_cast<typename STR::value_type>((res % 10) + '0');
      res /= 10;
    } while (res != 0);
    if (is_neg) {
      --it;
      DCHECK(it != outbuf.begin());
      *it = static_cast<typename STR::value_type>('-');
    }
    return STR(it, outbuf.end());
  }
};

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
class BaseHexIteratorRangeToIntTraits : public BaseIteratorRangeToNumberTraits<ITERATOR, int, 16> {};

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
bool StringToIntImpl(const StringPiece& input, VALUE* output) {
  return IteratorRangeToNumber<StringPieceToNumberTraits<VALUE, 10>>::Invoke(input.begin(), input.end(), output);
}

template <typename VALUE, int BASE>
class StringPiece16ToNumberTraits : public BaseIteratorRangeToNumberTraits<StringPiece16::const_iterator, VALUE, BASE> {
};

template <typename VALUE>
bool String16ToIntImpl(const StringPiece16& input, VALUE* output) {
  return IteratorRangeToNumber<StringPiece16ToNumberTraits<VALUE, 10>>::Invoke(input.begin(), input.end(), output);
}

template <typename VALUE, int BASE>
class BytesToNumberTraits : public BaseIteratorRangeToNumberTraits<buffer_t::const_iterator, VALUE, BASE> {};

template <typename VALUE>
bool BytesToIntImpl(const buffer_t& input, VALUE* output) {
  return IteratorRangeToNumber<BytesToNumberTraits<VALUE, 10>>::Invoke(input.begin(), input.end(), output);
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

template <typename R, typename T>
R hex_encode_impl(const T& input, bool is_lower) {
  static const char uHexChars[] = "0123456789ABCDEF";
  static const char lHexChars[] = "0123456789abcdef";

  typedef typename T::value_type value_type;
  const typename T::size_type size = input.size();
  R decoded;
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
  return decoded;
}

template <typename R, typename T>
R hex_decode_impl(const T& input) {
  std::vector<uint8_t> vec;
  bool res = HexStringToBytesT(input, &vec);
  if (!res) {
    return R();
  }

  return R(vec.begin(), vec.end());
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

string16 ConvertToString16(const buffer_t& from) {
  std::string str = ConvertToString(from);
  return ConvertToString16(str);
}

string16 ConvertToString16(const string16& from) {
  return from;
}

string16 ConvertToString16(const StringPiece16& from) {
  return from.as_string();
}

string16 ConvertToString16(bool value) {
  return value ? ASCIIToUTF16("true") : ASCIIToUTF16("false");
}

string16 ConvertToString16(char value) {
  return IntToStringT<string16, char, unsigned char, true>::IntToString(value);
}

string16 ConvertToString16(unsigned char value) {
  return IntToStringT<string16, char, unsigned char, false>::IntToString(value);
}

string16 ConvertToString16(short value) {
  return IntToStringT<string16, short, unsigned short, true>::IntToString(value);
}

string16 ConvertToString16(unsigned short value) {
  return IntToStringT<string16, short, unsigned short, false>::IntToString(value);
}

string16 ConvertToString16(int value) {
  return IntToStringT<string16, int, unsigned int, true>::IntToString(value);
}

string16 ConvertToString16(unsigned int value) {
  return IntToStringT<string16, int, unsigned int, false>::IntToString(value);
}

string16 ConvertToString16(long value) {
  return IntToStringT<string16, long, unsigned long, true>::IntToString(value);
}

string16 ConvertToString16(unsigned long value) {
  return IntToStringT<string16, long, unsigned long, false>::IntToString(value);
}

string16 ConvertToString16(long long value) {
  return IntToStringT<string16, long long, unsigned long long, true>::IntToString(value);
}

string16 ConvertToString16(unsigned long long value) {
  return IntToStringT<string16, long long, unsigned long long, false>::IntToString(value);
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

bool ConvertFromString16(const string16& from, buffer_t* out) {
  if (!out) {
    return false;
  }

  std::string ascii;
  bool res = ConvertFromString16(from, &ascii);
  if (!res) {
    return false;
  }

  *out = buffer_t(ascii.begin(), ascii.end());
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
  bool res = String16ToIntImpl(from, &loutput);
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
  bool res = String16ToIntImpl(from, &loutput);
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
  bool res = String16ToIntImpl(from, &loutput);
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
  bool res = String16ToIntImpl(from, &loutput);
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
  bool res = String16ToIntImpl(from, &loutput);
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
  bool res = String16ToIntImpl(from, &loutput);
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
  bool res = String16ToIntImpl(from, &loutput);
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
  bool res = String16ToIntImpl(from, &loutput);
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
  bool res = String16ToIntImpl(from, &loutput);
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
  bool res = String16ToIntImpl(from, &loutput);
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

bool ConvertFromString16(const string16& from, string16* out) {
  if (!out) {
    return false;
  }

  *out = from;
  return true;
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

std::string ConvertToString(const buffer_t& from) {
  return std::string(reinterpret_cast<const char*>(from.data()), from.size());
}

std::string ConvertToString(const string16& from) {
  std::string ascii;
  if (!ConvertFromString16(from, &ascii)) {
    return std::string();
  }

  return ascii;
}

std::string ConvertToString(const std::string& from) {
  return from;
}

std::string ConvertToString(const StringPiece& from) {
  return from.as_string();
}

std::string ConvertToString(bool value) {
  return value ? "true" : "false";
}

std::string ConvertToString(char value) {
  return IntToStringT<std::string, char, unsigned char, true>::IntToString(value);
}

std::string ConvertToString(unsigned char value) {
  return IntToStringT<std::string, char, unsigned char, false>::IntToString(value);
}

std::string ConvertToString(short value) {
  return IntToStringT<std::string, short, unsigned short, true>::IntToString(value);
}

std::string ConvertToString(unsigned short value) {
  return IntToStringT<std::string, short, unsigned short, false>::IntToString(value);
}

std::string ConvertToString(int value) {
  return IntToStringT<std::string, int, unsigned int, true>::IntToString(value);
}

std::string ConvertToString(unsigned int value) {
  return IntToStringT<std::string, int, unsigned int, false>::IntToString(value);
}

std::string ConvertToString(long value) {
  return IntToStringT<std::string, long, unsigned long, true>::IntToString(value);
}

std::string ConvertToString(unsigned long value) {
  return IntToStringT<std::string, long, unsigned long, false>::IntToString(value);
}

std::string ConvertToString(long long value) {
  return IntToStringT<std::string, long long, unsigned long long, true>::IntToString(value);
}

std::string ConvertToString(unsigned long long value) {
  return IntToStringT<std::string, long long, unsigned long long, false>::IntToString(value);
}

std::string ConvertToString(float value, int prec) {
  char buffer[16];
  if (prec == 0) {
    SNPrintf(buffer, sizeof(buffer), "%.0f", value);
  } else if (prec == 1) {
    SNPrintf(buffer, sizeof(buffer), "%.1f", value);
  } else if (prec == 2) {
    SNPrintf(buffer, sizeof(buffer), "%.2f", value);
  } else if (prec == 3) {
    SNPrintf(buffer, sizeof(buffer), "%.3f", value);
  } else if (prec == 4) {
    SNPrintf(buffer, sizeof(buffer), "%.4f", value);
  } else {
    SNPrintf(buffer, sizeof(buffer), "%f", value);
  }

  return buffer;
}

std::string ConvertToString(double value, int prec) {
  char buffer[32];
  if (prec == 0) {
    SNPrintf(buffer, sizeof(buffer), "%.0lf", value);
  } else if (prec == 1) {
    SNPrintf(buffer, sizeof(buffer), "%.1lf", value);
  } else if (prec == 2) {
    SNPrintf(buffer, sizeof(buffer), "%.2lf", value);
  } else if (prec == 3) {
    SNPrintf(buffer, sizeof(buffer), "%.3lf", value);
  } else if (prec == 4) {
    SNPrintf(buffer, sizeof(buffer), "%.4lf", value);
  } else {
    SNPrintf(buffer, sizeof(buffer), "%lf", value);
  }

  return buffer;
}

bool ConvertFromString(const std::string& from, buffer_t* out) {
  if (!out) {
    return false;
  }

  const char* cstr = from.c_str();
  *out = buffer_t(cstr, cstr + from.size());
  return true;
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
  bool res = StringToIntImpl(from, &loutput);
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
  bool res = StringToIntImpl(from, &loutput);
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
  bool res = StringToIntImpl(from, &loutput);
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
  bool res = StringToIntImpl(from, &loutput);
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
  bool res = StringToIntImpl(from, &loutput);
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
  bool res = StringToIntImpl(from, &loutput);
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
  bool res = StringToIntImpl(from, &loutput);
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
  bool res = StringToIntImpl(from, &loutput);
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
  bool res = StringToIntImpl(from, &loutput);
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
  bool res = StringToIntImpl(from, &loutput);
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

  *out = atof(from.c_str());
  return true;
}

bool ConvertFromString(const std::string& from, double* out) {
  if (!out) {
    return false;
  }

  *out = atof(from.c_str());
  return true;
}

bool ConvertFromString(const std::string& from, std::string* out) {
  if (!out) {
    return false;
  }

  *out = from;
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

buffer_t ConvertToBytes(const std::string& from) {
  buffer_t buff;
  if (!ConvertFromString(from, &buff)) {
    return buffer_t();
  }

  return buff;
}

buffer_t ConvertToBytes(const string16& from) {
  buffer_t buff;
  if (!ConvertFromString16(from, &buff)) {
    return buffer_t();
  }

  return buff;
}

buffer_t ConvertToBytes(const buffer_t& from) {
  return from;
}

buffer_t ConvertToBytes(bool value) {
  return value ? MAKE_BUFFER("true") : MAKE_BUFFER("false");
}

buffer_t ConvertToBytes(char value) {
  return IntToStringT<buffer_t, char, unsigned char, true>::IntToString(value);
}

buffer_t ConvertToBytes(unsigned char value) {
  return IntToStringT<buffer_t, char, unsigned char, false>::IntToString(value);
}

buffer_t ConvertToBytes(short value) {
  return IntToStringT<buffer_t, short, unsigned short, true>::IntToString(value);
}

buffer_t ConvertToBytes(unsigned short value) {
  return IntToStringT<buffer_t, short, unsigned short, false>::IntToString(value);
}

buffer_t ConvertToBytes(int value) {
  return IntToStringT<buffer_t, int, unsigned int, true>::IntToString(value);
}

buffer_t ConvertToBytes(unsigned int value) {
  return IntToStringT<buffer_t, int, unsigned int, false>::IntToString(value);
}

buffer_t ConvertToBytes(long value) {
  return IntToStringT<buffer_t, long, unsigned long, true>::IntToString(value);
}

buffer_t ConvertToBytes(unsigned long value) {
  return IntToStringT<buffer_t, long, unsigned long, false>::IntToString(value);
}

buffer_t ConvertToBytes(long long value) {
  return IntToStringT<buffer_t, long long, unsigned long long, true>::IntToString(value);
}

buffer_t ConvertToBytes(unsigned long long value) {
  return IntToStringT<buffer_t, long long, unsigned long long, false>::IntToString(value);
}

buffer_t ConvertToBytes(float value, int prec) {
  char buffer[16];
  int res = 0;
  if (prec == 0) {
    res = SNPrintf(buffer, sizeof(buffer), "%.0f", value);
  } else if (prec == 1) {
    res = SNPrintf(buffer, sizeof(buffer), "%.1f", value);
  } else if (prec == 2) {
    res = SNPrintf(buffer, sizeof(buffer), "%.2f", value);
  } else if (prec == 3) {
    res = SNPrintf(buffer, sizeof(buffer), "%.3f", value);
  } else if (prec == 4) {
    res = SNPrintf(buffer, sizeof(buffer), "%.4f", value);
  } else {
    res = SNPrintf(buffer, sizeof(buffer), "%f", value);
  }

  return buffer_t(buffer, buffer + res);
}

buffer_t ConvertToBytes(double value, int prec) {
  char buffer[32];
  int res = 0;
  if (prec == 0) {
    res = SNPrintf(buffer, sizeof(buffer), "%.0lf", value);
  } else if (prec == 1) {
    res = SNPrintf(buffer, sizeof(buffer), "%.1lf", value);
  } else if (prec == 2) {
    res = SNPrintf(buffer, sizeof(buffer), "%.2lf", value);
  } else if (prec == 3) {
    res = SNPrintf(buffer, sizeof(buffer), "%.3lf", value);
  } else if (prec == 4) {
    res = SNPrintf(buffer, sizeof(buffer), "%.4lf", value);
  } else {
    res = SNPrintf(buffer, sizeof(buffer), "%lf", value);
  }

  return buffer_t(buffer, buffer + res);
}

bool ConvertFromBytes(const buffer_t& from, string16* out) {
  if (!out) {
    return false;
  }

  const unsigned char* cstr = from.data();
  *out = string16(cstr, cstr + from.size());
  return true;
}

bool ConvertFromBytes(const buffer_t& from, std::string* out) {
  if (!out) {
    return false;
  }

  const unsigned char* cstr = from.data();
  *out = std::string(cstr, cstr + from.size());
  return true;
}

bool ConvertFromBytes(const buffer_t& from, bool* out) {
  if (!out) {
    return false;
  }

  buffer_t copy = from;
  std::transform(copy.begin(), copy.end(), copy.begin(), ::tolower);
  *out = copy == MAKE_BUFFER("true");
  return true;
}

bool ConvertFromBytes(const buffer_t& from, char* out) {
  if (!out) {
    return false;
  }

  char loutput = 0;
  bool res = BytesToIntImpl(from, &loutput);
  if (!res) {
    return false;
  }

  *out = loutput;
  return true;
}

bool ConvertFromBytes(const buffer_t& from, unsigned char* out) {
  if (!out) {
    return false;
  }

  unsigned char loutput = 0;
  bool res = BytesToIntImpl(from, &loutput);
  if (!res) {
    return false;
  }

  *out = loutput;
  return true;
}

bool ConvertFromBytes(const buffer_t& from, short* out) {
  if (!out) {
    return false;
  }

  short loutput = 0;
  bool res = BytesToIntImpl(from, &loutput);
  if (!res) {
    return false;
  }

  *out = loutput;
  return true;
}

bool ConvertFromBytes(const buffer_t& from, unsigned short* out) {
  if (!out) {
    return false;
  }

  unsigned short loutput = 0;
  bool res = BytesToIntImpl(from, &loutput);
  if (!res) {
    return false;
  }

  *out = loutput;
  return true;
}

bool ConvertFromBytes(const buffer_t& from, int* out) {
  if (!out) {
    return false;
  }

  int loutput = 0;
  bool res = BytesToIntImpl(from, &loutput);
  if (!res) {
    return false;
  }

  *out = loutput;
  return true;
}

bool ConvertFromBytes(const buffer_t& from, unsigned int* out) {
  if (!out) {
    return false;
  }

  unsigned int loutput = 0;
  bool res = BytesToIntImpl(from, &loutput);
  if (!res) {
    return false;
  }

  *out = loutput;
  return true;
}

bool ConvertFromBytes(const buffer_t& from, long* out) {
  if (!out) {
    return false;
  }

  long loutput = 0;
  bool res = BytesToIntImpl(from, &loutput);
  if (!res) {
    return false;
  }

  *out = loutput;
  return true;
}

bool ConvertFromBytes(const buffer_t& from, unsigned long* out) {
  if (!out) {
    return false;
  }

  unsigned long loutput = 0;
  bool res = BytesToIntImpl(from, &loutput);
  if (!res) {
    return false;
  }

  *out = loutput;
  return true;
}

bool ConvertFromBytes(const buffer_t& from, long long* out) {
  if (!out) {
    return false;
  }

  long long loutput = 0;
  bool res = BytesToIntImpl(from, &loutput);
  if (!res) {
    return false;
  }

  *out = loutput;
  return true;
}

bool ConvertFromBytes(const buffer_t& from, unsigned long long* out) {
  if (!out) {
    return false;
  }

  unsigned long long loutput = 0;
  bool res = BytesToIntImpl(from, &loutput);
  if (!res) {
    return false;
  }

  *out = loutput;
  return true;
}

bool ConvertFromBytes(const buffer_t& from, float* out) {
  if (!out) {
    return false;
  }

  *out = atof(reinterpret_cast<const char*>(from.data()));
  return true;
}

bool ConvertFromBytes(const buffer_t& from, double* out) {
  if (!out) {
    return false;
  }

  *out = atof(reinterpret_cast<const char*>(from.data()));
  return true;
}

bool ConvertFromBytes(const buffer_t& from, buffer_t* out) {
  if (!out) {
    return false;
  }

  *out = from;
  return true;
}

//

namespace utils {
namespace hex {

buffer_t encode(const buffer_t& input, bool is_lower) {
  return hex_encode_impl<buffer_t>(input, is_lower);
}

std::string encode(const StringPiece& input, bool is_lower) {
  return hex_encode_impl<std::string>(input, is_lower);
}

buffer_t decode(const buffer_t& input) {
  return hex_decode_impl<buffer_t>(input);
}

std::string decode(const StringPiece& input) {
  return hex_decode_impl<std::string>(input);
}

}  // namespace hex
}  // namespace utils

bool HexStringToInt(const StringPiece& input, int* output) {
  return IteratorRangeToNumber<HexIteratorRangeToIntTraits>::Invoke(input.begin(), input.end(), output);
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

}  // namespace common
