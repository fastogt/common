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

#include <common/compress/zlib_compress.h>

#if defined(HAVE_ZLIB)

#include <string.h>

#include <common/compress/coding.h>

#include <limits>

#define WINDOW_BITS 15
#define GZIP_ENCODING 16

namespace common {
namespace compress {

namespace {

template <typename CHAR, typename STR2>
Error EncodeZlibT(const CHAR* input,
                  size_t input_length,
                  bool sized,
                  uint8_t def,
                  STR2* output,
                  int compression_level) {
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

  // The memLevel parameter specifies how much memory should be allocated for
  // the internal compression state.
  // memLevel=1 uses minimum memory but is slow and reduces compression ratio.
  // memLevel=9 uses maximum memory for optimal speed.
  // The default value is 8. See zconf.h for more details.
  static const int mem_level = 8;
  z_stream _stream;
  memset(&_stream, 0, sizeof(z_stream));

  int st = deflateInit2(&_stream, compression_level, Z_DEFLATED, WINDOW_BITS | def, mem_level, Z_DEFAULT_STRATEGY);
  if (st != Z_OK) {
    return make_error("ZLIB compress internal error");
  }

  // Compress the input, and put compressed data in output.
  _stream.next_in = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(input));
  _stream.avail_in = static_cast<unsigned int>(input_length);

  // Initialize the output size.
  _stream.avail_out = static_cast<unsigned int>(output_len);
  _stream.next_out = reinterpret_cast<Bytef*>(&(*output)[output_header_len]);

  bool done = false;
  while (!done) {
    st = deflate(&_stream, Z_FINISH);
    switch (st) {
      case Z_STREAM_END:
        done = true;
        break;
      case Z_OK: {
        // No output space. Increase the output space by 20%.
        // We should never run out of output space if

        size_t old_sz = output_len;
        uint32_t output_len_delta = output_len / 5;
        output_len += output_len_delta < 10 ? 10 : output_len_delta;

        output->resize(output_len);

        // Set more output.
        _stream.next_out = reinterpret_cast<Bytef*>(&(*output)[old_sz]);
        _stream.avail_out = static_cast<unsigned int>(output_len - old_sz);
        break;
      }
      case Z_BUF_ERROR:
      default:
        delete[] output;
        deflateEnd(&_stream);
        return make_error("ZLIB compress internal error");
    }
  }

  size_t compressed_size = output_len - _stream.avail_out;
  output->resize(compressed_size + output_header_len);
  deflateEnd(&_stream);
  return Error();
}

template <typename CHAR, typename STR2>
Error DecodeZlibT(const CHAR* input, size_t input_length, bool sized, STR2* out) {
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

  z_stream _stream;
  memset(&_stream, 0, sizeof(z_stream));

  // For raw inflate, the windowBits should be -8..-15.
  // If windowBits is bigger than zero, it will use either zlib
  // header or gzip header. Adding 32 to it will do automatic detection.
  int st = inflateInit2(&_stream, WINDOW_BITS > 0 ? WINDOW_BITS + 32 : WINDOW_BITS);
  if (st != Z_OK) {
    return make_error("ZLIB decompress internal error");
  }

  _stream.next_in = const_cast<Bytef*>(reinterpret_cast<const Bytef*>(input));
  _stream.avail_in = static_cast<unsigned int>(input_length);

  char* output = new char[output_len];

  _stream.next_out = reinterpret_cast<Bytef*>(output);
  _stream.avail_out = static_cast<unsigned int>(output_len);

  bool done = false;
  while (!done) {
    st = inflate(&_stream, Z_SYNC_FLUSH);
    switch (st) {
      case Z_STREAM_END:
        done = true;
        break;
      case Z_OK: {
        // No output space. Increase the output space by 20%.
        // We should never run out of output space if

        size_t old_sz = output_len;
        uint32_t output_len_delta = output_len / 5;
        output_len += output_len_delta < 10 ? 10 : output_len_delta;
        char* tmp = new char[output_len];
        memcpy(tmp, output, old_sz);
        delete[] output;
        output = tmp;

        // Set more output.
        _stream.next_out = reinterpret_cast<Bytef*>(output + old_sz);
        _stream.avail_out = static_cast<unsigned int>(output_len - old_sz);
        break;
      }
      case Z_BUF_ERROR:
      default:
        delete[] output;
        inflateEnd(&_stream);
        return make_error("ZLIB decompress internal error");
    }
  }

  if (sized) {
    // If we encoded decompressed block size, we should have no bytes left
    DCHECK_EQ(_stream.avail_out, 0);
  }

  uint32_t decompress_size = output_len - _stream.avail_out;
  *out = STR2(output, output + decompress_size);
  delete[] output;
  inflateEnd(&_stream);
  return Error();
}
}  // namespace

Error EncodeZlib(const StringPiece& data, bool sized, uint8_t def, char_buffer_t* out, int compression_level) {
  return EncodeZlibT(data.data(), data.size(), sized, def, out, compression_level);
}

Error DecodeZlib(const StringPiece& data, bool sized, char_buffer_t* out) {
  return DecodeZlibT(data.data(), data.size(), sized, out);
}

Error EncodeZlib(const char_buffer_t& data, bool sized, uint8_t def, char_buffer_t* out, int compression_level) {
  return EncodeZlibT(data.data(), data.size(), sized, def, out, compression_level);
}

Error DecodeZlib(const char_buffer_t& data, bool sized, char_buffer_t* out) {
  return DecodeZlibT(data.data(), data.size(), sized, out);
}

}  // namespace compress
}  // namespace common

#endif
