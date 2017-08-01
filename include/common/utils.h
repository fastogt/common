/*  Copyright (C) 2014-2017 FastoGT. All right reserved.

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

#include <stdint.h>  // for uint64_t
#include <string.h>
#include <sys/types.h>  // for useconds_t
#include <functional>   // for binary_function, unary_function
#include <string>       // for string
#include <utility>      // for pair

#include <common/string16.h>  // for char16
#include <common/types.h>     // for buffer_t, byte_t

#define HAS_MEMBER(CLASS_NAME, FUNC_MUST_BE)                                                  \
  COMPILE_ASSERT(std::is_member_function_pointer<decltype(&CLASS_NAME::FUNC_MUST_BE)>::value, \
                 "Class does not contain member " #FUNC_MUST_BE "!")

namespace common {
namespace utils {
namespace hash {
uint64_t crc64(uint64_t crc, const byte_t* data, uint64_t lenght);
uint64_t crc64(uint64_t crc, const buffer_t& data);
}  // namespace hash

namespace traits {

template <typename T>
struct size_trait_info {
  enum { value = sizeof(T) };
};

template <size_t N>
struct size_trait_info<const char16[N]> {
  enum { value = N };
};

}  // namespace traits

namespace enums {

template <typename type, size_t size>
inline type FindTypeInArray(const char16* (&arr)[size], const char16* text) {
  for (size_t i = 0; i < size; ++i) {
    const size_t len = c16len(text);
    if (c16memcmp(text, arr[i], len) == 0) {
      return static_cast<type>(i);
    }
  }

  return static_cast<type>(0);
}

template <size_t size>
inline std::vector<string16> ConvertToVector(const char16* (&arr)[size]) {
  std::vector<string16> res;
  for (size_t i = 0; i < size; ++i) {
    res.push_back(arr[i]);
  }

  return res;
}

template <typename type, size_t size>
inline type FindTypeInArray(const char* (&arr)[size], const char* text) {
  for (size_t i = 0; i < size; ++i) {
    const size_t len = strlen(text);
    if (memcmp(text, arr[i], len) == 0) {
      return static_cast<type>(i);
    }
  }

  return static_cast<type>(0);
}

template <size_t size>
inline std::vector<std::string> ConvertToVector(const char* (&arr)[size]) {
  std::vector<std::string> res;
  for (size_t i = 0; i < size; ++i) {
    res.push_back(arr[i]);
  }

  return res;
}

}  // namespace enums

namespace delete_wrappers {

template <typename T>
struct default_delete : public std::unary_function<T, void> {
  inline void operator()(T* ptr) const { destroy(&ptr); }
};

template <typename T>
struct default_delete<T*> : public std::unary_function<T*, void> {
  inline void operator()(T* ptr) const { destroy(&ptr); }
};

template <typename T, size_t N>
struct default_delete<T[N]> : public std::unary_function<const T[N], void> {
  inline void operator()(const T ptr) const { delete[] ptr; }
};

}  // namespace delete_wrappers

namespace compare {
template <typename T>
struct RuntimeCmp : public std::binary_function<const T&, const T&, bool> {
  inline bool operator()(const T& t1, const T& t2) const { return t1 < t2; }
};

template <typename T>
struct RuntimeCmp<T*> : public std::binary_function<const T*, const T*, bool> {
  inline bool operator()(const T* t1, const T* t2) const { return (*t1) < (*t2); }
};

template <typename T, typename U>
struct RuntimeCmp<std::pair<T, U> > : public std::binary_function<const std::pair<T, U>, const std::pair<T, U>, bool> {
  inline bool operator()(const std::pair<T, U>& t1, const std::pair<T, U>& t2) const {
    if (t1.first == t2.first) {
      return t1.second < t2.second;
    }

    return t1.first < t2.first;
  }
};

}  // namespace compare

namespace base64 {
std::string encode64(const std::string& input);
std::string decode64(const std::string& input);
buffer_t encode64(const buffer_t& input);
buffer_t decode64(const buffer_t& input);
}  // namespace base64

namespace html {
std::string encode(const std::string& input);
std::string decode(const std::string& input);
}  // namespace html

void msleep(useconds_t msec);
void usleep(useconds_t usec);

char* strdupornull(const std::string& src);
char* strdupornull(const char* src);
void freeifnotnull(void* ptr);
const char* c_strornull(const std::string& val);
}  // namespace utils

#ifdef OS_POSIX
void create_as_daemon();
#endif
long get_current_process_pid();
}  // namespace common
