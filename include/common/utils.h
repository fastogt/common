/*  Copyright (C) 2014-2022 FastoGT. All right reserved.

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

#include <common/string_piece.h>
#include <common/types.h>  // for buffer_t, byte_t

#include <string>

#define HAS_MEMBER(CLASS_NAME, FUNC_MUST_BE)                                                  \
  COMPILE_ASSERT(std::is_member_function_pointer<decltype(&CLASS_NAME::FUNC_MUST_BE)>::value, \
                 "Class does not contain member " #FUNC_MUST_BE "!")

namespace common {
namespace utils {
namespace hash {
uint64_t crc64(uint64_t crc, const byte_t* data, size_t lenght);
uint64_t crc64(uint64_t crc, const buffer_t& data);
}  // namespace hash

namespace base64 {
bool encode64(const StringPiece& input, char_buffer_t* out);
bool decode64(const StringPiece& input, char_buffer_t* out);
bool encode64(const char_buffer_t& input, char_buffer_t* out);
bool decode64(const char_buffer_t& input, char_buffer_t* out);

//
bool encode64(const StringPiece& input, std::string* out);
bool encode64(const char_buffer_t& input, std::string* out);
}  // namespace base64

namespace html {
bool encode(const StringPiece& input, char_buffer_t* out);
bool decode(const StringPiece& input, char_buffer_t* out);
bool encode(const char_buffer_t& input, char_buffer_t* out);
bool decode(const char_buffer_t& input, char_buffer_t* out);
}  // namespace html

char* strdupornull(const std::string& src);
char* strdupornull(const char* src);
void freeifnotnull(void* ptr);
}  // namespace utils

#if defined(OS_POSIX)
bool create_as_daemon();
#endif
long get_current_process_pid();
}  // namespace common
