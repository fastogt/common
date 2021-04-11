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

#include <common/compress/snappy_compress.h>

#if defined(HAVE_SNAPPY)
#include <limits>

#include <snappy.h>

namespace common {
namespace {
template <typename CHAR, typename STR2>
Error EncodeSnappyT(const CHAR* input, size_t input_length, STR2* output) {
  if (!input || !output || input_length > std::numeric_limits<uint32_t>::max()) {
    // Can't compress more than 4GB
    return make_error_inval();
  }

  output->resize(snappy::MaxCompressedLength(input_length));
  size_t outlen;
  const char* stabled_input = reinterpret_cast<const char*>(input);
  char* stabled_out = reinterpret_cast<char*>(&(*output)[0]);
  snappy::RawCompress(stabled_input, input_length, stabled_out, &outlen);
  output->resize(outlen);
  return Error();
}

template <typename CHAR, typename STR2>
Error DecodeSnappyT(const CHAR* input, size_t input_length, STR2* out) {
  if (!input || !out) {
    return make_error_inval();
  }

  const char* stabled_input = reinterpret_cast<const char*>(input);
  std::string lout;
  bool is_ok = snappy::Uncompress(stabled_input, input_length, &lout);
  if (!is_ok) {
    size_t uncompressed_len;
    bool get_un = snappy::GetUncompressedLength(stabled_input, input_length, &uncompressed_len);
    if (get_un) {
      size_t diff = input_length - uncompressed_len;
      *out = STR2(stabled_input + diff, stabled_input + input_length);
      return Error();
    }
    return make_error_inval();
  }

  *out = STR2(lout.begin(), lout.end());
  return Error();
}
}  // namespace

namespace compress {

Error EncodeSnappy(const StringPiece& data, char_buffer_t* out) {
  return EncodeSnappyT(data.data(), data.size(), out);
}

Error DecodeSnappy(const StringPiece& data, char_buffer_t* out) {
  return DecodeSnappyT(data.data(), data.size(), out);
}

Error EncodeSnappy(const char_buffer_t& data, char_buffer_t* out) {
  return EncodeSnappyT(data.data(), data.size(), out);
}

Error DecodeSnappy(const char_buffer_t& data, char_buffer_t* out) {
  return DecodeSnappyT(data.data(), data.size(), out);
}

}  // namespace compress
}  // namespace common
#endif
