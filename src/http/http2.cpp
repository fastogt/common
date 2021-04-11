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

#include <common/http/http2.h>

#include <string.h>

#include <common/convert2string.h>  // for ConvertToString, etc
#include <common/portable_endian.h>

#define MAKE_STATIC_ENT(N, V, T, H) \
  { {MAKE_BUFFER(N), MAKE_BUFFER(V), 0}, nullptr, 0, (H), (T) }

#define lstreq(A, B, N) ((sizeof((A)) - 1) == (N) && memcmp((A), (B), (N)) == 0)

#define INDEX_RANGE_VALID(context, idx) ((idx) < (context)->hd_table.len + HTTP2_STATIC_TABLE_LENGTH)

namespace common {
namespace http2 {
namespace {

http2_entry static_table[] = {
    MAKE_STATIC_ENT(":authority", "", 0, 3153725150u),
    MAKE_STATIC_ENT(":method", "GET", 1, 695666056u),
    MAKE_STATIC_ENT(":method", "POST", 1, 695666056u),
    MAKE_STATIC_ENT(":path", "/", 3, 3292848686u),
    MAKE_STATIC_ENT(":path", "/index.html", 3, 3292848686u),
    MAKE_STATIC_ENT(":scheme", "http", 5, 2510477674u),
    MAKE_STATIC_ENT(":scheme", "https", 5, 2510477674u),
    MAKE_STATIC_ENT(":status", "200", 7, 4000288983u),
    MAKE_STATIC_ENT(":status", "204", 7, 4000288983u),
    MAKE_STATIC_ENT(":status", "206", 7, 4000288983u),
    MAKE_STATIC_ENT(":status", "304", 7, 4000288983u),
    MAKE_STATIC_ENT(":status", "400", 7, 4000288983u),
    MAKE_STATIC_ENT(":status", "404", 7, 4000288983u),
    MAKE_STATIC_ENT(":status", "500", 7, 4000288983u),
    MAKE_STATIC_ENT("accept-charset", "", 14, 3664010344u),
    MAKE_STATIC_ENT("accept-encoding", "gzip, deflate", 15, 3379649177u),
    MAKE_STATIC_ENT("accept-language", "", 16, 1979086614u),
    MAKE_STATIC_ENT("accept-ranges", "", 17, 1713753958u),
    MAKE_STATIC_ENT("accept", "", 18, 136609321u),
    MAKE_STATIC_ENT("access-control-allow-origin", "", 19, 2710797292u),
    MAKE_STATIC_ENT("age", "", 20, 742476188u),
    MAKE_STATIC_ENT("allow", "", 21, 2930878514u),
    MAKE_STATIC_ENT("authorization", "", 22, 2436257726u),
    MAKE_STATIC_ENT("cache-control", "", 23, 1355326669u),
    MAKE_STATIC_ENT("content-disposition", "", 24, 3889184348u),
    MAKE_STATIC_ENT("content-encoding", "", 25, 65203592u),
    MAKE_STATIC_ENT("content-language", "", 26, 24973587u),
    MAKE_STATIC_ENT("content-length", "", 27, 1308181789u),
    MAKE_STATIC_ENT("content-location", "", 28, 2302364718u),
    MAKE_STATIC_ENT("content-range", "", 29, 3555523146u),
    MAKE_STATIC_ENT("content-type", "", 30, 4244048277u),
    MAKE_STATIC_ENT("cookie", "", 31, 2007449791u),
    MAKE_STATIC_ENT("date", "", 32, 3564297305u),
    MAKE_STATIC_ENT("etag", "", 33, 113792960u),
    MAKE_STATIC_ENT("expect", "", 34, 2530896728u),
    MAKE_STATIC_ENT("expires", "", 35, 1049544579u),
    MAKE_STATIC_ENT("from", "", 36, 2513272949u),
    MAKE_STATIC_ENT("host", "", 37, 2952701295u),
    MAKE_STATIC_ENT("if-match", "", 38, 3597694698u),
    MAKE_STATIC_ENT("if-modified-since", "", 39, 2213050793u),
    MAKE_STATIC_ENT("if-none-match", "", 40, 2536202615u),
    MAKE_STATIC_ENT("if-range", "", 41, 2340978238u),
    MAKE_STATIC_ENT("if-unmodified-since", "", 42, 3794814858u),
    MAKE_STATIC_ENT("last-modified", "", 43, 3226950251u),
    MAKE_STATIC_ENT("link", "", 44, 232457833u),
    MAKE_STATIC_ENT("location", "", 45, 200649126u),
    MAKE_STATIC_ENT("max-forwards", "", 46, 1826162134u),
    MAKE_STATIC_ENT("proxy-authenticate", "", 47, 2709445359u),
    MAKE_STATIC_ENT("proxy-authorization", "", 48, 2686392507u),
    MAKE_STATIC_ENT("range", "", 49, 4208725202u),
    MAKE_STATIC_ENT("referer", "", 50, 3969579366u),
    MAKE_STATIC_ENT("refresh", "", 51, 3572655668u),
    MAKE_STATIC_ENT("retry-after", "", 52, 3336180598u),
    MAKE_STATIC_ENT("server", "", 53, 1085029842u),
    MAKE_STATIC_ENT("set-cookie", "", 54, 1848371000u),
    MAKE_STATIC_ENT("strict-transport-security", "", 55, 4138147361u),
    MAKE_STATIC_ENT("transfer-encoding", "", 56, 3719590988u),
    MAKE_STATIC_ENT("user-agent", "", 57, 606444526u),
    MAKE_STATIC_ENT("vary", "", 58, 1085005381u),
    MAKE_STATIC_ENT("via", "", 59, 1762798611u),
    MAKE_STATIC_ENT("www-authenticate", "", 60, 779865858u),
};

void http2_bufs_addb(buffer_t& bufs, uint8_t b) {
  bufs.push_back(b);
}

struct search_result {
  int32_t index;
  uint8_t name_value_match;
};

int http2_hd_entry_init(http2_entry* ent, const http2_nv* oent, int token) {
  /* Since nghttp2_hd_entry is used for indexing, ent->nv.flags always
  NGHTTP2_NV_FLAG_NONE */
  ent->nv.flags = HTTP2_NV_FLAG_NONE;

  ent->nv.name = oent->name;
  ent->nv.value = oent->value;

  ent->token = token;
  ent->next = nullptr;
  ent->hash = 0;

  return 0;
}

bool name_eq(const http2_nv* a, const http2_nv* b) {
  return a->name == b->name;
}

bool value_eq(const http2_nv* a, const http2_nv* b) {
  return a->value == b->value;
}

uint32_t name_hash(const http2_nv* nv) {
  // 32 bit FNV-1a: http://isthe.com/chongo/tech/comp/fnv/
  uint32_t h = 2166136261u;
  uint32_t i;

  for (i = 0; i < nv->name.size(); ++i) {
    h ^= nv->name[i];
    h += (h << 1) + (h << 4) + (h << 7) + (h << 8) + (h << 24);
  }

  return h;
}

void hd_map_insert(http2_map* map, http2_entry* ent) {
  http2_entry** bucket = &map->table[ent->hash & (MAP_SIZE - 1)];

  if (*bucket == nullptr) {
    *bucket = ent;
    return;
  }

  /* lower index is linked near the root */
  ent->next = *bucket;
  *bucket = ent;
}

http2_entry* hd_map_find(http2_map* map, int* exact_match, const http2_nv* nv, int token, uint32_t hash) {
  http2_entry* res = nullptr;
  *exact_match = 0;

  for (http2_entry* p = map->table[hash & (MAP_SIZE - 1)]; p; p = p->next) {
    if (hash != p->hash || token != p->token || (token == -1 && !name_eq(&p->nv, nv))) {
      continue;
    }
    if (!res) {
      res = p;
    }
    if (value_eq(&p->nv, nv)) {
      res = p;
      *exact_match = 1;
      break;
    }
  }

  return res;
}

void hd_map_remove(http2_map* map, http2_entry* ent) {
  http2_entry** bucket = &map->table[ent->hash & (MAP_SIZE - 1)];

  if (*bucket == nullptr) {
    return;
  }

  if (*bucket == ent) {
    *bucket = ent->next;
    ent->next = nullptr;
    return;
  }

  for (http2_entry* p = *bucket; p; p = p->next) {
    if (p->next == ent) {
      p->next = ent->next;
      ent->next = nullptr;
      return;
    }
  }
}

uint32_t entry_room(uint32_t namelen, uint32_t valuelen) {
  return HTTP2_ENTRY_OVERHEAD + namelen + valuelen;
}

http2_entry* hd_ringbuf_get(http2_ringbuf* ringbuf, uint32_t idx) {
  DCHECK(idx < ringbuf->len);
  return ringbuf->buffer[(ringbuf->first + idx) & ringbuf->mask];
}

int hd_ringbuf_reserve(http2_ringbuf* ringbuf, uint32_t bufsize) {
  uint32_t i;
  uint32_t size;
  http2_entry** buffer;

  if (ringbuf->mask + 1 >= bufsize) {
    return 0;
  }

  for (size = 1; size < bufsize; size <<= 1) {
  }

  buffer = static_cast<http2_entry**>(malloc(sizeof(http2_entry*) * size));
  if (buffer == nullptr) {
    return -1;
  }
  for (i = 0; i < ringbuf->len; ++i) {
    buffer[i] = hd_ringbuf_get(ringbuf, i);
  }

  ringbuf->buffer = buffer;
  ringbuf->mask = size - 1;
  ringbuf->first = 0;
  return 0;
}

int hd_ringbuf_push_front(http2_ringbuf* ringbuf, http2_entry* ent) {
  int rv = hd_ringbuf_reserve(ringbuf, ringbuf->len + 1);

  if (rv != 0) {
    return rv;
  }

  ringbuf->buffer[--ringbuf->first & ringbuf->mask] = ent;
  ++ringbuf->len;
  return 0;
}

void hd_ringbuf_pop_back(http2_ringbuf* ringbuf) {
  DCHECK_GT(ringbuf->len, 0);
  --ringbuf->len;
}

uint32_t count_encoded_length(uint32_t n, uint32_t prefix) {
  uint32_t k = static_cast<uint32_t>((1 << prefix) - 1);
  uint32_t len = 0;

  if (n < k) {
    return 1;
  }

  n -= k;
  ++len;

  for (; n >= 128; n >>= 7, ++len) {
  }

  return len + 1;
}

uint32_t encode_length(uint8_t* buf, uint32_t n, uint32_t prefix) {
  uint32_t k = static_cast<uint32_t>((1 << prefix) - 1);
  uint8_t* begin = buf;

  *buf = static_cast<uint8_t>(*buf & ~k);

  if (n < k) {
    *buf = static_cast<uint8_t>(*buf | n);
    return 1;
  }

  *buf = static_cast<uint8_t>(*buf | k);
  ++buf;

  n -= k;

  for (; n >= 128; n >>= 7) {
    *buf++ = static_cast<uint8_t>((1 << 7) | (n & 0x7f));
  }

  *buf++ = static_cast<uint8_t>(n);

  return static_cast<uint32_t>(buf - begin);
}

int emit_indexed_block(buffer_t& bufs, uint32_t idx) {
  uint8_t sb[16];
  uint8_t* bufp;

  uint32_t blocklen = count_encoded_length(idx + 1, 7);

  if (sizeof(sb) < blocklen) {
    return -1;
  }

  bufp = sb;
  *bufp = 0x80u;
  encode_length(bufp, idx + 1, 7);

  for (uint32_t i = 0; i < blocklen; ++i) {
    bufs.push_back(sb[i]);
  }
  return 0;
}

int emit_string(buffer_t& bufs, const buffer_t& src) {
  uint8_t sb[16];
  uint8_t* bufp;
  uint32_t blocklen;
  int huffman = 0;

  const uint8_t* str = src.data();
  uint32_t len = static_cast<uint32_t>(src.size());
  uint32_t enclen = http2_huffman_encode_count(str, len);

  if (enclen < len) {
    huffman = 1;
  } else {
    enclen = len;
  }

  blocklen = count_encoded_length(enclen, 7);

  if (sizeof(sb) < blocklen) {
    return -1;
  }

  bufp = sb;
  *bufp = huffman ? 1 << 7 : 0;
  encode_length(bufp, enclen, 7);

  for (uint32_t i = 0; i < blocklen; ++i) {
    bufs.push_back(sb[i]);
  }

  if (huffman) {
    return http2_huffman_encode(bufs, str, len);
  } else {
    DCHECK(enclen == len);
    for (uint32_t i = 0; i < len; ++i) {
      bufs.push_back(str[i]);
    }
  }

  return 0;
}

uint8_t pack_first_byte(int indexing_mode) {
  switch (indexing_mode) {
    case HTTP2_WITH_INDEXING:
      return 0x40u;
    case HTTP2_WITHOUT_INDEXING:
      return 0;
    case HTTP2_NEVER_INDEXING:
      return 0x10u;
    default:
      NOTREACHED();
  }
  /* This is required to compile with android NDK r10d +
  --enable-werror */
  return 0;
}

int emit_indname_block(buffer_t& bufs, uint32_t idx, const http2_nv* nv, int indexing_mode) {
  uint8_t* bufp;
  uint32_t blocklen;
  uint8_t sb[16];
  uint32_t prefixlen;

  if (indexing_mode == HTTP2_WITH_INDEXING) {
    prefixlen = 6;
  } else {
    prefixlen = 4;
  }

  blocklen = count_encoded_length(idx + 1, prefixlen);

  if (sizeof(sb) < blocklen) {
    return -1;
  }

  bufp = sb;

  *bufp = pack_first_byte(indexing_mode);

  encode_length(bufp, idx + 1, prefixlen);

  for (uint32_t i = 0; i < blocklen; ++i) {
    bufs.push_back(sb[i]);
  }

  return emit_string(bufs, nv->value);
}

int emit_newname_block(buffer_t& bufs, const http2_nv* nv, int indexing_mode) {
  http2_bufs_addb(bufs, pack_first_byte(indexing_mode));
  int rv = emit_string(bufs, nv->name);
  if (rv != 0) {
    return rv;
  }

  rv = emit_string(bufs, nv->value);
  if (rv != 0) {
    return rv;
  }

  return 0;
}

http2_entry* add_hd_table_incremental(http2_context* context,
                                      const http2_nv* nv,
                                      int token,
                                      http2_map* map,
                                      uint32_t hash) {
  uint32_t room = entry_room(nv->namelen(), nv->valuelen());

  while (context->hd_table_bufsize + room > context->hd_table_bufsize_max && context->hd_table.len > 0) {
    uint32_t idx = context->hd_table.len - 1;
    http2_entry* ent = hd_ringbuf_get(&context->hd_table, idx);

    context->hd_table_bufsize -= entry_room(ent->nv.namelen(), ent->nv.valuelen());

    hd_ringbuf_pop_back(&context->hd_table);
    if (map) {
      hd_map_remove(map, ent);
    }
  }

  http2_entry* new_ent = new http2_entry;
  int rv = http2_hd_entry_init(new_ent, nv, token);
  if (rv != 0) {
    return nullptr;
  }

  rv = hd_ringbuf_push_front(&context->hd_table, new_ent);

  if (rv != 0) {
    delete new_ent;
    return nullptr;
  }

  new_ent->seq = context->next_seq++;
  new_ent->hash = hash;

  if (map) {
    hd_map_insert(map, new_ent);
  }

  context->hd_table_bufsize += room;
  return new_ent;
}

int hd_inflate_remove_bufs(http2_inflater* inflater, http2_nv* nv, int value_only) {
  buffer_t buf = inflater->nvbufs;
  inflater->nvbufs.clear();

  if (value_only) {
    nv->name.clear();
    nv->value = buf;
  } else {
    nv->name = buf;
    nv->value = MAKE_BUFFER_SIZE(buf.data() + nv->namelen() + 1, buf.size() - nv->namelen());
  }

  return 0;
}

int hd_inflate_read(http2_inflater* inflater, buffer_t& bufs, uint8_t* in, uint8_t* last) {
  int len = std::min(static_cast<uint32_t>(last - in), inflater->left);
  for (int i = 0; i < len; ++i) {
    bufs.push_back(in[i]);
  }
  inflater->left -= len;
  return len;
}

int hd_inflate_remove_bufs_with_name(http2_inflater* inflater, http2_nv* nv, http2_entry* ent_name) {
  UNUSED(inflater);
  UNUSED(nv);
  UNUSED(ent_name);
  NOTREACHED();
  return 0;
}

search_result search_static_table(const http2_nv* nv, int token, int indexing_mode) {
  search_result res = {token, 0};
  if (indexing_mode == HTTP2_NEVER_INDEXING) {
    return res;
  }

  for (int i = token; i <= HTTP2_TOKEN_WWW_AUTHENTICATE && static_table[i].token == token; ++i) {
    if (value_eq(&static_table[i].nv, nv)) {
      res.index = i;
      res.name_value_match = 1;
      return res;
    }
  }
  return res;
}

search_result search_hd_table(http2_context* context,
                              const http2_nv* nv,
                              int token,
                              int indexing_mode,
                              http2_map* map,
                              uint32_t hash) {
  search_result res = {-1, 0};
  if (token >= 0 && token <= HTTP2_TOKEN_WWW_AUTHENTICATE) {
    res = search_static_table(nv, token, indexing_mode);
    if (res.name_value_match) {
      return res;
    }
  }

  int exact_match = 0;
  http2_entry* ent = hd_map_find(map, &exact_match, nv, token, hash);
  if (!ent) {
    return res;
  }

  if (res.index != -1 && !exact_match) {
    return res;
  }

  res.index = static_cast<uint32_t>(context->next_seq - 1 - ent->seq + HTTP2_STATIC_TABLE_LENGTH);

  if (exact_match) {
    res.name_value_match = 1;
  }

  return res;
}

void hd_context_shrink_table_size(http2_context* context, http2_map* map) {
  while (context->hd_table_bufsize > context->hd_table_bufsize_max && context->hd_table.len > 0) {
    uint32_t idx = context->hd_table.len - 1;
    http2_entry* ent = hd_ringbuf_get(&context->hd_table, idx);
    context->hd_table_bufsize -= entry_room(ent->nv.namelen(), ent->nv.valuelen());
    hd_ringbuf_pop_back(&context->hd_table);
    if (map) {
      hd_map_remove(map, ent);
    }
  }
}

int lookup_token(const uint8_t* name, uint32_t namelen) {
  switch (namelen) {
    case 2:
      switch (name[1]) {
        case 'e':
          if (lstreq("t", name, 1)) {
            return HTTP2_TOKEN_TE;
          }
          break;
      }
      break;
    case 3:
      switch (name[2]) {
        case 'a':
          if (lstreq("vi", name, 2)) {
            return HTTP2_TOKEN_VIA;
          }
          break;
        case 'e':
          if (lstreq("ag", name, 2)) {
            return HTTP2_TOKEN_AGE;
          }
          break;
      }
      break;
    case 4:
      switch (name[3]) {
        case 'e':
          if (lstreq("dat", name, 3)) {
            return HTTP2_TOKEN_DATE;
          }
          break;
        case 'g':
          if (lstreq("eta", name, 3)) {
            return HTTP2_TOKEN_ETAG;
          }
          break;
        case 'k':
          if (lstreq("lin", name, 3)) {
            return HTTP2_TOKEN_LINK;
          }
          break;
        case 'm':
          if (lstreq("fro", name, 3)) {
            return HTTP2_TOKEN_FROM;
          }
          break;
        case 't':
          if (lstreq("hos", name, 3)) {
            return HTTP2_TOKEN_HOST;
          }
          break;
        case 'y':
          if (lstreq("var", name, 3)) {
            return HTTP2_TOKEN_VARY;
          }
          break;
      }
      break;
    case 5:
      switch (name[4]) {
        case 'e':
          if (lstreq("rang", name, 4)) {
            return HTTP2_TOKEN_RANGE;
          }
          break;
        case 'h':
          if (lstreq(":pat", name, 4)) {
            return HTTP2_TOKEN__PATH;
          }
          if (lstreq(":pat", name, 4)) {
            return HTTP2_TOKEN__PATH;
          }
          break;
        case 'w':
          if (lstreq("allo", name, 4)) {
            return HTTP2_TOKEN_ALLOW;
          }
          break;
      }
      break;
    case 6:
      switch (name[5]) {
        case 'e':
          if (lstreq("cooki", name, 5)) {
            return HTTP2_TOKEN_COOKIE;
          }
          break;
        case 'r':
          if (lstreq("serve", name, 5)) {
            return HTTP2_TOKEN_SERVER;
          }
          break;
        case 't':
          if (lstreq("accep", name, 5)) {
            return HTTP2_TOKEN_ACCEPT;
          }
          if (lstreq("expec", name, 5)) {
            return HTTP2_TOKEN_EXPECT;
          }
          break;
      }
      break;
    case 7:
      switch (name[6]) {
        case 'd':
          if (lstreq(":metho", name, 6)) {
            return HTTP2_TOKEN__METHOD;
          }
          if (lstreq(":metho", name, 6)) {
            return HTTP2_TOKEN__METHOD;
          }
          break;
        case 'e':
          if (lstreq(":schem", name, 6)) {
            return HTTP2_TOKEN__SCHEME;
          }
          if (lstreq(":schem", name, 6)) {
            return HTTP2_TOKEN__SCHEME;
          }
          if (lstreq("upgrad", name, 6)) {
            return HTTP2_TOKEN_UPGRADE;
          }
          break;
        case 'h':
          if (lstreq("refres", name, 6)) {
            return HTTP2_TOKEN_REFRESH;
          }
          break;
        case 'r':
          if (lstreq("refere", name, 6)) {
            return HTTP2_TOKEN_REFERER;
          }
          break;
        case 's':
          if (lstreq(":statu", name, 6)) {
            return HTTP2_TOKEN__STATUS;
          }
          if (lstreq(":statu", name, 6)) {
            return HTTP2_TOKEN__STATUS;
          }
          if (lstreq(":statu", name, 6)) {
            return HTTP2_TOKEN__STATUS;
          }
          if (lstreq(":statu", name, 6)) {
            return HTTP2_TOKEN__STATUS;
          }
          if (lstreq(":statu", name, 6)) {
            return HTTP2_TOKEN__STATUS;
          }
          if (lstreq(":statu", name, 6)) {
            return HTTP2_TOKEN__STATUS;
          }
          if (lstreq(":statu", name, 6)) {
            return HTTP2_TOKEN__STATUS;
          }
          if (lstreq("expire", name, 6)) {
            return HTTP2_TOKEN_EXPIRES;
          }
          break;
      }
      break;
    case 8:
      switch (name[7]) {
        case 'e':
          if (lstreq("if-rang", name, 7)) {
            return HTTP2_TOKEN_IF_RANGE;
          }
          break;
        case 'h':
          if (lstreq("if-matc", name, 7)) {
            return HTTP2_TOKEN_IF_MATCH;
          }
          break;
        case 'n':
          if (lstreq("locatio", name, 7)) {
            return HTTP2_TOKEN_LOCATION;
          }
          break;
      }
      break;
    case 10:
      switch (name[9]) {
        case 'e':
          if (lstreq("keep-aliv", name, 9)) {
            return HTTP2_TOKEN_KEEP_ALIVE;
          }
          if (lstreq("set-cooki", name, 9)) {
            return HTTP2_TOKEN_SET_COOKIE;
          }
          break;
        case 'n':
          if (lstreq("connectio", name, 9)) {
            return HTTP2_TOKEN_CONNECTION;
          }
          break;
        case 't':
          if (lstreq("user-agen", name, 9)) {
            return HTTP2_TOKEN_USER_AGENT;
          }
          break;
        case 'y':
          if (lstreq(":authorit", name, 9)) {
            return HTTP2_TOKEN__AUTHORITY;
          }
          break;
      }
      break;
    case 11:
      switch (name[10]) {
        case 'r':
          if (lstreq("retry-afte", name, 10)) {
            return HTTP2_TOKEN_RETRY_AFTER;
          }
          break;
      }
      break;
    case 12:
      switch (name[11]) {
        case 'e':
          if (lstreq("content-typ", name, 11)) {
            return HTTP2_TOKEN_CONTENT_TYPE;
          }
          break;
        case 's':
          if (lstreq("max-forward", name, 11)) {
            return HTTP2_TOKEN_MAX_FORWARDS;
          }
          break;
      }
      break;
    case 13:
      switch (name[12]) {
        case 'd':
          if (lstreq("last-modifie", name, 12)) {
            return HTTP2_TOKEN_LAST_MODIFIED;
          }
          break;
        case 'e':
          if (lstreq("content-rang", name, 12)) {
            return HTTP2_TOKEN_CONTENT_RANGE;
          }
          break;
        case 'h':
          if (lstreq("if-none-matc", name, 12)) {
            return HTTP2_TOKEN_IF_NONE_MATCH;
          }
          break;
        case 'l':
          if (lstreq("cache-contro", name, 12)) {
            return HTTP2_TOKEN_CACHE_CONTROL;
          }
          break;
        case 'n':
          if (lstreq("authorizatio", name, 12)) {
            return HTTP2_TOKEN_AUTHORIZATION;
          }
          break;
        case 's':
          if (lstreq("accept-range", name, 12)) {
            return HTTP2_TOKEN_ACCEPT_RANGES;
          }
          break;
      }
      break;
    case 14:
      switch (name[13]) {
        case 'h':
          if (lstreq("content-lengt", name, 13)) {
            return HTTP2_TOKEN_CONTENT_LENGTH;
          }
          break;
        case 't':
          if (lstreq("accept-charse", name, 13)) {
            return HTTP2_TOKEN_ACCEPT_CHARSET;
          }
          break;
      }
      break;
    case 15:
      switch (name[14]) {
        case 'e':
          if (lstreq("accept-languag", name, 14)) {
            return HTTP2_TOKEN_ACCEPT_LANGUAGE;
          }
          break;
        case 'g':
          if (lstreq("accept-encodin", name, 14)) {
            return HTTP2_TOKEN_ACCEPT_ENCODING;
          }
          break;
      }
      break;
    case 16:
      switch (name[15]) {
        case 'e':
          if (lstreq("content-languag", name, 15)) {
            return HTTP2_TOKEN_CONTENT_LANGUAGE;
          }
          if (lstreq("www-authenticat", name, 15)) {
            return HTTP2_TOKEN_WWW_AUTHENTICATE;
          }
          break;
        case 'g':
          if (lstreq("content-encodin", name, 15)) {
            return HTTP2_TOKEN_CONTENT_ENCODING;
          }
          break;
        case 'n':
          if (lstreq("content-locatio", name, 15)) {
            return HTTP2_TOKEN_CONTENT_LOCATION;
          }
          if (lstreq("proxy-connectio", name, 15)) {
            return HTTP2_TOKEN_PROXY_CONNECTION;
          }
          break;
      }
      break;
    case 17:
      switch (name[16]) {
        case 'e':
          if (lstreq("if-modified-sinc", name, 16)) {
            return HTTP2_TOKEN_IF_MODIFIED_SINCE;
          }
          break;
        case 'g':
          if (lstreq("transfer-encodin", name, 16)) {
            return HTTP2_TOKEN_TRANSFER_ENCODING;
          }
          break;
      }
      break;
    case 18:
      switch (name[17]) {
        case 'e':
          if (lstreq("proxy-authenticat", name, 17)) {
            return HTTP2_TOKEN_PROXY_AUTHENTICATE;
          }
          break;
      }
      break;
    case 19:
      switch (name[18]) {
        case 'e':
          if (lstreq("if-unmodified-sinc", name, 18)) {
            return HTTP2_TOKEN_IF_UNMODIFIED_SINCE;
          }
          break;
        case 'n':
          if (lstreq("content-dispositio", name, 18)) {
            return HTTP2_TOKEN_CONTENT_DISPOSITION;
          }
          if (lstreq("proxy-authorizatio", name, 18)) {
            return HTTP2_TOKEN_PROXY_AUTHORIZATION;
          }
          break;
      }
      break;
    case 25:
      switch (name[24]) {
        case 'y':
          if (lstreq("strict-transport-securit", name, 24)) {
            return HTTP2_TOKEN_STRICT_TRANSPORT_SECURITY;
          }
          break;
      }
      break;
    case 27:
      switch (name[26]) {
        case 'n':
          if (lstreq("access-control-allow-origi", name, 26)) {
            return HTTP2_TOKEN_ACCESS_CONTROL_ALLOW_ORIGIN;
          }
          break;
      }
      break;
  }
  return -1;
}

int emit_indexed_header(http2_nv* nv_out, int* token_out, http2_entry* ent) {
  *nv_out = ent->nv;
  *token_out = ent->token;
  return 0;
}

int emit_literal_header(http2_nv* nv_out, int* token_out, http2_nv* nv) {
  *nv_out = *nv;
  *token_out = lookup_token(nv->name.data(), nv->namelen());
  return 0;
}

http2_entry* http2_hd_table_get(http2_context* context, uint32_t idx) {
  DCHECK(INDEX_RANGE_VALID(context, idx));
  if (idx >= HTTP2_STATIC_TABLE_LENGTH) {
    return hd_ringbuf_get(&context->hd_table, idx - HTTP2_STATIC_TABLE_LENGTH);
  } else {
    return &static_table[idx];
  }
}

int hd_inflate_commit_indexed(http2_inflater* inflater, http2_nv* nv_out, int* token_out) {
  http2_entry* ent = http2_hd_table_get(&inflater->ctx, inflater->index);
  emit_indexed_header(nv_out, token_out, ent);
  return 0;
}

uint32_t get_max_index(http2_context* context) {
  return context->hd_table.len + HTTP2_STATIC_TABLE_LENGTH - 1;
}

int hd_deflate_decide_indexing(http2_deflater* deflater, const http2_nv* nv, int token) {
  if (token == HTTP2_TOKEN__PATH || token == HTTP2_TOKEN_AGE || token == HTTP2_TOKEN_CONTENT_LENGTH ||
      token == HTTP2_TOKEN_ETAG || token == HTTP2_TOKEN_IF_MODIFIED_SINCE || token == HTTP2_TOKEN_IF_NONE_MATCH ||
      token == HTTP2_TOKEN_LOCATION || token == HTTP2_TOKEN_SET_COOKIE ||
      entry_room(nv->namelen(), nv->valuelen()) > deflater->ctx.hd_table_bufsize_max * 3 / 4) {
    return HTTP2_WITHOUT_INDEXING;
  }

  return HTTP2_WITH_INDEXING;
}

int deflate_nv(http2_deflater* deflater, buffer_t& bufs, const http2_nv* nv) {
  int rv;
  search_result res;
  int32_t idx;
  int indexing_mode;
  int token;
  uint32_t hash;

  token = lookup_token(nv->name.data(), nv->namelen());
  if (token == -1 || token > HTTP2_TOKEN_WWW_AUTHENTICATE) {
    hash = name_hash(nv);
  } else {
    hash = static_table[token].hash;
  }

  /* Don't index authorization header field since it may contain low
  entropy secret data (e.g., id/password).  Also cookie header
  field with less than 20 bytes value is also never indexed.  This
  is the same criteria used in Firefox codebase. */
  indexing_mode = token == HTTP2_TOKEN_AUTHORIZATION || (token == HTTP2_TOKEN_COOKIE && nv->valuelen() < 20) ||
                          (nv->flags & HTTP2_NV_FLAG_NO_INDEX)
                      ? HTTP2_NEVER_INDEXING
                      : hd_deflate_decide_indexing(deflater, nv, token);

  res = search_hd_table(&deflater->ctx, nv, token, indexing_mode, &deflater->map, hash);

  idx = res.index;

  if (res.name_value_match) {
    rv = emit_indexed_block(bufs, static_cast<uint32_t>(idx));
    if (rv != 0) {
      return rv;
    }
    return 0;
  }

  if (indexing_mode == HTTP2_WITH_INDEXING) {
    http2_entry* new_ent = nullptr;
    if (idx != -1 && idx < HTTP2_STATIC_TABLE_LENGTH) {
      http2_nv nv_indname;
      nv_indname = *nv;
      nv_indname.name = http2_hd_table_get(&deflater->ctx, static_cast<uint32_t>(idx))->nv.name;
      new_ent = add_hd_table_incremental(&deflater->ctx, &nv_indname, token, &deflater->map, hash);
    } else {
      new_ent = add_hd_table_incremental(&deflater->ctx, nv, token, &deflater->map, hash);
    }

    if (!new_ent) {
      return -1;  //
    }
  }

  if (idx == -1) {
    rv = emit_newname_block(bufs, nv, indexing_mode);
  } else {
    rv = emit_indname_block(bufs, static_cast<uint32_t>(idx), nv, indexing_mode);
  }

  if (rv != 0) {
    return rv;
  }

  return 0;
}

ssize_t decode_length(uint32_t* res,
                      uint32_t* shift_ptr,
                      int* final,
                      uint32_t initial,
                      uint32_t shift,
                      uint8_t* in,
                      uint8_t* last,
                      uint32_t prefix) {
  uint32_t k = static_cast<uint8_t>((1 << prefix) - 1);
  uint32_t n = initial;
  uint8_t* start = in;

  *shift_ptr = 0;
  *final = 0;

  if (n == 0) {
    if ((*in & k) != k) {
      *res = (*in) & k;
      *final = 1;
      return 1;
    }

    n = k;

    if (++in == last) {
      *res = n;
      return static_cast<ssize_t>(in - start);
    }
  }

  for (; in != last; ++in, shift += 7) {
    uint32_t add = *in & 0x7f;

    if ((UINT32_MAX >> shift) < add) {
      return -1;
    }

    add <<= shift;

    if (UINT32_MAX - add < n) {
      return -1;
    }

    n += add;

    if ((*in & (1 << 7)) == 0) {
      break;
    }
  }

  *shift_ptr = shift;

  if (in == last) {
    *res = n;
    return static_cast<ssize_t>(in - start);
  }

  *res = n;
  *final = 1;
  return static_cast<ssize_t>(in + 1 - start);
}

ssize_t hd_inflate_read_len(http2_inflater* inflater,
                            int* rfin,
                            uint8_t* in,
                            uint8_t* last,
                            uint32_t prefix,
                            uint32_t maxlen) {
  uint32_t out;
  *rfin = 0;
  ssize_t rv = decode_length(&out, &inflater->shift, rfin, inflater->left, inflater->shift, in, last, prefix);

  if (rv == -1) {
    return -1;
  }

  if (out > maxlen) {
    return -1;
  }

  inflater->left = out;
  return rv;
}

int hd_inflate_commit_newname(http2_inflater* inflater, http2_nv* nv_out, int* token_out) {
  http2_nv nv;

  int rv = hd_inflate_remove_bufs(inflater, &nv, 0);
  if (rv != 0) {
    return -1;
  }

  if (inflater->no_index) {
    nv.flags = HTTP2_NV_FLAG_NO_INDEX;
  } else {
    nv.flags = HTTP2_NV_FLAG_NONE;
  }

  if (inflater->index_required) {
    http2_entry* new_ent =
        add_hd_table_incremental(&inflater->ctx, &nv, lookup_token(nv.name.data(), nv.namelen()), nullptr, 0);

    if (new_ent) {
      emit_indexed_header(nv_out, token_out, new_ent);
      inflater->ent_keep = new_ent;

      return 0;
    }

    NOTREACHED();
    // nghttp2_mem_free(mem, nv.name);

    return -1;
  }

  emit_literal_header(nv_out, token_out, &nv);

  NOTREACHED();
  /*
  if (nv.name != inflater->nvbufs.head->buf.pos) {
  inflater->nv_keep = nv.name;
  }*/

  return 0;
}

int hd_inflate_commit_indname(http2_inflater* inflater, http2_nv* nv_out, int* token_out) {
  int rv;
  http2_nv nv;

  if (inflater->no_index) {
    nv.flags = HTTP2_NV_FLAG_NO_INDEX;
  } else {
    nv.flags = HTTP2_NV_FLAG_NONE;
  }

  http2_entry* ent_name = http2_hd_table_get(&inflater->ctx, inflater->index);

  if (inflater->index_required) {
    if (inflater->index < HTTP2_STATIC_TABLE_LENGTH) {
      /* We don't copy name in static table */
      rv = hd_inflate_remove_bufs(inflater, &nv, 1 /* value only */);
      if (rv != 0) {
        return -1;
      }
      nv.name = ent_name->nv.name;
    } else {
      rv = hd_inflate_remove_bufs_with_name(inflater, &nv, ent_name);
      if (rv != 0) {
        return -1;
      }
    }

    http2_entry* new_ent = add_hd_table_incremental(&inflater->ctx, &nv, ent_name->token, nullptr, 0);

    /* At this point, ent_name might be deleted. */

    if (new_ent) {
      emit_indexed_header(nv_out, token_out, new_ent);
      delete inflater->ent_keep;
      inflater->ent_keep = new_ent;
      return 0;
    }

    NOTREACHED();
    if (inflater->index < HTTP2_STATIC_TABLE_LENGTH) {
      // nghttp2_mem_free(mem, nv.value);
    } else {
      // nghttp2_mem_free(mem, nv.name);
    }

    return -1;
  }

  rv = hd_inflate_remove_bufs(inflater, &nv, 1 /* value only */);
  if (rv != 0) {
    return -1;
  }

  nv.name = ent_name->nv.name;

  emit_literal_header(nv_out, token_out, &nv);

  NOTREACHED();
  /*
  if (nv.value != inflater->nvbufs.head->buf.pos) {
  inflater->nv_keep = nv.value;
  }*/

  return 0;
}

void hd_inflate_set_huffman_encoded(http2_inflater* inflater, const uint8_t* in) {
  inflater->huffman_encoded = (*in & (1 << 7)) != 0;
}

ssize_t hd_inflate_read_huff(http2_inflater* inflater, buffer_t& bufs, uint8_t* in, uint8_t* last) {
  int final = 0;
  uint32_t diff = last - in;
  if (diff >= inflater->left) {
    last = in + inflater->left;
    final = 1;
  }

  ssize_t readlen = http2_huffman_decode(&inflater->huff_decode_ctx, bufs, in, static_cast<uint32_t>(last - in), final);

  if (readlen < 0) {
    return readlen;
  }

  inflater->left -= static_cast<uint32_t>(readlen);
  return readlen;
}

}  // namespace

http2_deflater::http2_deflater()
    : ctx(), map(), deflate_hd_table_bufsize_max(0), min_hd_table_bufsize_max(0), notify_table_size_change(0) {
  memset(&map, 0, sizeof(http2_map));
  if (deflate_hd_table_bufsize_max < HTTP2_DEFAULT_MAX_BUFFER_SIZE) {
    notify_table_size_change = 1;
    ctx.hd_table_bufsize_max = deflate_hd_table_bufsize_max;
  } else {
    notify_table_size_change = 0;
  }

  min_hd_table_bufsize_max = UINT32_MAX;
}

int http2_deflater::http2_deflate_hd_bufs(buffer_t& bufs, const http2_nvs_t& nv) {
  if (ctx.bad) {
    return -1;
  }

  int rv = -1;
  for (uint32_t i = 0; i < nv.size(); ++i) {
    rv = deflate_nv(this, bufs, &nv[i]);
    if (rv != 0) {
      goto fail;
    }
  }

  return 0;

fail:
  ctx.bad = 1;
  return rv;
}

uint32_t http2_nv::namelen() const {
  return static_cast<uint32_t>(name.size());
}

uint32_t http2_nv::valuelen() const {
  return static_cast<uint32_t>(value.size());
}

http2_ringbuf::http2_ringbuf(uint32_t bufsize) : buffer(nullptr), mask(0), first(0), len(0) {
  if (bufsize == 0) {
    DNOTREACHED();
    return;
  }

  uint32_t size = 0;
  for (size = 1; size < bufsize; size <<= 1) {
  }

  buffer = static_cast<http2_entry**>(malloc(sizeof(http2_entry*) * size));
  if (!buffer) {
    return;
  }

  mask = size - 1;
  first = 0;
  len = 0;
}

http2_ringbuf::~http2_ringbuf() {
  if (buffer) {
    free(buffer);
    buffer = nullptr;
  }
}

http2_context::http2_context()
    : hd_table_bufsize_max(HTTP2_DEFAULT_MAX_BUFFER_SIZE),
      hd_table(hd_table_bufsize_max / HTTP2_ENTRY_OVERHEAD),
      hd_table_bufsize(0),
      next_seq(0),
      bad(0) {}

http2_context::~http2_context() {}

http2_inflater::http2_inflater()
    : ctx(),
      ent_keep(nullptr),
      nv_keep(nullptr),
      settings_hd_table_bufsize_max(HTTP2_DEFAULT_MAX_BUFFER_SIZE),
      opcode(HTTP2_OPCODE_NONE),
      state(HTTP2_STATE_INFLATE_START),
      huff_decode_ctx(),
      huffman_encoded(0),
      index_required(0),
      no_index(0) {
  huffman_encoded = 0;
  index = 0;
  left = 0;
  shift = 0;
  newnamelen = 0;
  index_required = 0;
  no_index = 0;
}

http2_inflater::~http2_inflater() {
  delete ent_keep;
  ent_keep = nullptr;
}

ssize_t http2_inflater::http2_inflate_hd(http2_nv* nv_out,
                                         int* inflate_flags,
                                         uint8_t* in,
                                         uint32_t inlen,
                                         int in_final) {
  int token;

  return http2_inflate_hd2(nv_out, inflate_flags, &token, in, inlen, in_final);
}

ssize_t http2_inflater::http2_inflate_hd2(http2_nv* nv_out,
                                          int* inflate_flags,
                                          int* token_out,
                                          uint8_t* in,
                                          uint32_t inlen,
                                          int in_final) {
  ssize_t rv = 0;
  uint8_t* first = in;
  uint8_t* last = in + inlen;
  int rfin = 0;
  int busy = 0;

  if (ctx.bad) {
    return -1;
  }

  if (ent_keep) {
    delete ent_keep;
    ent_keep = nullptr;
  }

  nv_keep = nullptr;
  *token_out = -1;
  *inflate_flags = HTTP2_INFLATE_NONE;

  for (; in != last || busy;) {
    busy = 0;
    switch (state) {
      case HTTP2_STATE_EXPECT_TABLE_SIZE:
        if ((*in & 0xe0u) != 0x20u) {
          rv = -1;
          goto fail;
        }
      /* fall through */
      case HTTP2_STATE_INFLATE_START:
      case HTTP2_STATE_OPCODE:
        if ((*in & 0xe0u) == 0x20u) {
          if (state == HTTP2_STATE_OPCODE) {
            rv = -1;
            goto fail;
          }
          opcode = HTTP2_OPCODE_INDEXED;
          state = HTTP2_STATE_READ_TABLE_SIZE;
        } else if (*in & 0x80u) {
          opcode = HTTP2_OPCODE_INDEXED;
          state = HTTP2_STATE_READ_INDEX;
        } else {
          if (*in == 0x40u || *in == 0 || *in == 0x10u) {
            opcode = HTTP2_OPCODE_NEWNAME;
            state = HTTP2_STATE_NEWNAME_CHECK_NAMELEN;
          } else {
            opcode = HTTP2_OPCODE_INDNAME;
            state = HTTP2_STATE_READ_INDEX;
          }
          index_required = (*in & 0x40) != 0;
          no_index = (*in & 0xf0u) == 0x10u;
          if (opcode == HTTP2_OPCODE_NEWNAME) {
            ++in;
          }
        }
        left = 0;
        shift = 0;
        break;
      case HTTP2_STATE_READ_TABLE_SIZE:
        rfin = 0;
        rv = hd_inflate_read_len(this, &rfin, in, last, 5, settings_hd_table_bufsize_max);
        if (rv < 0) {
          goto fail;
        }
        in += rv;
        if (!rfin) {
          goto almost_ok;
        }
        ctx.hd_table_bufsize_max = left;
        hd_context_shrink_table_size(&ctx, nullptr);
        state = HTTP2_STATE_INFLATE_START;
        break;
      case HTTP2_STATE_READ_INDEX: {
        uint32_t prefixlen;

        if (opcode == HTTP2_OPCODE_INDEXED) {
          prefixlen = 7;
        } else if (index_required) {
          prefixlen = 6;
        } else {
          prefixlen = 4;
        }

        rfin = 0;
        rv = hd_inflate_read_len(this, &rfin, in, last, prefixlen, get_max_index(&ctx) + 1);
        if (rv < 0) {
          goto fail;
        }

        in += rv;

        if (!rfin) {
          goto almost_ok;
        }

        if (left == 0) {
          rv = -1;
          goto fail;
        }

        if (opcode == HTTP2_OPCODE_INDEXED) {
          index = left;
          --index;

          rv = hd_inflate_commit_indexed(this, nv_out, token_out);
          if (rv < 0) {
            goto fail;
          }

          state = HTTP2_STATE_OPCODE;
          /* If rv == 1, no header was emitted */
          if (rv == 0) {
            *inflate_flags |= HTTP2_INFLATE_EMIT;
            return in - first;
          }
        } else {
          index = left;
          --index;
          state = HTTP2_STATE_CHECK_VALUELEN;
        }
        break;
      }
      case HTTP2_STATE_NEWNAME_CHECK_NAMELEN:
        hd_inflate_set_huffman_encoded(this, in);
        state = HTTP2_STATE_NEWNAME_READ_NAMELEN;
        left = 0;
        shift = 0;
      /* Fall through */
      case HTTP2_STATE_NEWNAME_READ_NAMELEN:
        rfin = 0;
        rv = hd_inflate_read_len(this, &rfin, in, last, 7, HTTP2_MAX_NV);
        if (rv < 0) {
          goto fail;
        }
        in += rv;
        if (!rfin) {
          goto almost_ok;
        }

        if (huffman_encoded) {
          NOTREACHED();
          http2_huffman_decode_context_init(&huff_decode_ctx);
          state = HTTP2_STATE_NEWNAME_READ_NAMEHUFF;
        } else {
          state = HTTP2_STATE_NEWNAME_READ_NAME;
        }
        break;
      case HTTP2_STATE_NEWNAME_READ_NAMEHUFF:
        rv = hd_inflate_read_huff(this, nvbufs, in, last);
        if (rv < 0) {
          goto fail;
        }

        in += rv;

        if (left) {
          goto almost_ok;
        }

        newnamelen = nvbufs.size();
        state = HTTP2_STATE_CHECK_VALUELEN;
        break;
      case HTTP2_STATE_NEWNAME_READ_NAME:
        rv = hd_inflate_read(this, nvbufs, in, last);
        if (rv < 0) {
          goto fail;
        }

        in += rv;
        if (left) {
          goto almost_ok;
        }

        newnamelen = nvbufs.size();
        state = HTTP2_STATE_CHECK_VALUELEN;
        break;
      case HTTP2_STATE_CHECK_VALUELEN:
        hd_inflate_set_huffman_encoded(this, in);
        state = HTTP2_STATE_READ_VALUELEN;
        left = 0;
        shift = 0;
      /* Fall through */
      case HTTP2_STATE_READ_VALUELEN:
        rfin = 0;
        rv = hd_inflate_read_len(this, &rfin, in, last, 7, HTTP2_MAX_NV);
        if (rv < 0) {
          goto fail;
        }

        in += rv;

        if (!rfin) {
          goto almost_ok;
        }

        if (huffman_encoded) {
          http2_huffman_decode_context_init(&huff_decode_ctx);
          state = HTTP2_STATE_READ_VALUEHUFF;
        } else {
          state = HTTP2_STATE_READ_VALUE;
        }

        busy = 1;

        break;
      case HTTP2_STATE_READ_VALUEHUFF:
        rv = hd_inflate_read_huff(this, nvbufs, in, last);
        if (rv < 0) {
          goto fail;
        }

        in += rv;

        if (left) {
          goto almost_ok;
        }

        if (opcode == HTTP2_OPCODE_NEWNAME) {
          rv = hd_inflate_commit_newname(this, nv_out, token_out);
        } else {
          rv = hd_inflate_commit_indname(this, nv_out, token_out);
        }

        if (rv != 0) {
          goto fail;
        }

        state = HTTP2_STATE_OPCODE;
        *inflate_flags |= HTTP2_INFLATE_EMIT;

        return in - first;
      case HTTP2_STATE_READ_VALUE:
        rv = hd_inflate_read(this, nvbufs, in, last);
        if (rv < 0) {
          goto fail;
        }

        in += rv;

        if (left) {
          goto almost_ok;
        }

        if (opcode == HTTP2_OPCODE_NEWNAME) {
          rv = hd_inflate_commit_newname(this, nv_out, token_out);
        } else {
          rv = hd_inflate_commit_indname(this, nv_out, token_out);
        }

        if (rv != 0) {
          goto fail;
        }

        state = HTTP2_STATE_OPCODE;
        *inflate_flags |= HTTP2_INFLATE_EMIT;

        return in - first;
    }
  }

  DCHECK(in == last);

  if (in_final) {
    if (state != HTTP2_STATE_OPCODE) {
      rv = -1;
      goto fail;
    }
    *inflate_flags |= HTTP2_INFLATE_FINAL;
  }
  return in - first;

almost_ok:
  if (in_final && state != HTTP2_STATE_OPCODE) {
    rv = -1;
    goto fail;
  }
  return in - first;

fail:
  ctx.bad = 1;
  return rv;
}

sid::sid() : id_() {
  SIZEOF_DATA_MUST_BE(sid, 4);
}

sid::sid(uint32_t stream_id) {
  id_[0] = stream_id >> 31 & 0xff;
  id_[1] = stream_id >> 16 & 0xff;
  id_[2] = stream_id >> 8 & 0xff;
  id_[3] = stream_id & 0xff;
}

uint32_t sid::id() const {
  return static_cast<uint32_t>(id_[0] << 31 | id_[1] << 16 | id_[2] << 8 | id_[3]);
}

frame_hdr::frame_hdr() : length_(), type_(UINT8_MAX), flags_(), stream_id_() {
  SIZEOF_DATA_MUST_BE(frame_hdr, FRAME_HEADER_SIZE);
}

frame_hdr::frame_hdr(frame_t type, uint8_t flags, uint32_t stream_id, uint32_t length)
    : length_(), type_(type), flags_(flags), stream_id_(stream_id) {
  DCHECK(stream_id == frame_hdr::stream_id());

  length_[0] = length >> 16 & 0xff;
  length_[1] = length >> 8 & 0xff;
  length_[2] = length & 0xff;
  DCHECK(length == frame_hdr::length());
}

bool frame_hdr::IsValid() const {
  if (type_ == HTTP2_DATA) {
    return true;
  } else if (type_ == HTTP2_HEADERS) {
    return true;
  } else if (type_ == HTTP2_PRIORITY) {
    if (length() != 5) {
      return false;
    }
    return true;
  } else if (type_ == HTTP2_RST_STREAM) {
    return true;
  } else if (type_ == HTTP2_SETTINGS) {
    if (stream_id() != 0) {
      return false;
    }

    if (flags_ & HTTP2_FLAG_ACK) {
      return length() == 0;
    }

    return length() % 6 == 0;
  } else if (type_ == HTTP2_PUSH_PROMISE) {
    return true;
  } else if (type_ == HTTP2_PING) {
    return true;
  } else if (type_ == HTTP2_GOAWAY) {
    return true;
  } else if (type_ == HTTP2_WINDOW_UPDATE) {
    return true;
  } else if (type_ == HTTP2_CONTINUATION) {
    return true;
  } else {
    return false;
  }
}

frame_t frame_hdr::type() const {
  return static_cast<frame_t>(type_);
}

uint8_t frame_hdr::flags() const {
  return flags_;
}

uint32_t frame_hdr::stream_id() const {
  return stream_id_.id();
}

uint32_t frame_hdr::length() const {
  return static_cast<uint32_t>(length_[0] << 16 | length_[1] << 8 | length_[2]);
}

http2_settings_entry::http2_settings_entry() : settings_id_(), value_() {
  SIZEOF_DATA_MUST_BE(http2_settings_entry, 6);
}

EHTTP2_SETTINGS http2_settings_entry::settings_id() const {
  return static_cast<EHTTP2_SETTINGS>(be16toh(settings_id_));
}

uint32_t http2_settings_entry::value() const {
  return be32toh(value_);
}

http2_priority_spec::http2_priority_spec() : stream_id_(), weight_(0) {
  SIZEOF_DATA_MUST_BE(http2_priority_spec, 5);
}

uint32_t http2_priority_spec::stream_id() const {
  return stream_id_.id();
}

http2_headers_spec::http2_headers_spec() : padlen_(0), pri_spec_() {}

uint8_t http2_headers_spec::padlen() const {
  return padlen_;
}

http2_priority_spec http2_headers_spec::priority() const {
  return pri_spec_;
}

uint8_t http2_priority_spec::weight() const {
  return weight_ + 1;
}

uint32_t http2_goaway_spec::last_stream_id() const {
  return last_stream_id_.id();
}

http2_error_code http2_goaway_spec::error_code() const {
  return static_cast<http2_error_code>(be32toh(error_code_));
}

uint8_t* http2_goaway_spec::opaque_data() const {
  return opaque_data_;
}

// frames

frame_base::frame_base() : header_(), payload_() {}

frame_base::frame_base(const frame_hdr& head, const void* data) : header_(head), payload_() {
  uint32_t payload_size = header_.length();
  if (data && payload_size > 0) {
    payload_.resize(payload_size);
    byte_t* start = &payload_[0];
    memcpy(start, data, payload_size);
  }
}

frame_base::frame_base(const frame_hdr& head, const buffer_t& data) : header_(head), payload_(data) {}

frame_base frame_base::create_frame(const frame_hdr& head, const char* data) {
  frame_t type = head.type();
  switch (type) {
    case HTTP2_DATA: {
      return frame_data(head, data);
    }
    case HTTP2_HEADERS: {
      return frame_headers(head, data);
    }
    case HTTP2_PRIORITY: {
      const http2_priority_spec* spec = reinterpret_cast<const http2_priority_spec*>(data);
      return frame_priority(head, spec);
    }
    case HTTP2_RST_STREAM: {
      const uint32_t* erc = reinterpret_cast<const uint32_t*>(data);
      return frame_rst(head, erc);
    }
    case HTTP2_SETTINGS: {
      const http2_settings_entry* settings = reinterpret_cast<const http2_settings_entry*>(data);
      return frame_settings(head, settings);
    }
    case HTTP2_PUSH_PROMISE: {
      NOTREACHED();
      return frame_base();
    }
    case HTTP2_PING: {
      NOTREACHED();
      return frame_base();
    }
    case HTTP2_GOAWAY: {
      const http2_goaway_spec* sinf = reinterpret_cast<const http2_goaway_spec*>(data);
      return frame_goaway(head, sinf);
    }
    case HTTP2_WINDOW_UPDATE: {
      NOTREACHED();
      return frame_base();
    }
    case HTTP2_CONTINUATION: {
      NOTREACHED();
      return frame_base();
    }
    default: {
      NOTREACHED();
      return frame_base(head, data);
    }
  }
}

bool frame_base::IsValid() const {
  return header_.IsValid();
}

frame_t frame_base::type() const {
  return header_.type();
}

uint32_t frame_base::stream_id() const {
  return header_.stream_id();
}

uint8_t frame_base::flags() const {
  return header_.flags();
}

buffer_t frame_base::payload() const {
  return payload_;
}

const byte_t* frame_base::c_payload() const {
  const byte_t* start = payload_.data();
  return start;
}

uint32_t frame_base::payload_size() const {
  CHECK(payload_.size() == header_.length());
  return header_.length();
}

buffer_t frame_base::raw_data() const {
  if (!IsValid()) {
    return buffer_t();
  }

  buffer_t res;
  res.resize(sizeof(frame_hdr) + payload_size());

  byte_t* start = &res[0];
  memcpy(start, &header_, sizeof(frame_hdr));
  memcpy(start + sizeof(frame_hdr), payload_.data(), payload_size());
  return res;
}

// frame_rst //
frame_rst::frame_rst(const frame_hdr& head, const uint32_t* er_code) : frame_base(head, er_code) {
  CHECK(head.type() == HTTP2_RST_STREAM);
}

frame_hdr frame_rst::create_frame_header(uint8_t flags, uint32_t stream_id) {
  const uint32_t size = sizeof(http2_error_code);
  return frame_hdr(HTTP2_RST_STREAM, flags, stream_id, size);
}

http2_error_code frame_rst::error_code() const {
  const http2_error_code* ec = reinterpret_cast<const http2_error_code*>(payload_.data());
  return static_cast<http2_error_code>(be32toh(*ec));
}
// frame_rst //

// frame_data //
frame_data::frame_data(const frame_hdr& head, const void* data) : frame_base(head, data) {}

frame_data::frame_data(const frame_hdr& head, const buffer_t& data) : frame_base(head, data) {
  CHECK(head.type() == HTTP2_DATA);
}

frame_hdr frame_data::create_frame_header(uint8_t flags, uint32_t stream_id, uint32_t size) {
  return frame_hdr(HTTP2_DATA, flags, stream_id, size);
}

uint8_t frame_data::padlen() const {
  const uint32_t* plen = reinterpret_cast<const uint32_t*>(payload_.data());
  return be32toh(*plen);
}
// frame_data //

// frame_goaway //
frame_goaway::frame_goaway(const frame_hdr& head, const http2_goaway_spec* info) : frame_base(head, info) {
  CHECK(head.type() == HTTP2_GOAWAY);
}

frame_hdr frame_goaway::create_frame_header(uint8_t flags, uint32_t stream_id, uint32_t lenght) {
  return frame_hdr(HTTP2_GOAWAY, flags, stream_id, lenght);
}

uint32_t frame_goaway::last_stream_id() const {
  const http2_goaway_spec* pay = reinterpret_cast<const http2_goaway_spec*>(payload_.data());
  return pay->last_stream_id();
}

http2_error_code frame_goaway::error_code() const {
  const http2_goaway_spec* pay = reinterpret_cast<const http2_goaway_spec*>(payload_.data());
  return pay->error_code();
}

uint8_t* frame_goaway::opaque_data() const {
  return const_cast<uint8_t*>(payload_.data() + (sizeof(http2_error_code) + sizeof(sid)));
}

uint32_t frame_goaway::opaque_data_len() const {
  return payload_size() - sizeof(http2_error_code) - sizeof(sid);
}

// frame_goaway //

// frame_settings //

frame_settings::frame_settings(const frame_hdr& head, const http2_settings_entry* settings)
    : frame_base(head, settings) {
  CHECK(head.type() == HTTP2_SETTINGS);
}

frame_hdr frame_settings::create_frame_header(uint8_t flags, uint32_t stream_id, uint32_t lenght) {
  return frame_hdr(HTTP2_SETTINGS, flags, stream_id, lenght);
}

uint32_t frame_settings::niv() const {
  return payload_size() / sizeof(http2_settings_entry);
}

const http2_settings_entry* frame_settings::iv() const {
  const http2_settings_entry* payl = reinterpret_cast<const http2_settings_entry*>(payload_.data());
  return payl;
}

// frame_settings //

// frame_priority //
frame_priority::frame_priority(const frame_hdr& head, const http2_priority_spec* priority)
    : frame_base(head, priority) {
  CHECK(head.type() == HTTP2_PRIORITY);
}

frame_hdr frame_priority::create_frame_header(uint8_t flags, uint32_t stream_id) {
  const uint32_t size = sizeof(http2_priority_spec);
  return frame_hdr(HTTP2_PRIORITY, flags, stream_id, size);
}

const http2_priority_spec* frame_priority::priority() const {
  const http2_priority_spec* payl = reinterpret_cast<const http2_priority_spec*>(payload_.data());
  return payl;
}
// frame_priority //

// frame_headers //
frame_headers::frame_headers(const frame_hdr& head, const void* data) : frame_base(head, data) {
  CHECK(head.type() == HTTP2_HEADERS);
}

frame_headers::frame_headers(const frame_hdr& head, const buffer_t& data) : frame_base(head, data) {
  CHECK(head.type() == HTTP2_HEADERS);
}

frame_hdr frame_headers::create_frame_header(uint8_t flags, uint32_t stream_id, uint32_t size) {
  return frame_hdr(HTTP2_HEADERS, flags, stream_id, size);
}

uint8_t frame_headers::padlen() const {
  if (header_.flags() & HTTP2_FLAG_PADDED) {
    return payload_[0];
  } else {
    return 0;
  }
}

const http2_priority_spec* frame_headers::priority() const {
  if (header_.flags() & HTTP2_FLAG_PRIORITY) {
    if (header_.flags() & HTTP2_FLAG_PADDED) {
      return reinterpret_cast<const http2_priority_spec*>(payload_.data() + 1);
    }

    return reinterpret_cast<const http2_priority_spec*>(payload_.data());
  }

  return nullptr;
}

http2_nvs_t frame_headers::nva() const {
  http2_nvs_t res;
  http2::http2_inflater ihd;
  uint8_t* in = const_cast<uint8_t*>(c_payload() + sizeof(http2::http2_priority_spec));
  uint32_t inlen = payload_size() - sizeof(http2::http2_priority_spec);

  for (;;) {
    http2::http2_nv nv;
    int inflate_flags = 0;
    ssize_t sz = ihd.http2_inflate_hd(&nv, &inflate_flags, in, inlen, 0);
    if (sz == -1) {
      break;
    }

    in += sz;
    inlen -= sz;

    if (inflate_flags & http2::HTTP2_INFLATE_FINAL) {
      break;
    }
    if ((inflate_flags & http2::HTTP2_INFLATE_EMIT) == 0 && inlen == 0) {
      break;
    }

    res.push_back(nv);
  }

  return res;
}

// frame_headers //

// frames

bool is_preface_data(const char* data, uint32_t len) {
  if (!data || len < PREFACE_STARTS_LEN) {
    return false;
  }

  return strncmp(data, PREFACE_STARTS, PREFACE_STARTS_LEN) == 0;
}

bool is_frame_header_data(const char* data, uint32_t len) {
  if (!data || len < sizeof(frame_hdr)) {
    return false;
  }

  frame_hdr head;
  memcpy(&head, data, sizeof(frame_hdr));

  return head.IsValid();
}

frames_t parse_frames(const char* data, uint32_t len) {
  if (!is_frame_header_data(data, len)) {
    return frames_t();
  }

  const char* start = data;
  frames_t res;
  uint32_t step = 0;
  for (uint32_t i = 0; i < len; i += step) {
    const char* header_start = start + i;
    frame_hdr head;
    memcpy(&head, header_start, sizeof(frame_hdr));

    step = head.length() + sizeof(frame_hdr);
    CHECK(i + step <= len);

    const char* payload_start = header_start + sizeof(frame_hdr);
    frame_base fr = frame_base::create_frame(head, payload_start);
    res.push_back(fr);
  }

  return res;
}

frames_t find_frames_by_type(const frames_t& frames, frame_t type) {
  frames_t res;
  for (size_t i = 0; i < frames.size(); ++i) {
    frame_base fr = frames[i];
    if (fr.type() == type) {
      res.push_back(fr);
    }
  }
  return res;
}

std::pair<http::http_status, Error> parse_http_request(const frame_headers& frame, http::HttpRequest* req_out) {
  if (!frame.IsValid() || frame.type() != HTTP2_HEADERS || !req_out) {
    return std::make_pair(http::HS_FORBIDDEN, make_error_inval());
  }

  uint8_t validation_flags = 0;

  http::http_method lmethod = http::HM_GET;
  std::string lpath;
  http::HttpRequest::body_t lbody;
  http::headers_t lheaders;

  std::vector<http2_nv> nva = frame.nva();
  for (size_t i = 0; i < nva.size(); ++i) {
    http2_nv cur = nva[i];
    std::string key = ConvertToString(cur.name);
    std::string value = ConvertToString(cur.value);

    if (key == ":method") {
      http::http_method met;
      bool res = ConvertFromString(value, &met);
      UNUSED(res);
      lmethod = met;
      validation_flags |= 1;
    } else if (key == ":path") {
      lpath = value;
      validation_flags |= 2;
    } else if (key == ":scheme") {
      // lprotocol = value;
      validation_flags |= 4;
    } else if (key == ":authority") {
      continue;
    } else if (key == ":status") {
      continue;
    } else {
      http::HttpHeader head(key, value);
      lheaders.push_back(head);
    }
  }

  if ((validation_flags & 1) == 0) {
    return std::make_pair(http::HS_NOT_IMPLEMENTED, make_error("That method not specified."));
  } else if ((validation_flags & 2) == 0) {
    return std::make_pair(http::HS_BAD_REQUEST, make_error("Bad filename."));
  } else if ((validation_flags & 4) == 0) {
    return std::make_pair(http::HS_FORBIDDEN, make_error("Scheme not found."));
  }

  *req_out = http::HttpRequest(lmethod, lpath, http::HP_2_0, lheaders, lbody);

  return std::make_pair(http::HS_OK, Error());
}

}  // namespace http2
}  // namespace common
