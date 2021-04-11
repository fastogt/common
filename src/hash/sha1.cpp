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

#include <common/hash/sha1.h>

#include <stdint.h>
#include <string.h>

#include <common/portable_endian.h>

/* code */
#define SHA1_K0 0x5a827999
#define SHA1_K20 0x6ed9eba1
#define SHA1_K40 0x8f1bbcdc
#define SHA1_K60 0xca62c1d6

namespace common {
namespace hash {
namespace {
uint32_t sha1_rol32(uint32_t number, uint8_t bits) {
  return ((number << bits) | (number >> (32 - bits)));
}

void sha1_hashBlock(SHA1_CTX* s) {
  uint8_t i;
  uint32_t a, b, c, d, e, t;

  a = s->state[0];
  b = s->state[1];
  c = s->state[2];
  d = s->state[3];
  e = s->state[4];
  for (i = 0; i < 80; i++) {
    if (i >= 16) {
      t = s->buffer[(i + 13) & 15] ^ s->buffer[(i + 8) & 15] ^ s->buffer[(i + 2) & 15] ^ s->buffer[i & 15];
      s->buffer[i & 15] = sha1_rol32(t, 1);
    }
    if (i < 20) {
      t = (d ^ (b & (c ^ d))) + SHA1_K0;
    } else if (i < 40) {
      t = (b ^ c ^ d) + SHA1_K20;
    } else if (i < 60) {
      t = ((b & c) | (d & (b | c))) + SHA1_K40;
    } else {
      t = (b ^ c ^ d) + SHA1_K60;
    }
    t += sha1_rol32(a, 5) + e + s->buffer[i & 15];
    e = d;
    d = c;
    c = sha1_rol32(b, 30);
    b = a;
    a = t;
  }
  s->state[0] += a;
  s->state[1] += b;
  s->state[2] += c;
  s->state[3] += d;
  s->state[4] += e;
}

void sha1_addUncounted(SHA1_CTX* s, uint8_t data) {
  uint8_t* const b = reinterpret_cast<uint8_t*>(s->buffer);
#if defined(ARCH_CPU_BIG_ENDIAN)
  b[s->buffer_offset] = data;
#else
  b[s->buffer_offset ^ 3] = data;
#endif
  s->buffer_offset++;
  if (s->buffer_offset == BLOCK_LENGTH) {
    sha1_hashBlock(s);
    s->buffer_offset = 0;
  }
}

void sha1_updatebyte(SHA1_CTX* s, uint8_t data) {
  ++s->byte_count;
  sha1_addUncounted(s, data);
}

void sha1_pad(SHA1_CTX* s) {
  // Implement SHA-1 padding (fips180-2 Â§5.1.1)

  // Pad with 0x80 followed by 0x00 until the end of the block
  sha1_addUncounted(s, 0x80);
  while (s->buffer_offset != 56) {
    sha1_addUncounted(s, 0x00);
  }

  // Append length in the last 8 bytes
  sha1_addUncounted(s, 0);                    // We're only using 32 bit lengths
  sha1_addUncounted(s, 0);                    // But SHA-1 supports 64 bit lengths
  sha1_addUncounted(s, 0);                    // So zero pad the top bits
  sha1_addUncounted(s, s->byte_count >> 29);  // Shifting to multiply by 8
  sha1_addUncounted(s, s->byte_count >> 21);  // as SHA-1 supports bitstreams as well as
  sha1_addUncounted(s, s->byte_count >> 13);  // byte.
  sha1_addUncounted(s, s->byte_count >> 5);
  sha1_addUncounted(s, s->byte_count << 3);
}

}  // namespace

void SHA1_Init(SHA1_CTX* s) {
  s->state[0] = 0x67452301;
  s->state[1] = 0xefcdab89;
  s->state[2] = 0x98badcfe;
  s->state[3] = 0x10325476;
  s->state[4] = 0xc3d2e1f0;
  s->byte_count = 0;
  s->buffer_offset = 0;
}

void SHA1_Update(SHA1_CTX* s, const unsigned char* data, size_t len) {
  for (; len--;) {
    sha1_updatebyte(s, *data++);
  }
}

void SHA1_Final(SHA1_CTX* s, uint8_t* result) {
  // Pad to complete the last block
  sha1_pad(s);

#ifndef ARCH_CPU_BIG_ENDIAN
  // Swap byte order back
  int i;
  for (i = 0; i < 5; i++) {
    s->state[i] = (((s->state[i]) << 24) & 0xff000000) | (((s->state[i]) << 8) & 0x00ff0000) |
                  (((s->state[i]) >> 8) & 0x0000ff00) | (((s->state[i]) >> 24) & 0x000000ff);
  }
#endif

  // Return pointer to hash (20 characters)
  memcpy(result, reinterpret_cast<uint8_t*>(s->state), SHA1_HASH_LENGTH);
}

}  // namespace hash
}  // namespace common
