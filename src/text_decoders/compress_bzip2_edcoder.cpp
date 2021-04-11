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

#include <common/text_decoders/compress_bzip2_edcoder.h>

#include <common/compress/bzip2_compress.h>

namespace common {

CompressBZip2EDcoder::CompressBZip2EDcoder(bool sized) : IEDcoder(ED_ZLIB), sized_(sized) {}

Error CompressBZip2EDcoder::DoEncode(const StringPiece& data, char_buffer_t* out) {
#if defined(HAVE_BZIP2)
  return compress::EncodeBZip2(data, sized_, out);
#else
  UNUSED(data);
  UNUSED(out);
  return make_error("ED_BZIP2 encode not supported");
#endif
}

Error CompressBZip2EDcoder::DoDecode(const StringPiece& data, char_buffer_t* out) {
#if defined(HAVE_BZIP2)
  return compress::DecodeBZip2(data, sized_, out);
#else
  UNUSED(data);
  UNUSED(out);
  return make_error("ED_BZIP2 decode not supported");
#endif
}

Error CompressBZip2EDcoder::DoEncode(const char_buffer_t& data, char_buffer_t* out) {
#if defined(HAVE_BZIP2)
  return compress::EncodeBZip2(data, sized_, out);
#else
  UNUSED(data);
  UNUSED(out);
  return make_error("ED_BZIP2 encode not supported");
#endif
}

Error CompressBZip2EDcoder::DoDecode(const char_buffer_t& data, char_buffer_t* out) {
#if defined(HAVE_BZIP2)
  return compress::DecodeBZip2(data, sized_, out);
#else
  UNUSED(data);
  UNUSED(out);
  return make_error("ED_BZIP2 decode not supported");
#endif
}

}  // namespace common
