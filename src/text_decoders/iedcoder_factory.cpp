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

#include <common/text_decoders/iedcoder_factory.h>

#include <common/text_decoders/base64_edcoder.h>
#include <common/text_decoders/compress_bzip2_edcoder.h>
#include <common/text_decoders/compress_lz4_edcoder.h>
#include <common/text_decoders/compress_snappy_edcoder.h>
#include <common/text_decoders/compress_zlib_edcoder.h>
#include <common/text_decoders/hex_edcoder.h>
#include <common/text_decoders/html_edcoder.h>
#include <common/text_decoders/none_edcoder.h>
#include <common/text_decoders/unicode_edcoder.h>
#include <common/text_decoders/uunicode_edcoder.h>
#include <common/text_decoders/xhex_edcoder.h>

namespace common {

IEDcoder* CreateEDCoder(EDType type) {
  if (type == ED_NONE) {
    return new NoneEDcoder;
  } else if (type == ED_BASE64) {
    return new Base64EDcoder;
  } else if (type == ED_ZLIB) {
    return new CompressZlibEDcoder;
  } else if (type == ED_BZIP2) {
    return new CompressBZip2EDcoder;
  } else if (type == ED_LZ4) {
    return new CompressLZ4EDcoder;
  } else if (type == ED_SNAPPY) {
    return new CompressSnappyEDcoder;
  } else if (type == ED_HEX) {
    return new HexEDcoder;
  } else if (type == ED_XHEX) {
    return new XHexEDcoder;
  } else if (type == ED_UNICODE) {
    return new UnicodeEDcoder;
  } else if (type == ED_UUNICODE) {
    return new UUnicodeEDcoder;
  } else if (type == ED_HTML_ESC) {
    return new HtmlEscEDcoder;
  }

  DNOTREACHED() << "Unknown EDCoder type:" << type;
  return nullptr;
}

IEDcoder* CreateEDCoder(const std::string& name) {
  EDType t;
  if (!ConvertFromString(name, &t)) {
    DNOTREACHED() << "Unknown EDCoder name:" << name;
    return nullptr;
  }

  return CreateEDCoder(t);
}

}  // namespace common
