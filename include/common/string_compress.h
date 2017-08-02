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

#include <string>

#include <common/error.h>

namespace common {

Error EncodeHex(const buffer_t& data, bool is_lower, buffer_t* out) WARN_UNUSED_RESULT;
Error EncodeHex(const std::string& data, bool is_lower, std::string* out) WARN_UNUSED_RESULT;

Error DecodeHex(const buffer_t& data, buffer_t* out) WARN_UNUSED_RESULT;
Error DecodeHex(const std::string& data, std::string* out) WARN_UNUSED_RESULT;

Error EncodeBase64(const buffer_t& data, buffer_t* out) WARN_UNUSED_RESULT;
Error DecodeBase64(const buffer_t& data, buffer_t* out) WARN_UNUSED_RESULT;

}  // namespace common

#ifdef HAVE_ZLIB

#include <zlib.h>

namespace common {
Error EncodeZlib(const buffer_t& data, buffer_t* out, int compressionlevel = Z_BEST_COMPRESSION) WARN_UNUSED_RESULT;
Error DecodeZlib(const buffer_t& data, buffer_t* out) WARN_UNUSED_RESULT;
}  // namespace common

#endif

#ifdef HAVE_SNAPPY
namespace common {
Error EncodeSnappy(const std::string& data, std::string* out) WARN_UNUSED_RESULT;
Error DecodeSnappy(const std::string& data, std::string* out) WARN_UNUSED_RESULT;
}  // namespace common
#endif
