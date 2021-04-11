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

#include <stdint.h>     // for uint8_t, uint32_t
#include <sys/types.h>  // for ssize_t

#include <common/types.h>  // for buffer_t

namespace common {
namespace http2 {

enum http2_huff_decode_flag { HTTP2_HUFF_ACCEPTED = 1, HTTP2_HUFF_SYM = (1 << 1), HTTP2_HUFF_FAIL = (1 << 2) };

struct http2_huff_decode {
  uint8_t state;
  uint8_t flags;
  uint8_t sym;
};

typedef http2_huff_decode huff_decode_table_type[16];

struct http2_hd_huff_decode_context {
  uint8_t state;
  uint8_t accept;
};

struct http2_huff_sym {
  uint32_t nbits;
  uint32_t code;
};

void http2_huffman_decode_context_init(http2_hd_huff_decode_context* ctx);

uint32_t http2_huffman_encode_count(const uint8_t* src, uint32_t len);
ssize_t http2_huffman_encode(buffer_t& bufs, const uint8_t* src, uint32_t srclen);
ssize_t http2_huffman_decode(http2_hd_huff_decode_context* ctx,
                             buffer_t& bufs,
                             const uint8_t* src,
                             uint32_t srclen,
                             int final);

}  // namespace http2
}  // namespace common
