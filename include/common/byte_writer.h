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

#include <string.h>

#include <string>
#include <vector>

namespace common {

template <typename CharT, size_t sz, typename = std::true_type>
class ByteWriter;

template <typename>
struct is_byte : std::false_type {};
template <>
struct is_byte<char> : std::true_type {};
template <>
struct is_byte<unsigned char> : std::true_type {};

template <typename CharT, size_t sz>
class ByteWriter<CharT, sz, typename is_byte<CharT>::type> {
 public:
  typedef std::vector<CharT> vector_t;

  ByteWriter() : buffer_() { buffer_.reserve(sz); }

  inline vector_t str() const { return buffer_; }

  inline bool empty() const { return buffer_.empty(); }

  inline void clear() { buffer_.clear(); }

  inline void append_object(char obj) { buffer_.push_back(obj); }

  inline void append_object(unsigned char obj) { buffer_.push_back(obj); }

  template <typename ch>
  inline void append_object(const ch* obj) {
    for (size_t i = 0; i < strlen(obj); ++i) {
      append_object(obj[i]);
    }
  }

  template <typename ch>
  inline void append_object(const std::basic_string<ch>& obj) {
    for (size_t i = 0; i < obj.size(); ++i) {
      append_object(obj[i]);
    }
  }

  template <typename ch>
  inline void append_object(const std::vector<ch>& obj) {
    for (size_t i = 0; i < obj.size(); ++i) {
      append_object(obj[i]);
    }
  }

 private:
  vector_t buffer_;
};

template <size_t sz>
using char_writer = ByteWriter<char, sz>;

template <size_t sz>
using unsigned_char_writer = ByteWriter<unsigned char, sz>;

template <typename CharT, size_t sz, typename T>
inline ByteWriter<CharT, sz>& operator<<(ByteWriter<CharT, sz>& os, T obj) {
  os.append_object(std::forward<T>(obj));
  return os;
}

}  // namespace common
