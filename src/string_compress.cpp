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

#include <common/string_compress.h>

#include <string.h>  // for memset

#include <string>  // for string

#include <common/sprintf.h>  // for SNPrintf
#include <common/value.h>    // for ErrorValue, etc

#ifdef HAVE_ZLIB

#include <zconf.h>  // for Byte, Bytef

#define BUFFER_SIZE 1024 * 32

namespace common {

Error EncodeZlib(const std::string& data, std::string* out, int compressionlevel) {
  if (data.empty() || !out) {
    return make_error_value("Invalid input argument(s)", ErrorValue::E_ERROR);
  }

  z_stream zs;  // z_stream is zlib's control structure
  memset(&zs, 0, sizeof(zs));

  if (deflateInit(&zs, compressionlevel) != Z_OK) {
    return common::make_error_value("Zlib init error", common::ErrorValue::E_ERROR);
  }

  zs.next_in = reinterpret_cast<Byte*>(const_cast<char*>(data.c_str()));
  zs.avail_in = data.size();  // set the z_stream's input

  int ret;
  char* outbuffer = new char[BUFFER_SIZE];

  std::string lout;

  // retrieve the compressed bytes blockwise
  do {
    zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
    zs.avail_out = BUFFER_SIZE;

    ret = deflate(&zs, Z_FINISH);

    if (lout.size() < zs.total_out) {
      // append the block to the output string
      lout.append(outbuffer, zs.total_out - lout.size());
    }
  } while (ret == Z_OK);

  deflateEnd(&zs);

  delete[] outbuffer;

  if (ret != Z_STREAM_END) {  // an error occurred that was not EOF
    char buffer[256];
    SNPrintf(buffer, sizeof(buffer), "Zlib encode error returned code: %d", ret);
    return common::make_error_value(buffer, common::ErrorValue::E_ERROR);
  }

  *out = lout;
  return Error();
}

Error DecodeZlib(const std::string& data, std::string* out) {
  if (data.empty() || !out) {
    return make_error_value("Invalid input argument(s)", ErrorValue::E_ERROR);
  }

  z_stream zs;  // z_stream is zlib's control structure
  memset(&zs, 0, sizeof(zs));

  if (inflateInit(&zs) != Z_OK) {
    return common::make_error_value("Zlib init error", common::ErrorValue::E_ERROR);
  }

  zs.next_in = reinterpret_cast<Byte*>(const_cast<char*>(data.data()));
  zs.avail_in = data.size();

  int ret;
  char* outbuffer = new char[BUFFER_SIZE];
  std::string lout;
  // get the decompressed bytes blockwise using repeated calls to inflate
  do {
    zs.next_out = reinterpret_cast<Bytef*>(outbuffer);
    zs.avail_out = BUFFER_SIZE;

    ret = inflate(&zs, 0);

    if (lout.size() < zs.total_out) {
      lout.append(outbuffer, zs.total_out - lout.size());
    }
  } while (ret == Z_OK);

  inflateEnd(&zs);

  delete[] outbuffer;

  if (ret != Z_STREAM_END) {  // an error occurred that was not EOF
    char buffer[256];
    SNPrintf(buffer, sizeof(buffer), "Zlib decode error returned code: %d", ret);
    return common::make_error_value(buffer, common::ErrorValue::E_ERROR);
  }

  *out = lout;
  return Error();
}

}  // namespace common

#endif

#ifdef HAVE_SNAPPY
#include <snappy.h>

namespace common {

Error EncodeSnappy(const std::string& data, std::string* out) {
  if (data.empty() || !out) {
    return make_error_value("Invalid input argument(s)", ErrorValue::E_ERROR);
  }

  std::string lout;
  size_t writed_bytes = snappy::Compress(data.c_str(), data.length(), &lout);
  if (writed_bytes == 0) {
    return make_error_value("Invalid input argument(s)", ErrorValue::E_ERROR);
  }

  *out = lout;
  return Error();
}

Error DecodeSnappy(const std::string& data, std::string* out) {
  if (data.empty() || !out) {
    return make_error_value("Invalid input argument(s)", ErrorValue::E_ERROR);
  }

  std::string lout;
  size_t writed_bytes = snappy::Uncompress(data.c_str(), data.length(), &lout);
  if (writed_bytes == 0) {
    return make_error_value("Invalid input argument(s)", ErrorValue::E_ERROR);
  }

  *out = lout;
  return Error();
}

}  // namespace common
#endif
