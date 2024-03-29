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

#pragma once

#include <stddef.h>  // for size_t
#include <stdint.h>  // for uint8_t, uint32_t

#define SHA1_HASH_LENGTH 20
#define BLOCK_LENGTH 64

namespace common {
namespace hash {

typedef struct SHA1_CTX {
  uint32_t buffer[BLOCK_LENGTH / 4];
  uint32_t state[SHA1_HASH_LENGTH / 4];
  uint32_t byte_count;
  uint8_t buffer_offset;
  uint8_t key_buffer[BLOCK_LENGTH];
  uint8_t inner_hash[SHA1_HASH_LENGTH];
} SHA1_CTX;

void SHA1_Init(SHA1_CTX* s);

void SHA1_Update(SHA1_CTX* s, const unsigned char* data, size_t len);

void SHA1_Final(SHA1_CTX* s, unsigned char* result);

}  // namespace hash
}  // namespace common
