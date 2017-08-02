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

#include <common/convert2string.h>
#include <common/sprintf.h>  // for SNPrintf
#include <common/utils.h>

namespace common {

Error EncodeHex(const std::string& data, bool is_lower, std::string* out) {
  if (!out) {
    return make_inval_error_value(ErrorValue::E_ERROR);
  }

  *out = utils::hex::encode(data, is_lower);
  return Error();
}

Error EncodeHex(const buffer_t& data, bool is_lower, buffer_t* out) {
  if (!out) {
    return make_inval_error_value(ErrorValue::E_ERROR);
  }

  *out = utils::hex::encode(data, is_lower);
  return Error();
}

Error DecodeHex(const buffer_t& data, buffer_t* out) {
  if (!out) {
    return make_inval_error_value(ErrorValue::E_ERROR);
  }

  *out = utils::hex::decode(data);
  return Error();
}

Error DecodeHex(const std::string& data, std::string* out) {
  if (!out) {
    return make_inval_error_value(ErrorValue::E_ERROR);
  }

  *out = utils::hex::decode(data);
  return Error();
}

Error EncodeBase64(const buffer_t& data, buffer_t* out) {
  if (data.empty() || !out) {
    return make_inval_error_value(ErrorValue::E_ERROR);
  }

  *out = utils::base64::encode64(data);
  return Error();
}

Error DecodeBase64(const buffer_t& data, buffer_t* out) {
  if (data.empty() || !out) {
    return make_inval_error_value(ErrorValue::E_ERROR);
  }

  *out = utils::base64::decode64(data);
  return Error();
}

}  // namespace common

#ifdef HAVE_ZLIB

#include <zconf.h>  // for Byte, Bytef

#define BUFFER_SIZE 1024 * 32

namespace common {

Error EncodeZlib(const buffer_t& data, buffer_t* out, int compressionlevel) {
  if (data.empty() || !out) {
    return make_inval_error_value(ErrorValue::E_ERROR);
  }

  z_stream zs;  // z_stream is zlib's control structure
  memset(&zs, 0, sizeof(zs));

  if (deflateInit(&zs, compressionlevel) != Z_OK) {
    return common::make_error_value("Zlib init error", common::ErrorValue::E_ERROR);
  }
#if defined(ZLIB_CONST) && !defined(z_const)
  zs.next_in = data.data();
#else
  zs.next_in = const_cast<Bytef*>(data.data());
#endif
  zs.avail_in = data.size();  // set the z_stream's input

  int ret;
  Bytef* outbuffer = new Bytef[BUFFER_SIZE];

  buffer_t lout;

  // retrieve the compressed bytes blockwise
  do {
    zs.next_out = outbuffer;
    zs.avail_out = BUFFER_SIZE;

    ret = deflate(&zs, Z_FINISH);

    if (lout.size() < zs.total_out) {
      // append the block to the output string
      lout = MAKE_BUFFER_SIZE(outbuffer, zs.total_out - lout.size());
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

Error DecodeZlib(const buffer_t& data, buffer_t* out) {
  if (data.empty() || !out) {
    return make_inval_error_value(ErrorValue::E_ERROR);
  }

  z_stream zs;  // z_stream is zlib's control structure
  memset(&zs, 0, sizeof(zs));

  if (inflateInit(&zs) != Z_OK) {
    return common::make_error_value("Zlib init error", common::ErrorValue::E_ERROR);
  }

#if defined(ZLIB_CONST) && !defined(z_const)
  zs.next_in = data.data();
#else
  zs.next_in = const_cast<Bytef*>(data.data());
#endif
  zs.avail_in = data.size();

  int ret;
  Bytef* outbuffer = new Bytef[BUFFER_SIZE];
  buffer_t lout;
  // get the decompressed bytes blockwise using repeated calls to inflate
  do {
    zs.next_out = outbuffer;
    zs.avail_out = BUFFER_SIZE;

    ret = inflate(&zs, 0);

    if (lout.size() < zs.total_out) {
      lout = MAKE_BUFFER_SIZE(outbuffer, zs.total_out - lout.size());
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
    return make_inval_error_value(ErrorValue::E_ERROR);
  }

  std::string lout;
  size_t writed_bytes = snappy::Compress(data.c_str(), data.length(), &lout);
  if (writed_bytes == 0) {
    return make_inval_error_value(ErrorValue::E_ERROR);
  }

  *out = lout;
  return Error();
}

Error DecodeSnappy(const std::string& data, std::string* out) {
  if (data.empty() || !out) {
    return make_inval_error_value(ErrorValue::E_ERROR);
  }

  std::string lout;
  size_t writed_bytes = snappy::Uncompress(data.c_str(), data.length(), &lout);
  if (writed_bytes == 0) {
    return make_inval_error_value(ErrorValue::E_ERROR);
  }

  *out = lout;
  return Error();
}

}  // namespace common
#endif
