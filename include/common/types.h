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

#pragma once

#include <stdint.h>
#include <time.h>

#include <iosfwd>
#include <string>
#include <vector>  // for vector

namespace common {

enum tribool { FAIL = 0, SUCCESS = 1, INDETERMINATE = -1 };

typedef uint8_t byte_t;

inline size_t hash_combine(size_t seed, size_t value) {
  return seed ^ (value + 0x9e3779b9 + (seed << 6u) + (seed >> 2u));
}

template <typename T>
class ByteArray : public std::vector<T> {
 public:
  typedef std::vector<T> base_class;
  typedef ByteArray<T> self_type;
  typedef typename base_class::value_type value_type;
  typedef typename base_class::iterator iterator;
  typedef typename base_class::const_iterator const_iterator;

  ByteArray() : base_class() {}
  template <typename it>
  ByteArray(it begin, it end) : base_class(begin, end) {}
  ByteArray(std::initializer_list<T> t) : base_class(t) {}

  ByteArray(const base_class& rhs) : base_class(rhs) {}

  ByteArray(const self_type&) = default;
  ByteArray& operator=(const self_type&) = default;

  inline void append(unsigned char t) { base_class::push_back(t); }

  inline void append(char t) { base_class::push_back(t); }

  inline void append(const unsigned char* obj, size_t size) {
    for (size_t i = 0; i < size; ++i) {
      append(obj[i]);
    }
  }

  inline void append(const char* obj, size_t size) {
    for (size_t i = 0; i < size; ++i) {
      append(obj[i]);
    }
  }

  template <typename ch>
  inline void append(const std::vector<ch>& obj) {
    for (size_t i = 0; i < obj.size(); ++i) {
      append(obj[i]);
    }
  }

  template <typename ch>
  inline void append(const std::basic_string<ch>& obj) {
    for (size_t i = 0; i < obj.size(); ++i) {
      append(obj[i]);
    }
  }

  template <typename ch>
  ByteArray& operator+=(ch rhs) {
    append(rhs);
    return *this;
  }

  template <typename ch>
  ByteArray& operator+=(const std::basic_string<ch>& rhs) {
    append(rhs);
    return *this;
  }

  template <typename ch>
  ByteArray& operator+=(const std::vector<ch>& rhs) {
    append(rhs);
    return *this;
  }

  std::string as_string() const {
    // std::string doesn't like to take a NULL pointer even with a 0 size.
    return base_class::empty() ? std::string()
                               : std::string(reinterpret_cast<const char*>(base_class::data()), base_class::size());
  }
};

typedef ByteArray<byte_t> byte_array_t;
typedef byte_array_t buffer_t;

typedef ByteArray<char> char_byte_array_t;
typedef char_byte_array_t char_buffer_t;

typedef int64_t time64_t;  // millisecond
typedef time_t utctime_t;  // seconds

class IMetaClassInfo {
 public:
  virtual const char* ClassName() const = 0;
  virtual ~IMetaClassInfo();
};

template <typename T>
class ClonableBase {
 public:
  virtual T* Clone() const = 0;
  virtual ~ClonableBase() {}
};

}  // namespace common

namespace std {

template <typename T>
struct hash<common::ByteArray<T>> {
  size_t operator()(const common::ByteArray<T>& k) const {
    size_t seed = 0;
    for (const auto& elem : k) {
      seed = common::hash_combine(seed, std::hash<T>()(elem));
    }
    return seed;
  }
};

}  // namespace std

std::ostream& operator<<(std::ostream& out, const common::buffer_t& buff);
std::ostream& operator<<(std::ostream& out, const common::char_buffer_t& buff);

#define MAKE_BUFFER_SIZE(STR, SIZE) common::buffer_t(STR, STR + SIZE)
#define MAKE_BUFFER(STR) MAKE_BUFFER_SIZE(STR, sizeof(STR) - 1)

#define MAKE_CHAR_BUFFER_SIZE(STR, SIZE) common::char_buffer_t(STR, STR + SIZE)
#define MAKE_CHAR_BUFFER(STR) MAKE_CHAR_BUFFER_SIZE(STR, sizeof(STR) - 1)

#define MAKE_STRING_SIZE(STR, SIZE) std::string(STR, STR + SIZE)
#define MAKE_STRING(STR) MAKE_STRING_SIZE(STR, sizeof(STR) - 1)
