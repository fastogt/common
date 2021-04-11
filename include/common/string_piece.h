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

// StringPiece is a simple structure containing a pointer into some external
// storage and a size.  The user of a StringPiece must ensure that the stringpiece
// is not used after the corresponding external storage has been
// deallocated.
//
// Multiple threads can invoke const methods on a StringPiece without
// external synchronization, but if any of the threads may call a
// non-const method, all threads accessing the same StringPiece must use
// external synchronization.

#pragma once

#include <stddef.h>  // for size_t, NULL, ptrdiff_t

#include <common/string16.h>

namespace common {

template <typename STRING_TYPE>
class BasicStringPiece;
typedef BasicStringPiece<std::string> StringPiece;
typedef BasicStringPiece<string16> StringPiece16;

// internal --------------------------------------------------------------------

// Many of the StringPiece functions use different implementations for the
// 8-bit and 16-bit versions, and we don't want lots of template expansions in
// this (very common) header that will slow down compilation.
//
// So here we define overloaded functions called by the StringPiece template.
// For those that share an implementation, the two versions will expand to a
// template internal to the .cc file.
namespace internal {

void CopyToString(const StringPiece& self, std::string* target);
void CopyToString(const StringPiece16& self, string16* target);

void AppendToString(const StringPiece& self, std::string* target);
void AppendToString(const StringPiece16& self, string16* target);

size_t copy(const StringPiece& self, char* buf, size_t n, size_t pos);
size_t copy(const StringPiece16& self, char16* buf, size_t n, size_t pos);

size_t find(const StringPiece& self, const StringPiece& s, size_t pos);
size_t find(const StringPiece16& self, const StringPiece16& s, size_t pos);
size_t find(const StringPiece& self, char c, size_t pos);
size_t find(const StringPiece16& self, char16 c, size_t pos);

size_t rfind(const StringPiece& self, const StringPiece& s, size_t pos);
size_t rfind(const StringPiece16& self, const StringPiece16& s, size_t pos);
size_t rfind(const StringPiece& self, char c, size_t pos);
size_t rfind(const StringPiece16& self, char16 c, size_t pos);

size_t find_first_of(const StringPiece& self, const StringPiece& s, size_t pos);
size_t find_first_of(const StringPiece16& self, const StringPiece16& s, size_t pos);

size_t find_first_not_of(const StringPiece& self, const StringPiece& s, size_t pos);
size_t find_first_not_of(const StringPiece16& self, const StringPiece16& s, size_t pos);
size_t find_first_not_of(const StringPiece& self, char c, size_t pos);
size_t find_first_not_of(const StringPiece16& self, char16 c, size_t pos);

size_t find_last_of(const StringPiece& self, const StringPiece& s, size_t pos);
size_t find_last_of(const StringPiece16& self, const StringPiece16& s, size_t pos);
size_t find_last_of(const StringPiece& self, char c, size_t pos);
size_t find_last_of(const StringPiece16& self, char16 c, size_t pos);

size_t find_last_not_of(const StringPiece& self, const StringPiece& s, size_t pos);
size_t find_last_not_of(const StringPiece16& self, const StringPiece16& s, size_t pos);
size_t find_last_not_of(const StringPiece16& self, char16 c, size_t pos);
size_t find_last_not_of(const StringPiece& self, char c, size_t pos);

StringPiece substr(const StringPiece& self, size_t pos, size_t n);
StringPiece16 substr(const StringPiece16& self, size_t pos, size_t n);

}  // namespace internal

// BasicStringPiece ------------------------------------------------------------

// Defines the types, methods, operators, and data members common to both
// StringPiece and StringPiece16. Do not refer to this class directly, but
// rather to BasicStringPiece, StringPiece, or StringPiece16.
//
// This is templatized by string class type rather than character type, so
// BasicStringPiece<std::string> or BasicStringPiece<common::string16>.
template <typename STRING_TYPE>
class BasicStringPiece {
 public:
  // Standard STL container boilerplate.
  typedef size_t size_type;
  typedef typename STRING_TYPE::value_type value_type;
  typedef const value_type* pointer;
  typedef const value_type& reference;
  typedef const value_type& const_reference;
  typedef ptrdiff_t difference_type;
  typedef const value_type* const_iterator;
  typedef std::reverse_iterator<const_iterator> const_reverse_iterator;

  static const size_type npos;

 public:
  // We provide non-explicit singleton constructors so users can pass
  // in a "const char*" or a "string" wherever a "StringPiece" is
  // expected (likewise for char16, string16, StringPiece16).
  BasicStringPiece() : ptr_(nullptr), length_(0) {}
  BasicStringPiece(const value_type* str)
      : ptr_(str), length_((str == nullptr) ? 0 : STRING_TYPE::traits_type::length(str)) {}
  BasicStringPiece(const STRING_TYPE& str) : ptr_(str.data()), length_(str.size()) {}
  BasicStringPiece(const value_type* offset, size_type len) : ptr_(offset), length_(len) {}
  BasicStringPiece(const typename STRING_TYPE::const_iterator& begin, const typename STRING_TYPE::const_iterator& end)
      : ptr_((end > begin) ? &(*begin) : nullptr), length_((end > begin) ? static_cast<size_type>(end - begin) : 0) {}

  // data() may return a pointer to a buffer with embedded NULs, and the
  // returned buffer may or may not be null terminated.  Therefore it is
  // typically a mistake to pass data() to a routine that expects a NUL
  // terminated string.
  const value_type* data() const { return ptr_; }
  size_type size() const { return length_; }
  size_type length() const { return length_; }
  bool empty() const { return length_ == 0; }

  value_type front() const {
    CHECK_NE(0UL, length_);
    return ptr_[0];
  }

  value_type back() const {
    CHECK_NE(0UL, length_);
    return ptr_[length_ - 1];
  }

  void clear() {
    ptr_ = nullptr;
    length_ = 0;
  }
  void set(const value_type* data, size_type len) {
    ptr_ = data;
    length_ = len;
  }
  void set(const value_type* str) {
    ptr_ = str;
    length_ = str ? STRING_TYPE::traits_type::length(str) : 0;
  }

  value_type operator[](size_type i) const { return ptr_[i]; }

  void remove_prefix(size_type n) {
    ptr_ += n;
    length_ -= n;
  }

  void remove_suffix(size_type n) { length_ -= n; }

  int compare(const BasicStringPiece<STRING_TYPE>& x) const {
    int r = wordmemcmp(ptr_, x.ptr_, (length_ < x.length_ ? length_ : x.length_));
    if (r == 0) {
      if (length_ < x.length_)
        r = -1;
      else if (length_ > x.length_)
        r = +1;
    }
    return r;
  }

  STRING_TYPE as_string() const {
    // std::string doesn't like to take a NULL pointer even with a 0 size.
    return empty() ? STRING_TYPE() : STRING_TYPE(data(), size());
  }

  explicit operator STRING_TYPE() const { return as_string(); }

  const_iterator begin() const { return ptr_; }
  const_iterator end() const { return ptr_ + length_; }
  const_reverse_iterator rbegin() const { return const_reverse_iterator(ptr_ + length_); }
  const_reverse_iterator rend() const { return const_reverse_iterator(ptr_); }

  size_type max_size() const { return length_; }
  size_type capacity() const { return length_; }

  static int wordmemcmp(const value_type* p, const value_type* p2, size_type N) {
    return STRING_TYPE::traits_type::compare(p, p2, N);
  }

  // Sets the value of the given string target type to be the current string.
  // This saves a temporary over doing |a = b.as_string()|
  void CopyToString(STRING_TYPE* target) const { internal::CopyToString(*this, target); }

  void AppendToString(STRING_TYPE* target) const { internal::AppendToString(*this, target); }

  size_type copy(value_type* buf, size_type n, size_type pos = 0) const { return internal::copy(*this, buf, n, pos); }

  // Does "this" start with "x"
  bool starts_with(const BasicStringPiece& x) const {
    return ((this->length_ >= x.length_) && (wordmemcmp(this->ptr_, x.ptr_, x.length_) == 0));
  }

  // Does "this" end with "x"
  bool ends_with(const BasicStringPiece& x) const {
    return ((this->length_ >= x.length_) &&
            (wordmemcmp(this->ptr_ + (this->length_ - x.length_), x.ptr_, x.length_) == 0));
  }

  // find: Search for a character or substring at a given offset.
  size_type find(const BasicStringPiece<STRING_TYPE>& s, size_type pos = 0) const {
    return internal::find(*this, s, pos);
  }
  size_type find(value_type c, size_type pos = 0) const { return internal::find(*this, c, pos); }

  // rfind: Reverse find.
  size_type rfind(const BasicStringPiece& s, size_type pos = BasicStringPiece::npos) const {
    return internal::rfind(*this, s, pos);
  }
  size_type rfind(value_type c, size_type pos = BasicStringPiece::npos) const { return internal::rfind(*this, c, pos); }

  // find_first_of: Find the first occurence of one of a set of characters.
  size_type find_first_of(const BasicStringPiece& s, size_type pos = 0) const {
    return internal::find_first_of(*this, s, pos);
  }
  size_type find_first_of(value_type c, size_type pos = 0) const { return find(c, pos); }

  // find_first_not_of: Find the first occurence not of a set of characters.
  size_type find_first_not_of(const BasicStringPiece& s, size_type pos = 0) const {
    return internal::find_first_not_of(*this, s, pos);
  }
  size_type find_first_not_of(value_type c, size_type pos = 0) const {
    return internal::find_first_not_of(*this, c, pos);
  }

  // find_last_of: Find the last occurence of one of a set of characters.
  size_type find_last_of(const BasicStringPiece& s, size_type pos = BasicStringPiece::npos) const {
    return internal::find_last_of(*this, s, pos);
  }
  size_type find_last_of(value_type c, size_type pos = BasicStringPiece::npos) const { return rfind(c, pos); }

  // find_last_not_of: Find the last occurence not of a set of characters.
  size_type find_last_not_of(const BasicStringPiece& s, size_type pos = BasicStringPiece::npos) const {
    return internal::find_last_not_of(*this, s, pos);
  }
  size_type find_last_not_of(value_type c, size_type pos = BasicStringPiece::npos) const {
    return internal::find_last_not_of(*this, c, pos);
  }

  // substr.
  BasicStringPiece substr(size_type pos, size_type n = BasicStringPiece::npos) const {
    return internal::substr(*this, pos, n);
  }

 protected:
  const value_type* ptr_;
  size_type length_;
};

template <typename STRING_TYPE>
const typename BasicStringPiece<STRING_TYPE>::size_type BasicStringPiece<STRING_TYPE>::npos =
    typename BasicStringPiece<STRING_TYPE>::size_type(-1);

// MSVC doesn't like complex extern templates and DLLs.
#if !defined(COMPILER_MSVC)
extern template class BasicStringPiece<std::string>;
extern template class BasicStringPiece<string16>;
#endif

// StingPiece operators --------------------------------------------------------

bool operator==(const StringPiece& x, const StringPiece& y);

inline bool operator!=(const StringPiece& x, const StringPiece& y) {
  return !(x == y);
}

inline bool operator<(const StringPiece& x, const StringPiece& y) {
  const int r = StringPiece::wordmemcmp(x.data(), y.data(), (x.size() < y.size() ? x.size() : y.size()));
  return ((r < 0) || ((r == 0) && (x.size() < y.size())));
}

inline bool operator>(const StringPiece& x, const StringPiece& y) {
  return y < x;
}

inline bool operator<=(const StringPiece& x, const StringPiece& y) {
  return !(x > y);
}

inline bool operator>=(const StringPiece& x, const StringPiece& y) {
  return !(x < y);
}

// StringPiece16 operators -----------------------------------------------------

inline bool operator==(const StringPiece16& x, const StringPiece16& y) {
  if (x.size() != y.size()) {
    return false;
  }

  return StringPiece16::wordmemcmp(x.data(), y.data(), x.size()) == 0;
}

inline bool operator!=(const StringPiece16& x, const StringPiece16& y) {
  return !(x == y);
}

inline bool operator<(const StringPiece16& x, const StringPiece16& y) {
  const int r = StringPiece16::wordmemcmp(x.data(), y.data(), (x.size() < y.size() ? x.size() : y.size()));
  return ((r < 0) || ((r == 0) && (x.size() < y.size())));
}

inline bool operator>(const StringPiece16& x, const StringPiece16& y) {
  return y < x;
}

inline bool operator<=(const StringPiece16& x, const StringPiece16& y) {
  return !(x > y);
}

inline bool operator>=(const StringPiece16& x, const StringPiece16& y) {
  return !(x < y);
}

std::ostream& operator<<(std::ostream& o, const StringPiece& piece);

}  // namespace common
