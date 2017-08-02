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

#include <common/text_decoders/iedcoder.h>

#include <stdint.h>  // for uint32_t
#include <string>    // for string, operator==

#include <common/convert2string.h>

#include <common/text_decoders/base64_edcoder.h>  // for Base64EDcoder
#include <common/text_decoders/compress_snappy_edcoder.h>
#include <common/text_decoders/compress_zlib_edcoder.h>  // for CompressZlibEDcoder
#include <common/text_decoders/hex_edcoder.h>            // for HexEDcoder
#include <common/text_decoders/html_edcoder.h>           // for HtmlEscEDcoder
#include <common/text_decoders/msgpack_edcoder.h>        // for MsgPackEDcoder

namespace common {
std::string ConvertToString(EDTypes t) {
  return EDecoderTypes[t];
}

bool ConvertFromString(const std::string& from, EDTypes* out) {
  if (!out) {
    return false;
  }

  for (uint32_t i = 0; i < SIZEOFMASS(EDecoderTypes); ++i) {
    if (from == EDecoderTypes[i]) {
      *out = static_cast<EDTypes>(i);
      return true;
    }
  }

  return false;
}

IEDcoder::~IEDcoder() {}

IEDcoder::IEDcoder(EDTypes type) : type_(type) {}

Error IEDcoder::Encode(const std::string& data, std::string* out) {
  if (data.empty()) {
    return make_inval_error_value(ErrorValue::E_ERROR);
  }

  return EncodeImpl(data, out);
}

Error IEDcoder::Decode(const std::string& data, std::string* out) {
  if (data.empty()) {
    return make_inval_error_value(ErrorValue::E_ERROR);
  }

  return DecodeImpl(data, out);
}

EDTypes IEDcoder::GetType() const {
  return type_;
}

IEDcoder* IEDcoder::CreateEDCoder(EDTypes type) {
  if (type == Base64) {
    return new Base64EDcoder;
  } else if (type == CompressZlib) {
    return new CompressZlibEDcoder;
  } else if (type == CompressSnappy) {
    return new CompressSnappyEDcoder;
  } else if (type == Hex) {
    return new HexEDcoder;
  } else if (type == MsgPack) {
    return new MsgPackEDcoder;
  } else if (type == HtmlEsc) {
    return new HtmlEscEDcoder;
  }

  NOTREACHED();
  return nullptr;
}

IEDcoder* IEDcoder::CreateEDCoder(const std::string& name) {
  EDTypes t;
  if (!ConvertFromString(name, &t)) {
    NOTREACHED();
    return nullptr;
  }

  return CreateEDCoder(t);
}

}  // namespace common
