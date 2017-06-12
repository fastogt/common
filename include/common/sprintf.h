/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

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

#include <stddef.h>  // for size_t
#include <stdlib.h>  // for free, NULL

#include <string>  // for string

#include <common/macros.h>       // for DCHECK, DCHECK_NE
#include <common/string_util.h>  // for snprintf
#include <common/types.h>        // for buffer_t

namespace common {

template <typename T>
inline T normalize(T t) {
  return t;
}

inline const char* normalize(const std::string& text) {
  return text.c_str();
}

inline const char* normalize(const buffer_t& buffer) {
  return (const char*)buffer.data();
}

template <typename... Args>
inline int SNPrintf(char* buff, size_t buff_size, const char* fmt, Args... args) {
  int res = snprintf(buff, buff_size, fmt, normalize(args)...);
  DCHECK(res != -1 && res < (int)buff_size);
  return res;
}

template <typename... Args>
std::string MemSPrintf(const char* fmt, Args... args) {
  char* ret = NULL;
  int res = vasprintf(&ret, fmt, normalize(args)...);
  DCHECK_NE(res, -1);
  std::string str(ret);
  free(ret);
  return str;
}

}  // namespace common
