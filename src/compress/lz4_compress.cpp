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

#include <limits>

#include <common/compress/coding.h>

namespace common {
namespace {
template <typename CHAR, typename STR2>
Error EncodeLZ4T(const CHAR* input, size_t input_length, bool sized, STR2* output) {
  if (!input || !output || input_length > std::numeric_limits<uint32_t>::max()) {
    // Can't compress more than 4GB
    return make_error_inval();
  }

  int stabled_input_size = static_cast<int>(input_length);
  size_t output_header_len;
  if (sized) {
    output_header_len = compress::PutDecompressedSizeInfo(output, static_cast<uint32_t>(input_length));
  } else {
    output_header_len = 0;
  }

  int compress_bound = LZ4_compressBound(stabled_input_size);
  output->resize(static_cast<size_t>(output_header_len + compress_bound));

  int outlen;
  const char* stabled_input = reinterpret_cast<const char*>(input);
  char* stabled_output = reinterpret_cast<char*>(&(*output)[output_header_len]);
#if LZ4_VERSION_NUMBER >= 10400  // r124+
  LZ4_stream_t* stream = LZ4_createStream();
#if LZ4_VERSION_NUMBER >= 10700  // r129+
  outlen = LZ4_compress_fast_continue(stream, stabled_input, stabled_output, stabled_input_size, compress_bound, 1);
#else  // up to r128
  outlen =
      LZ4_compress_limitedOutput_continue(stream, stabled_input, stabled_output, stabled_input_size, compress_bound);
#endif
  LZ4_freeStream(stream);
#else   // up to r123
  outlen = LZ4_compress_limitedOutput(stabled_input, stabled_output, stabled_input_size, compress_bound);
#endif  // LZ4_VERSION_NUMBER >= 10400

  if (outlen == 0) {
    return make_error("LZ4 compress internal error");
  }
  output->resize(static_cast<size_t>(output_header_len + outlen));
  return Error();
}

template <typename CHAR, typename STR2>
Error DecodeLZ4T(const CHAR* input, size_t input_length, bool sized, STR2* out) {
  if (!input || !out) {
    return make_error_inval();
  }

  uint32_t output_len = 0;
  if (sized) {
    // new encoding, using varint32 to store size information
    if (!compress::GetDecompressedSizeInfo(&input, &input_length, &output_len)) {
      return make_error_inval();
    }
  } else {
    output_len = input_length * 8;  // may be help
  }

  CHAR* output = new CHAR[output_len];
  const char* stabled_input = reinterpret_cast<const char*>(input);
  char* stabled_output = reinterpret_cast<char*>(output);
#if LZ4_VERSION_NUMBER >= 10400  // r124+
  LZ4_streamDecode_t* stream = LZ4_createStreamDecode();
  int decompress_size = LZ4_decompress_safe_continue(stream, stabled_input, stabled_output,
                                                     static_cast<int>(input_length), static_cast<int>(output_len));
  LZ4_freeStreamDecode(stream);
#else   // up to r123
  int decompress_size =
      LZ4_decompress_safe(stabled_input, stabled_output, static_cast<int>(input_length), static_cast<int>(output_len));
#endif  // LZ4_VERSION_NUMBER >= 10400

  if (decompress_size < 0) {
    delete[] output;
    return make_error("LZ4 decompress_size internal error");
  }

  if (sized) {
    DCHECK(decompress_size == static_cast<int>(output_len));
  }
  *out = STR2(output, output + decompress_size);
  delete[] output;
  return Error();
}
}  // namespace

namespace compress {

Error EncodeLZ4(const buffer_t& data, bool sized, buffer_t* out) {
  return EncodeLZ4T(data.data(), data.size(), sized, out);
}

Error DecodeLZ4(const buffer_t& data, bool sized, buffer_t* out) {
  return DecodeLZ4T(data.data(), data.size(), sized, out);
}

Error EncodeLZ4(const StringPiece& data, bool sized, std::string* out) {
  return EncodeLZ4T(data.data(), data.size(), sized, out);
}

Error DecodeLZ4(const StringPiece& data, bool sized, std::string* out) {
  return DecodeLZ4T(data.data(), data.size(), sized, out);
}

}  // namespace compress
}  // namespace common

#endif
