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

#include <stdint.h>     // for uint32_t, uint8_t
#include <sys/types.h>  // for ssize_t

#include <string>   // for string
#include <utility>  // for pair
#include <vector>   // for vector

#include <common/error.h>      // for Error
#include <common/http/http.h>  // for HttpRequest (ptr only), etc
#include <common/http/http2_huffman.h>

#define PREFACE_STARTS "PRI * HTTP/2.0\r\n\r\nSM\r\n\r\n"
#define PREFACE_STARTS_LEN (sizeof(PREFACE_STARTS) - 1)

#define FRAME_HEADER_SIZE 9

#define HTTP2_STATIC_TABLE_LENGTH 61
#define MAP_SIZE 128
#define HTTP2_ENTRY_OVERHEAD 32
#define HTTP2_MAX_NV 65536
#define HTTP2_DEFAULT_HEADER_TABLE_SIZE (1 << 12)
#define HTTP2_DEFAULT_MAX_BUFFER_SIZE HTTP2_DEFAULT_HEADER_TABLE_SIZE

namespace common {
namespace http2 {

enum EHTTP2_SETTINGS : uint16_t {
  HTTP2_SETTINGS_HEADER_TABLE_SIZE = 0x1,
  HTTP2_SETTINGS_ENABLE_PUSH = 0x2,
  HTTP2_SETTINGS_MAX_CONCURRENT_STREAMS = 0x3,
  HTTP2_SETTINGS_INITIAL_WINDOW_SIZE = 0x4,
  HTTP2_SETTINGS_MAX_FRAME_SIZE = 0x5,
  HTTP2_SETTINGS_MAX_HEADER_LIST_SIZE = 0x6
};

enum frame_t : uint8_t {
  HTTP2_DATA = 0,
  HTTP2_HEADERS = 0x01,
  HTTP2_PRIORITY = 0x02,
  HTTP2_RST_STREAM = 0x03,
  HTTP2_SETTINGS = 0x04,
  HTTP2_PUSH_PROMISE = 0x05,
  HTTP2_PING = 0x06,
  HTTP2_GOAWAY = 0x07,
  HTTP2_WINDOW_UPDATE = 0x08,
  HTTP2_CONTINUATION = 0x09
};

enum http2_error_code : uint32_t {
  HTTP2_NO_ERROR = 0x00,
  HTTP2_PROTOCOL_ERROR = 0x01,
  HTTP2_INTERNAL_ERROR = 0x02,
  HTTP2_FLOW_CONTROL_ERROR = 0x03,
  HTTP2_SETTINGS_TIMEOUT = 0x04,
  HTTP2_STREAM_CLOSED = 0x05,
  HTTP2_FRAME_SIZE_ERROR = 0x06,
  HTTP2_REFUSED_STREAM = 0x07,
  HTTP2_CANCEL = 0x08,
  HTTP2_COMPRESSION_ERROR = 0x09,
  HTTP2_CONNECT_ERROR = 0x0a,
  HTTP2_ENHANCE_YOUR_CALM = 0x0b,
  HTTP2_INADEQUATE_SECURITY = 0x0c,
  HTTP2_HTTP_1_1_REQUIRED = 0x0d
};

enum http2_flag : uint8_t {
  HTTP2_FLAG_NONE = 0,
  HTTP2_FLAG_END_STREAM = 0x01,
  HTTP2_FLAG_END_HEADERS = 0x04,
  HTTP2_FLAG_ACK = 0x01,
  HTTP2_FLAG_PADDED = 0x08,
  HTTP2_FLAG_PRIORITY = 0x20
};

enum http2_token {
  HTTP2_TOKEN__AUTHORITY = 0,
  HTTP2_TOKEN__METHOD = 1,
  HTTP2_TOKEN__PATH = 3,
  HTTP2_TOKEN__SCHEME = 5,
  HTTP2_TOKEN__STATUS = 7,
  HTTP2_TOKEN_ACCEPT_CHARSET = 14,
  HTTP2_TOKEN_ACCEPT_ENCODING = 15,
  HTTP2_TOKEN_ACCEPT_LANGUAGE = 16,
  HTTP2_TOKEN_ACCEPT_RANGES = 17,
  HTTP2_TOKEN_ACCEPT = 18,
  HTTP2_TOKEN_ACCESS_CONTROL_ALLOW_ORIGIN = 19,
  HTTP2_TOKEN_AGE = 20,
  HTTP2_TOKEN_ALLOW = 21,
  HTTP2_TOKEN_AUTHORIZATION = 22,
  HTTP2_TOKEN_CACHE_CONTROL = 23,
  HTTP2_TOKEN_CONTENT_DISPOSITION = 24,
  HTTP2_TOKEN_CONTENT_ENCODING = 25,
  HTTP2_TOKEN_CONTENT_LANGUAGE = 26,
  HTTP2_TOKEN_CONTENT_LENGTH = 27,
  HTTP2_TOKEN_CONTENT_LOCATION = 28,
  HTTP2_TOKEN_CONTENT_RANGE = 29,
  HTTP2_TOKEN_CONTENT_TYPE = 30,
  HTTP2_TOKEN_COOKIE = 31,
  HTTP2_TOKEN_DATE = 32,
  HTTP2_TOKEN_ETAG = 33,
  HTTP2_TOKEN_EXPECT = 34,
  HTTP2_TOKEN_EXPIRES = 35,
  HTTP2_TOKEN_FROM = 36,
  HTTP2_TOKEN_HOST = 37,
  HTTP2_TOKEN_IF_MATCH = 38,
  HTTP2_TOKEN_IF_MODIFIED_SINCE = 39,
  HTTP2_TOKEN_IF_NONE_MATCH = 40,
  HTTP2_TOKEN_IF_RANGE = 41,
  HTTP2_TOKEN_IF_UNMODIFIED_SINCE = 42,
  HTTP2_TOKEN_LAST_MODIFIED = 43,
  HTTP2_TOKEN_LINK = 44,
  HTTP2_TOKEN_LOCATION = 45,
  HTTP2_TOKEN_MAX_FORWARDS = 46,
  HTTP2_TOKEN_PROXY_AUTHENTICATE = 47,
  HTTP2_TOKEN_PROXY_AUTHORIZATION = 48,
  HTTP2_TOKEN_RANGE = 49,
  HTTP2_TOKEN_REFERER = 50,
  HTTP2_TOKEN_REFRESH = 51,
  HTTP2_TOKEN_RETRY_AFTER = 52,
  HTTP2_TOKEN_SERVER = 53,
  HTTP2_TOKEN_SET_COOKIE = 54,
  HTTP2_TOKEN_STRICT_TRANSPORT_SECURITY = 55,
  HTTP2_TOKEN_TRANSFER_ENCODING = 56,
  HTTP2_TOKEN_USER_AGENT = 57,
  HTTP2_TOKEN_VARY = 58,
  HTTP2_TOKEN_VIA = 59,
  HTTP2_TOKEN_WWW_AUTHENTICATE = 60,
  HTTP2_TOKEN_TE,
  HTTP2_TOKEN_CONNECTION,
  HTTP2_TOKEN_KEEP_ALIVE,
  HTTP2_TOKEN_PROXY_CONNECTION,
  HTTP2_TOKEN_UPGRADE
};

enum http2_indexing_mode { HTTP2_WITH_INDEXING, HTTP2_WITHOUT_INDEXING, HTTP2_NEVER_INDEXING };

enum http2_nv_flag { HTTP2_NV_FLAG_NONE = 0, HTTP2_NV_FLAG_NO_INDEX = 0x01 };

struct http2_nv {
  uint32_t namelen() const;
  uint32_t valuelen() const;

  buffer_t name;
  buffer_t value;
  uint8_t flags;
};

struct http2_entry {
  http2_nv nv;
  http2_entry* next;
  uint32_t seq;
  uint32_t hash;
  int token;
};

struct http2_ringbuf {
  explicit http2_ringbuf(uint32_t bufsize);
  ~http2_ringbuf();

  http2_entry** buffer;
  uint32_t mask;
  uint32_t first;
  uint32_t len;
};

struct http2_context {
  http2_context();
  ~http2_context();

  uint32_t hd_table_bufsize_max;
  http2_ringbuf hd_table;
  uint32_t hd_table_bufsize;
  uint32_t next_seq;
  uint8_t bad;
};

typedef struct {
  http2_entry* table[MAP_SIZE];
} http2_map;
typedef std::vector<http2_nv> http2_nvs_t;

struct http2_deflater {
  http2_deflater();

  int http2_deflate_hd_bufs(buffer_t& bufs, const http2_nvs_t& nv);

  http2_context ctx;
  http2_map map;
  uint32_t deflate_hd_table_bufsize_max;
  uint32_t min_hd_table_bufsize_max;
  uint8_t notify_table_size_change;
};

enum http2_opcode { HTTP2_OPCODE_NONE, HTTP2_OPCODE_INDEXED, HTTP2_OPCODE_NEWNAME, HTTP2_OPCODE_INDNAME };

enum http2_inflate_state {
  HTTP2_STATE_EXPECT_TABLE_SIZE,
  HTTP2_STATE_INFLATE_START,
  HTTP2_STATE_OPCODE,
  HTTP2_STATE_READ_TABLE_SIZE,
  HTTP2_STATE_READ_INDEX,
  HTTP2_STATE_NEWNAME_CHECK_NAMELEN,
  HTTP2_STATE_NEWNAME_READ_NAMELEN,
  HTTP2_STATE_NEWNAME_READ_NAMEHUFF,
  HTTP2_STATE_NEWNAME_READ_NAME,
  HTTP2_STATE_CHECK_VALUELEN,
  HTTP2_STATE_READ_VALUELEN,
  HTTP2_STATE_READ_VALUEHUFF,
  HTTP2_STATE_READ_VALUE
};

enum http2_inflate_flag { HTTP2_INFLATE_NONE = 0, HTTP2_INFLATE_FINAL = 0x01, HTTP2_INFLATE_EMIT = 0x02 };

struct http2_inflater {
  http2_inflater();
  ~http2_inflater();

  ssize_t http2_inflate_hd(http2_nv* nv_out, int* inflate_flags, uint8_t* in, uint32_t inlen, int in_final);
  ssize_t http2_inflate_hd2(http2_nv* nv_out,
                            int* inflate_flags,
                            int* token_out,
                            uint8_t* in,
                            uint32_t inlen,
                            int in_final);

  http2_context ctx;
  buffer_t nvbufs;
  http2_entry* ent_keep;
  uint8_t* nv_keep;
  uint32_t left;
  uint32_t index;
  uint32_t newnamelen;
  uint32_t settings_hd_table_bufsize_max;
  uint32_t shift;
  http2_opcode opcode;
  http2_inflate_state state;
  http2_hd_huff_decode_context huff_decode_ctx;
  uint8_t huffman_encoded;
  uint8_t index_required;
  uint8_t no_index;

 private:
  DISALLOW_COPY_AND_ASSIGN(http2_inflater);
};

struct sid {
 public:
  sid();
  explicit sid(uint32_t stream_id);

  uint32_t id() const;

 private:
  uint8_t id_[4];
};

struct frame_hdr {
 public:
  frame_hdr();
  frame_hdr(frame_t type, uint8_t flags, uint32_t stream_id, uint32_t lenght);

  bool IsValid() const;

  frame_t type() const;
  uint8_t flags() const;
  uint32_t stream_id() const;
  uint32_t length() const;

 private:
  uint8_t length_[3];
  uint8_t type_;  // frame_t
  uint8_t flags_;
  sid stream_id_;
};

#pragma pack(push, 1)
struct http2_settings_entry {
 public:
  http2_settings_entry();

  EHTTP2_SETTINGS settings_id() const;
  uint32_t value() const;

 private:
  EHTTP2_SETTINGS settings_id_;
  uint32_t value_;
};
#pragma pack(pop)

struct http2_priority_spec {
 public:
  http2_priority_spec();

  uint32_t stream_id() const;
  uint8_t weight() const;

 private:
  sid stream_id_;
  uint8_t weight_;
};

struct http2_headers_spec {
 public:
  http2_headers_spec();

  uint8_t padlen() const;
  http2_priority_spec priority() const;

 private:
  uint8_t padlen_;
  http2_priority_spec pri_spec_;
};

struct http2_goaway_spec {
 public:
  uint32_t last_stream_id() const;
  http2_error_code error_code() const;
  uint8_t* opaque_data() const;

 private:
  sid last_stream_id_;
  http2_error_code error_code_;
  uint8_t* opaque_data_;
};

// frames

struct frame_base {
 public:
  frame_base();

  bool IsValid() const;

  frame_t type() const;
  uint32_t stream_id() const;
  uint8_t flags() const;

  buffer_t payload() const;
  const byte_t* c_payload() const;
  uint32_t payload_size() const;

  buffer_t raw_data() const;

  static frame_base create_frame(const frame_hdr& head, const char* data);

 protected:
  frame_base(const frame_hdr& head, const void* data);
  frame_base(const frame_hdr& head, const buffer_t& data);

  frame_hdr header_;
  buffer_t payload_;
};

struct frame_rst  // +
    : public frame_base {
  explicit frame_rst(const frame_hdr& head, const uint32_t* er_code);  // betoh

  static frame_hdr create_frame_header(uint8_t flags, uint32_t stream_id);
  http2_error_code error_code() const;
};

struct frame_data  // +
    : public frame_base {
  explicit frame_data(const frame_hdr& head, const void* data);
  explicit frame_data(const frame_hdr& head, const buffer_t& data);

  static frame_hdr create_frame_header(uint8_t flags, uint32_t stream_id, uint32_t size);
  uint8_t padlen() const;
};

struct frame_goaway  // +
    : public frame_base {
  explicit frame_goaway(const frame_hdr& head, const http2_goaway_spec* info);

  static frame_hdr create_frame_header(uint8_t flags, uint32_t stream_id, uint32_t lenght);

  uint32_t last_stream_id() const;
  http2_error_code error_code() const;

  uint8_t* opaque_data() const;
  uint32_t opaque_data_len() const;
};

struct frame_settings                                                                    // +
    : public frame_base {                                                                // payload % 6 = 0
  explicit frame_settings(const frame_hdr& head, const http2_settings_entry* settings);  // info can be NULL

  static frame_hdr create_frame_header(uint8_t flags, uint32_t stream_id, uint32_t lenght);

  uint32_t niv() const;
  const http2_settings_entry* iv() const;
};

struct frame_priority  // +
    : public frame_base {
  explicit frame_priority(const frame_hdr& head, const http2_priority_spec* priority);

  static frame_hdr create_frame_header(uint8_t flags, uint32_t stream_id);
  const http2_priority_spec* priority() const;
};

struct frame_headers  // +
    : public frame_base {
  explicit frame_headers(const frame_hdr& head, const void* data);
  explicit frame_headers(const frame_hdr& head, const buffer_t& data);

  static frame_hdr create_frame_header(uint8_t flags, uint32_t stream_id, uint32_t size);

  uint8_t padlen() const;
  const http2_priority_spec* priority() const;
  http2_nvs_t nva() const;
};

// frames

typedef std::vector<frame_base> frames_t;

bool is_preface_data(const char* data, uint32_t len);
bool is_frame_header_data(const char* data, uint32_t len);
frames_t parse_frames(const char* data, uint32_t len);
frames_t find_frames_by_type(const frames_t& frames, frame_t type);

std::pair<http::http_status, Error> parse_http_request(const frame_headers& frame,
                                                       http::HttpRequest* req_out) WARN_UNUSED_RESULT;
}  // namespace http2

std::string ConvertToString(http::http_method method);
std::string ConvertToString(http::http_status status);
std::string ConvertToString(http::HttpHeader header);
std::string ConvertToString(http::HttpRequest request);
}  // namespace common
