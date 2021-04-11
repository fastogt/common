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

#include <common/text_decoders/iedcoder.h>

namespace common {

const std::array<const char*, ENCODER_DECODER_NUM_TYPES> edecoder_types = {
    {"NoComression", "Base64", "Zlib", "BZip2", "LZ4", "Snappy", "Hex", "XHex", "Unicode", "UUnicode", "HtmlEscape"}};

std::string ConvertToString(EDType type) {
  if (type >= 0 && type < edecoder_types.size()) {
    return edecoder_types[type];
  }

  DNOTREACHED() << "Unknown EDCoder type:" << type;
  return "Unknown";
}

bool ConvertFromString(const std::string& from, EDType* out) {
  if (!out) {
    return false;
  }

  for (size_t i = 0; i < edecoder_types.size(); ++i) {
    if (from == edecoder_types[i]) {
      *out = static_cast<EDType>(i);
      return true;
    }
  }

  return false;
}

IEDcoder::~IEDcoder() {}

IEDcoder::IEDcoder(EDType type) : type_(type) {}

Error IEDcoder::Encode(const StringPiece& data, char_buffer_t* out) {
  if (!out || data.empty()) {
    return make_error_inval();
  }

  return DoEncode(data, out);
}

Error IEDcoder::Decode(const StringPiece& data, char_buffer_t* out) {
  if (!out || data.empty()) {
    return make_error_inval();
  }

  return DoDecode(data, out);
}

Error IEDcoder::Encode(const char_buffer_t& data, char_buffer_t* out) {
  if (!out || data.empty()) {
    return make_error_inval();
  }

  return DoEncode(data, out);
}
Error IEDcoder::Decode(const char_buffer_t& data, char_buffer_t* out) {
  if (!out || data.empty()) {
    return make_error_inval();
  }

  return DoDecode(data, out);
}

EDType IEDcoder::GetType() const {
  return type_;
}

}  // namespace common
