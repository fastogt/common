/*  Copyright (C) 2014-2022 FastoGT. All right reserved.

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

#include <common/compress/unicode.h>
#include <common/convert2string.h>
#include <common/text_decoders/unicode_edcoder.h>

namespace common {

UnicodeEDcoder::UnicodeEDcoder(bool is_lower) : IEDcoder(ED_UNICODE), is_lower_(is_lower) {}

Error UnicodeEDcoder::DoEncode(const StringPiece& data, char_buffer_t* out) {
  string16 sdata = ConvertToString16(data);
  return compress::EncodeUnicode(sdata, is_lower_, out);
}

Error UnicodeEDcoder::DoDecode(const StringPiece& data, char_buffer_t* out) {
  string16 sdata;
  Error err = compress::DecodeUnicode(data, &sdata);
  if (err) {
    return err;
  }

  *out = ConvertToCharBytes(sdata);
  return Error();
}

Error UnicodeEDcoder::DoEncode(const char_buffer_t& data, char_buffer_t* out) {
  string16 sdata = ConvertToString16(data);
  return compress::EncodeUnicode(sdata, is_lower_, out);
}

Error UnicodeEDcoder::DoDecode(const char_buffer_t& data, char_buffer_t* out) {
  string16 sdata;
  Error err = compress::DecodeUnicode(StringPiece(data.data(), data.size()), &sdata);
  if (err) {
    return err;
  }

  *out = ConvertToCharBytes(sdata);
  return Error();
}

}  // namespace common
