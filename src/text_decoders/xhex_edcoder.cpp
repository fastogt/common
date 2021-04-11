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

#include <common/text_decoders/xhex_edcoder.h>

#include <common/compress/xhex.h>

namespace common {

XHexEDcoder::XHexEDcoder(bool is_lower) : IEDcoder(ED_XHEX), is_lower_(is_lower) {}

Error XHexEDcoder::DoEncode(const StringPiece& data, char_buffer_t* out) {
  return compress::EncodeXHex(data, is_lower_, out);
}

Error XHexEDcoder::DoDecode(const StringPiece& data, char_buffer_t* out) {
  return compress::DecodeXHex(data, out);
}

Error XHexEDcoder::DoEncode(const char_buffer_t& data, char_buffer_t* out) {
  return compress::EncodeXHex(data, is_lower_, out);
}

Error XHexEDcoder::DoDecode(const char_buffer_t& data, char_buffer_t* out) {
  return compress::DecodeXHex(data, out);
}

}  // namespace common
