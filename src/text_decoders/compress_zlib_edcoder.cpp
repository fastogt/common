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

#include <common/text_decoders/compress_zlib_edcoder.h>

#include <common/compress/zlib_compress.h>

namespace common {

CompressZlibEDcoder::CompressZlibEDcoder(bool sized, ZlibDeflates def)
    : IEDcoder(ED_ZLIB), sized_(sized), deflate_(def) {}

Error CompressZlibEDcoder::DoEncode(const StringPiece& data, char_buffer_t* out) {
#if defined(HAVE_ZLIB)
  return compress::EncodeZlib(data, sized_, deflate_, out);
#else
  UNUSED(data);
  UNUSED(out);
  return make_error("ED_ZLIB encode not supported");
#endif
}

Error CompressZlibEDcoder::DoDecode(const StringPiece& data, char_buffer_t* out) {
#if defined(HAVE_ZLIB)
  return compress::DecodeZlib(data, sized_, out);
#else
  UNUSED(data);
  UNUSED(out);
  return make_error("ED_ZLIB decode not supported");
#endif
}

Error CompressZlibEDcoder::DoEncode(const char_buffer_t& data, char_buffer_t* out) {
#if defined(HAVE_ZLIB)
  return compress::EncodeZlib(data, sized_, deflate_, out);
#else
  UNUSED(data);
  UNUSED(out);
  return make_error("ED_ZLIB encode not supported");
#endif
}

Error CompressZlibEDcoder::DoDecode(const char_buffer_t& data, char_buffer_t* out) {
#if defined(HAVE_ZLIB)
  return compress::DecodeZlib(data, sized_, out);
#else
  UNUSED(data);
  UNUSED(out);
  return make_error("ED_ZLIB decode not supported");
#endif
}

}  // namespace common
