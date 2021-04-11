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

#include <common/compress/bzip2_compress.h>

#if defined(HAVE_BZIP2)
#include <string.h>

#include <limits>

#include <bzlib.h>

#include <common/compress/coding.h>

namespace common {
namespace {
template <typename CHAR, typename STR2>
Error EncodeBZip2T(const CHAR* input, size_t input_length, bool sized, STR2* output) {
  if (!input || !output || input_length > std::numeric_limits<uint32_t>::max()) {
    // Can't compress more than 4GB
    return make_error_inval();
  }

  output->clear();
  size_t output_header_len;
  uint32_t output_len = 0;
  if (sized) {
    output_header_len = compress::PutDecompressedSizeInfo(output, static_cast<uint32_t>(input_length));
  } else {
    output_header_len = 0;
  }

  // Resize output to be the plain data length.
  // This may not be big enough if the compression actually expands data.
  output_len = output_header_len + input_length;
  output->resize(output_len);

  bz_stream _stream;
  memset(&_stream, 0, sizeof(bz_stream));

  // Block size 1 is 100K.
  // 0 is for silent.
  // 30 is the default workFactor
  int st = BZ2_bzCompressInit(&_stream, 1, 0, 30);
  if (st != BZ_OK) {
    return make_error("BZip2 compress internal error");
  }

  // Compress the input, and put compressed data in output.
  _stream.next_in = const_cast<char*>(reinterpret_cast<const char*>(input));
  _stream.avail_in = static_cast<unsigned int>(input_length);

  // Initialize the output size.
  _stream.avail_out = static_cast<unsigned int>(input_length);
  _stream.next_out = reinterpret_cast<char*>(&(*output)[output_header_len]);

  bool done = false;
  while (!done) {
    st = BZ2_bzCompress(&_stream, BZ_FINISH);
    switch (st) {
      case BZ_STREAM_END:
        done = true;
        break;
      case BZ_OK: {
        // No output space. Increase the output space by 20%.
        // We should never run out of output space if

        size_t old_sz = output_len;
        uint32_t output_len_delta = output_len / 5;
        output_len += output_len_delta < 10 ? 10 : output_len_delta;

        output->resize(output_len);

        // Set more output.
        _stream.next_out = reinterpret_cast<char*>(&(*output)[old_sz]);
        _stream.avail_out = static_cast<unsigned int>(output_len - old_sz);
        break;
      }
      default:
        BZ2_bzCompressEnd(&_stream);
        return make_error("ZLIB compress internal error");
    }
  }
  // The only return value we really care about is BZ_STREAM_END.
  // BZ_FINISH_OK means insufficient output space. This means the compression
  // is bigger than decompressed size. Just fail the compression in that case.

  size_t compressed_size = output_len - _stream.avail_out;
  output->resize(compressed_size + output_header_len);
  BZ2_bzCompressEnd(&_stream);
  return Error();
}

template <typename CHAR, typename STR2>
Error DecodeBZip2T(const CHAR* input, size_t input_length, bool sized, STR2* out) {
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

  bz_stream _stream;
  memset(&_stream, 0, sizeof(bz_stream));

  int st = BZ2_bzDecompressInit(&_stream, 0, 0);
  if (st != BZ_OK) {
    return make_error("BZip2 decompress internal error");
  }

  _stream.next_in = const_cast<char*>(reinterpret_cast<const char*>(input));
  _stream.avail_in = static_cast<unsigned int>(input_length);

  char* output = new char[output_len];

  _stream.next_out = reinterpret_cast<char*>(output);
  _stream.avail_out = static_cast<unsigned int>(output_len);

  bool done = false;
  while (!done) {
    st = BZ2_bzDecompress(&_stream);
    switch (st) {
      case BZ_STREAM_END:
        done = true;
        break;
      case BZ_OK: {
        // No output space. Increase the output space by 20%.
        // We should never run out of output space if
        // compress_format_version == 2
        uint32_t old_sz = output_len;
        output_len = output_len * 1.2;
        char* tmp = new char[output_len];
        memcpy(tmp, output, old_sz);
        delete[] output;
        output = tmp;

        // Set more output.
        _stream.next_out = reinterpret_cast<char*>(output + old_sz);
        _stream.avail_out = static_cast<unsigned int>(output_len - old_sz);
        break;
      }
      default:
        delete[] output;
        BZ2_bzDecompressEnd(&_stream);
        return make_error("BZip2 decompress internal error");
    }
  }

  if (sized) {
    // If we encoded decompressed block size, we should have no bytes left
    DCHECK_EQ(_stream.avail_out, 0);
  }
  uint32_t decompress_size = output_len - _stream.avail_out;
  *out = STR2(output, output + decompress_size);
  delete[] output;
  BZ2_bzDecompressEnd(&_stream);
  return Error();
}
}  // namespace

namespace compress {

Error EncodeBZip2(const StringPiece& data, bool sized, char_buffer_t* out) {
  return EncodeBZip2T(data.data(), data.size(), sized, out);
}

Error DecodeBZip2(const StringPiece& data, bool sized, char_buffer_t* out) {
  return DecodeBZip2T(data.data(), data.size(), sized, out);
}

Error EncodeBZip2(const char_buffer_t& data, bool sized, char_buffer_t* out) {
  return EncodeBZip2T(data.data(), data.size(), sized, out);
}

Error DecodeBZip2(const char_buffer_t& data, bool sized, char_buffer_t* out) {
  return DecodeBZip2T(data.data(), data.size(), sized, out);
}

}  // namespace compress
}  // namespace common
#endif
