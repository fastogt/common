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

#pragma once

#include <array>
#include <string>

#include <common/error.h>  // for Error
#include <common/string_piece.h>
#include <common/types.h>

namespace common {

enum EDType : unsigned {
  ED_NONE = 0,
  ED_BASE64,
  ED_ZLIB,
  ED_BZIP2,
  ED_LZ4,
  ED_SNAPPY,
  ED_HEX,
  ED_XHEX,
  ED_UNICODE,
  ED_UUNICODE,
  ED_HTML_ESC,
  ENCODER_DECODER_NUM_TYPES
};
extern const std::array<const char*, ENCODER_DECODER_NUM_TYPES> edecoder_types;

class IEDcoder {
 public:
  Error Encode(const StringPiece& data, char_buffer_t* out) WARN_UNUSED_RESULT;
  Error Decode(const StringPiece& data, char_buffer_t* out) WARN_UNUSED_RESULT;
  Error Encode(const char_buffer_t& data, char_buffer_t* out) WARN_UNUSED_RESULT;
  Error Decode(const char_buffer_t& data, char_buffer_t* out) WARN_UNUSED_RESULT;

  EDType GetType() const;

  virtual ~IEDcoder();

 protected:
  explicit IEDcoder(EDType type);

 private:
  virtual Error DoEncode(const StringPiece& data, char_buffer_t* out) = 0;
  virtual Error DoDecode(const StringPiece& data, char_buffer_t* out) = 0;
  virtual Error DoEncode(const char_buffer_t& data, char_buffer_t* out) = 0;
  virtual Error DoDecode(const char_buffer_t& data, char_buffer_t* out) = 0;

  const EDType type_;
};

std::string ConvertToString(EDType type);
bool ConvertFromString(const std::string& from, EDType* out) WARN_UNUSED_RESULT;

}  // namespace common
