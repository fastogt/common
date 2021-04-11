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

#include <common/string_util.h>  // for snprintf

namespace common {

bool normalize_for_printf(bool t);
char normalize_for_printf(char t);
unsigned char normalize_for_printf(unsigned char t);
short normalize_for_printf(short t);
unsigned short normalize_for_printf(unsigned short t);
int normalize_for_printf(int t);
unsigned int normalize_for_printf(unsigned int t);
long normalize_for_printf(long t);
unsigned long normalize_for_printf(unsigned long t);
long long normalize_for_printf(long long t);
unsigned long long normalize_for_printf(unsigned long long t);
float normalize_for_printf(float t);
double normalize_for_printf(double t);
long double normalize_for_printf(long double t);
void* normalize_for_printf(void* t);
const void* normalize_for_printf(const void* t);
const char* normalize_for_printf(char* t);
const char* normalize_for_printf(const char* t);
const char* normalize_for_printf(const std::string& text);

template <typename... Args>
inline int SNPrintf(char* buff, size_t buff_size, const char* fmt, Args... args) {
  int res = snprintf(buff, buff_size, fmt, normalize_for_printf(args)...);
  if (res < 0 || buff_size < static_cast<size_t>(res)) {
    DNOTREACHED();
    return res;
  }
  return res;
}

template <typename... Args>
std::string MemSPrintf(const char* fmt, Args... args) {  // args should be null terminated strings or simple types
  char* ret = nullptr;
  int res = vasprintf(&ret, fmt, normalize_for_printf(args)...);
  if (res == -1) {
    DNOTREACHED();
    return std::string();
  }

  size_t stabled_res = static_cast<size_t>(res);
  std::string str(ret, stabled_res);
  free(ret);
  return str;
}

}  // namespace common
