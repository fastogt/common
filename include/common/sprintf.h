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

#include <common/string_util.h>  // for snprintf

namespace common {

template <typename T>
inline T normalize(T t);

template <>
inline char normalize(char t) {
  return t;
}

template <>
inline int8_t normalize(int8_t t) {
  return t;
}

template <>
inline uint8_t normalize(uint8_t t) {
  return t;
}

template <>
inline int16_t normalize(int16_t t) {
  return t;
}

template <>
inline uint16_t normalize(uint16_t t) {
  return t;
}

template <>
inline int32_t normalize(int32_t t) {
  return t;
}

template <>
inline uint32_t normalize(uint32_t t) {
  return t;
}

template <>
inline int64_t normalize(int64_t t) {
  return t;
}

template <>
inline uint64_t normalize(uint64_t t) {
  return t;
}

template <>
inline float normalize(float t) {
  return t;
}

template <>
inline double normalize(double t) {
  return t;
}

template <>
inline long double normalize(long double t) {
  return t;
}

template <>
inline void* normalize(void* t) {
  return t;
}

template <>
inline const char* normalize(const char* t) {
  return t;
}

template <>
inline char* normalize(char* t) {
  return t;
}

inline const char* normalize(const std::string& text) {
  return text.c_str();
}

template <typename... Args>
inline int SNPrintf(char* buff, size_t buff_size, const char* fmt, Args... args) {
  int res = snprintf(buff, buff_size, fmt, normalize(args)...);
  if (res < 0 || buff_size < static_cast<size_t>(res)) {
    DNOTREACHED();
    return res;
  }
  return res;
}

template <typename... Args>
std::string MemSPrintf(const char* fmt, Args... args) {  // args should be not null terminated strings or simple types
  char* ret = NULL;
  int res = vasprintf(&ret, fmt, normalize(args)...);
  if (res == -1) {
    DNOTREACHED();
    return std::string();
  }
  std::string str(ret, res);
  free(ret);
  return str;
}

}  // namespace common
