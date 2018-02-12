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

#include <common/compress/lz4_compress.h>

#ifdef HAVE_LZ4

#include <lz4.h>

namespace common {
namespace compress {

namespace {
template <typename STR, typename STR2>
Error EncodeLZ4T(const STR& data, STR2* out) {
  if (data.empty() || !out) {
    return make_error_inval();
  }

  const size_t src_size = data.size() + 1;
  const size_t max_dst_size = LZ4_compressBound(src_size);
  char* dst_data = new char[max_dst_size];
  const char* src_data = reinterpret_cast<const char*>(data.data());
  int processed = LZ4_compress_default(src_data, dst_data, src_size, max_dst_size);
  if (processed < 0) {
    return make_error("LZ4 compress internal error");
  }

  *out = STR2(dst_data, dst_data + processed);
  return Error();
}

template <typename STR, typename STR2>
Error DecodeLZ4T(const STR& data, STR2* out) {
  if (data.empty() || !out) {
    return make_error_inval();
  }

  const size_t src_size = data.size() + 1;
  char* dst_data = new char[src_size];
  const char* source_data = reinterpret_cast<const char*>(data.data());
  int processed = LZ4_decompress_fast(source_data, dst_data, src_size);
  if (processed < 0) {
    return make_error("LZ4 decompress internal error");
  }

  *out = STR2(dst_data, dst_data + processed);
  return Error();
}
}  // namespace

Error EncodeLZ4(const buffer_t& data, buffer_t* out) {
  return EncodeLZ4T(data, out);
}

Error DecodeLZ4(const buffer_t& data, buffer_t* out) {
  return DecodeLZ4T(data, out);
}

Error EncodeLZ4(const StringPiece& data, std::string* out) {
  return EncodeLZ4T(data, out);
}

Error DecodeLZ4(const StringPiece& data, std::string* out) {
  return DecodeLZ4T(data, out);
}

}  // namespace compress
}  // namespace common

#endif
