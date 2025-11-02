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

#include <common/convert2string.h>
#include <common/hash/sha1.h>
#include <common/libev/websocket/websocket_client.h>
#include <common/utils.h>

#define BUF_SIZE 4096
#define MAX_PAYLOAD_SIZE (1024 * 1024)

namespace {

inline uint16_t myhtons(uint16_t n) {
  return ((n & 0xff00) >> 8) | ((n & 0x00ff) << 8);
}

inline uint16_t myntohs(uint16_t n) {
  return ((n & 0xff00) >> 8) | ((n & 0x00ff) << 8);
}

inline uint32_t myhtonl(uint32_t n) {
  return ((n & 0xff000000) >> 24) | ((n & 0x00ff0000) >> 8) | ((n & 0x0000ff00) << 8) | ((n & 0x000000ff) << 24);
}

inline uint32_t myntohl(uint32_t n) {
  return ((n & 0xff000000) >> 24) | ((n & 0x00ff0000) >> 8) | ((n & 0x0000ff00) << 8) | ((n & 0x000000ff) << 24);
}

inline uint64_t myhtonll(uint64_t n) {
  return (uint64_t)myhtonl(n >> 32) | ((uint64_t)myhtonl(n) << 32);
}

inline uint64_t myntohll(uint64_t n) {
  return (uint64_t)myhtonl(n >> 32) | ((uint64_t)myhtonl(n) << 32);
}

int32_t unmask_payload_data(const char* masking_key, char* payload_data, size_t payload_len) {
  if (!masking_key || !payload_data || payload_len == 0) {
    return -1;
  }
  for (size_t i = 0; i < payload_len; ++i) {
    *(payload_data + i) = *(payload_data + i) ^ *(masking_key + i % 4);
  }
  return 0;
}
}  // namespace

namespace common {
namespace libev {
namespace websocket {

namespace {
typedef struct FrameBuffer {
  char* data;
  size_t len;
} frame_buffer_t;

void domask(char* payload, size_t size, const char masking_key[4]) {
  int i = 0;
  while (size-- > 0)
    *payload++ ^= masking_key[i++ % 4];
}

frame_buffer_t* frame_buffer_new(uint8_t fin,
                                 uint8_t opcode,
                                 const char* payload_data,
                                 size_t payload_len,
                                 uint8_t mask,
                                 const char masking_key[4]) {
  if (fin > 1 || opcode > 0xf) {
    return NULL;
  }

  char* p = NULL;    // buffer
  uint64_t len = 0;  // buffer length

  unsigned char c1 = 0x00;
  unsigned char c2 = 0x00;
  c1 = c1 | (fin << 7);   // set fin
  c1 = c1 | opcode;       // set opcode
  c2 = c2 | (mask << 7);  // set mask

  if (!payload_data || payload_len == 0) {
    if (mask == 0) {
      p = new char[2];
      *p = c1;
      *(p + 1) = c2;
      len = 2;
    } else {
      p = new char[2 + 4];
      *p = c1;
      *(p + 1) = c2;
      memcpy(p + 2, masking_key, 4);
      len = 2 + 4;
    }
  } else if (payload_data && payload_len <= 125) {
    if (mask == 0) {
      p = new char[2 + payload_len];
      *p = c1;
      *(p + 1) = c2 + payload_len;
      memcpy(p + 2, payload_data, payload_len);
      len = 2 + payload_len;
    } else {
      p = new char[2 + 4 + payload_len];
      *p = c1;
      *(p + 1) = c2 + payload_len;
      memcpy(p + 2, masking_key, 4);
      memcpy(p + 6, payload_data, payload_len);
      domask(p + 6, payload_len, masking_key);
      len = 2 + 4 + payload_len;
    }
  } else if (payload_data && payload_len >= 126 && payload_len <= 65535) {
    if (mask == 0) {
      p = new char[4 + payload_len];
      *p = c1;
      *(p + 1) = c2 + 126;
      uint16_t tmplen = myhtons((uint16_t)payload_len);
      memcpy(p + 2, &tmplen, 2);
      memcpy(p + 4, payload_data, payload_len);
      len = 4 + payload_len;
    } else {
      p = new char[4 + 4 + payload_len];
      *p = c1;
      *(p + 1) = c2 + 126;
      uint16_t tmplen = myhtons((uint16_t)payload_len);
      memcpy(p + 2, &tmplen, 2);
      memcpy(p + 4, masking_key, 4);
      memcpy(p + 8, payload_data, payload_len);
      domask(p + 8, payload_len, masking_key);
      len = 4 + 4 + payload_len;
    }
  } else if (payload_data && payload_len >= 65536) {
    if (mask == 0) {
      p = new char[10 + payload_len];
      *p = c1;
      *(p + 1) = c2 + 127;
      uint64_t tmplen = myhtonll(payload_len);
      memcpy(p + 2, &tmplen, 8);
      memcpy(p + 10, payload_data, payload_len);
      len = 10 + payload_len;
    } else {
      p = new char[10 + 4 + payload_len];
      *p = c1;
      *(p + 1) = c2 + 127;
      uint64_t tmplen = myhtonll(payload_len);
      memcpy(p + 2, &tmplen, 8);
      memcpy(p + 10, masking_key, 4);
      memcpy(p + 14, payload_data, payload_len);
      domask(p + 14, payload_len, masking_key);
      len = 10 + 4 + payload_len;
    }
  }

  frame_buffer_t* fb = NULL;
  if (p && len > 0) {
    fb = new frame_buffer_t;
    if (fb) {
      fb->data = p;
      fb->len = len;
    }
  }
  return fb;
}

frame_buffer_t* frame_buffer_server_new(uint8_t fin, uint8_t opcode, const char* payload_data, size_t payload_len) {
  uint8_t mask = 0;           // must not mask at server endpoint
  char masking_key[4] = {0};  // no need at server endpoint

  return frame_buffer_new(fin, opcode, payload_data, payload_len, mask, masking_key);
}

void frame_buffer_free(frame_buffer_t* fb) {
  if (fb) {
    if (fb->data) {
      delete[] fb->data;
    }
    delete fb;
  }
}

bool parse_frame_header(const char* buf, frame_t* frame) {
  if (!buf || !frame) {
    return false;
  }

  unsigned char c1 = *buf;
  unsigned char c2 = *(buf + 1);
  frame->fin = (c1 >> 7) & 0xff;
  frame->opcode = c1 & 0x0f;
  frame->mask = (c2 >> 7) & 0xff;
  frame->payload_len = c2 & 0x7f;
  return true;
}

const char masking_key[4] = {0x12, 0x34, 0x56, 0x78};
}  // namespace

WebSocketClient::WebSocketClient(libev::IoLoop* server, const net::socket_info& info)
    : http::HttpServerClient(server, info) {}

WebSocketClient::~WebSocketClient() {}

const char* WebSocketClient::ClassName() const {
  return "WebSocketClient";
}

ErrnoError WebSocketClient::SendFrame(const char* data, size_t size) {
  frame_buffer_t* fb = frame_buffer_new(1, 1, data, size, 1, masking_key);
  size_t nwrite = 0;
  ErrnoError errn = SingleWrite(fb->data, fb->len, &nwrite);
  frame_buffer_free(fb);
  return errn;
}

ErrnoError WebSocketClient::StartHandshake(const uri::GURL& url, const common::http::headers_t& extra_headers) {
  if (!url.is_valid()) {
    return make_errno_error_inval();
  }

  if (!url.SchemeIsWSOrWSS()) {
    return make_errno_error_inval();
  }

  srand(::time(NULL));
  unsigned char key_nonce[16];
  for (int z = 0; z < 16; z++) {
    key_nonce[z] = rand() & 0xff;
  }

  std::string base64;
  auto buf = MAKE_CHAR_BUFFER_SIZE(key_nonce, 16);
  if (!common::utils::base64::encode64(buf, &base64)) {
    return make_errno_error("can't decode key to base64", EAGAIN);
  }

  common::http::headers_t headers = {
      common::http::HttpHeader("Upgrade", "websocket"), common::http::HttpHeader("User-Agent", USER_AGENT_VALUE),
      common::http::HttpHeader("Connection", "Upgrade"), common::http::HttpHeader("Sec-WebSocket-Key", base64),
      common::http::HttpHeader("Sec-WebSocket-Version", "13")};

  for (size_t i = 0; i < extra_headers.size(); ++i) {
    const auto header = extra_headers[i];
    headers.push_back(header);
  }
  return SendRequest(common::http::HM_GET, url, common::http::HP_1_1, headers);
}

ErrnoError WebSocketClient::SendEOS() {
  frame_buffer_t* fb = frame_buffer_server_new(1, 8, nullptr, 0);
  size_t nwrite = 0;
  ErrnoError errn = SingleWrite(fb->data, fb->len, &nwrite);
  frame_buffer_free(fb);
  return errn;
}

WebSocketServerClient::WebSocketServerClient(libev::IoLoop* server, const net::socket_info& info)
    : WebSocketClient(server, info), frame_{}, ntoread_(0), step_(ZERO) {}

WebSocketServerClient::~WebSocketServerClient() {}

const char* WebSocketServerClient::ClassName() const {
  return "WebSocketServerClient";
}

WsStep WebSocketServerClient::Step() const {
  return step_;
}

ErrnoError WebSocketServerClient::SendFrame(const char* data, size_t size) {
  frame_buffer_t* fb = frame_buffer_server_new(1, 1, data, size);
  size_t nwrite = 0;
  ErrnoError errn = SingleWrite(fb->data, fb->len, &nwrite);
  frame_buffer_free(fb);
  return errn;
}

ErrnoError WebSocketServerClient::SendPong() {
  frame_buffer_t* fb = frame_buffer_server_new(1, 10, nullptr, 0);
  size_t nwrite = 0;
  ErrnoError errn = SingleWrite(fb->data, fb->len, &nwrite);
  frame_buffer_free(fb);
  return errn;
}

ErrnoError WebSocketServerClient::ProcessFrame(std::function<void(char*, size_t)> pred) {
  if (step_ == common::libev::websocket::ONE) {
    char buff[BUF_SIZE] = {0};
    size_t nread = 0;
    common::ErrnoError errn = SingleRead(buff, ntoread_, &nread);
    if (errn || nread == 0) {
      if (nread == 0) {
        return make_errno_error("Connection closed", EAGAIN);
      }
      return errn;
    }

    if (parse_frame_header(buff, &frame_)) {
      if (frame_.payload_len <= 125) {
        step_ = THREE;
        ntoread_ = 4;
      } else if (frame_.payload_len == 126) {
        step_ = TWO;
        ntoread_ = 2;
      } else if (frame_.payload_len == 127) {
        step_ = TWO;
        ntoread_ = 8;
      }
    }
  } else if (step_ == common::libev::websocket::TWO) {
    char* tmp = (char*)malloc(ntoread_);
    size_t nread = 0;
    common::ErrnoError errn = SingleRead(tmp, ntoread_, &nread);
    if (errn || nread == 0) {
      free(tmp);
      if (nread == 0) {
        return make_errno_error("Connection closed", EAGAIN);
      }
      return errn;
    }

    if (frame_.payload_len == 126) {
      frame_.payload_len = ntohs(*(uint16_t*)tmp);
    } else if (frame_.payload_len == 127) {
      frame_.payload_len = myntohll(*(uint64_t*)tmp);
    }
    free(tmp);
    step_ = THREE;
    ntoread_ = 4;
  } else if (step_ == common::libev::websocket::THREE) {
    char buff[BUF_SIZE] = {0};
    size_t nread = 0;
    common::ErrnoError errn = SingleRead(buff, ntoread_, &nread);
    if (errn || nread == 0) {
      if (nread == 0) {
        return make_errno_error("Connection closed", EAGAIN);
      }
      return errn;
    }

    memcpy(frame_.masking_key, buff, ntoread_);
    if (frame_.payload_len > 0) {
      step_ = FOUR;
      ntoread_ = frame_.payload_len;
    } else if (frame_.payload_len == 0) {
      /*recv a whole frame*/
      if (frame_.mask == 0) {
        // recv an unmasked frame
      }
      if (frame_.fin == 1 && frame_.opcode == 0x8) {
        // 0x8 denotes a connection close
        SendEOS();
        return make_errno_error("Connection closed", EAGAIN);
      } else if (frame_.fin == 1 && frame_.opcode == 0x9) {
        // 0x9 denotes a ping
        SendPong();
      } else {
      }

      step_ = ONE;
      ntoread_ = 2;
      memset(&frame_, 0, sizeof(frame_t));
    }
  } else if (step_ == common::libev::websocket::FOUR) {
    if (frame_.payload_len > MAX_PAYLOAD_SIZE) {
      return make_errno_error("Payload too large", E2BIG);
    }
    if (frame_.payload_len > 0) {
      char* buff = new char[frame_.payload_len];
      if (!buff) {
        return make_errno_error("Memory allocation failed", ENOMEM);
      }
      size_t total_read = 0;
      while (total_read < frame_.payload_len) {
        size_t nread = 0;
        common::ErrnoError errn = SingleRead(buff + total_read, frame_.payload_len - total_read, &nread);
        if (errn || nread == 0) {
          delete[] buff;
          if (nread == 0) {
            return make_errno_error("Connection closed", EAGAIN);
          }
          return errn;
        }
        total_read += nread;
      }
      if (frame_.mask == 1) {
        unmask_payload_data((const char*)frame_.masking_key, buff, frame_.payload_len);
      }
      pred(buff, total_read);
      delete[] buff;
    }

    /*recv a whole frame*/
    if (frame_.fin == 1 && frame_.opcode == 0x8) {
      // 0x8 denotes a connection close
      SendEOS();
      return make_errno_error("Connection closed", EAGAIN);
    } else if (frame_.fin == 1 && frame_.opcode == 0x9) {
      // 0x9 denotes a ping
      SendPong();
    } else {
      // execute custom operation
    }

    if (frame_.opcode == 0x1) {  // 0x1 denotes a text frame
    }
    if (frame_.opcode == 0x2) {  // 0x1 denotes a binary frame
    }

    step_ = ONE;
    ntoread_ = 2;
    memset(&frame_, 0, sizeof(frame_t));
  }
  return ErrnoError();
}

ErrnoError WebSocketServerClient::SendSwitchProtocolsResponse(const std::string& key,
                                                              const common::http::headers_t& extra_headers,
                                                              const http::HttpServerInfo& info) {
  if (key.empty()) {
    return make_errno_error_inval();
  }

  common::hash::SHA1_CTX ctx;
  const common::buffer_t bytes_key = ConvertToBytes(key + "258EAFA5-E914-47DA-95CA-C5AB0DC85B11");
  common::hash::SHA1_Init(&ctx);
  common::hash::SHA1_Update(&ctx, bytes_key.data(), bytes_key.size());
  unsigned char sha1_result[SHA1_HASH_LENGTH];
  common::hash::SHA1_Final(&ctx, sha1_result);
  std::string base64;
  if (!common::utils::base64::encode64(MAKE_CHAR_BUFFER_SIZE(sha1_result, SHA1_HASH_LENGTH), &base64)) {
    return make_errno_error("can't encode key to base64", EAGAIN);
  }

  common::http::headers_t headers = {
      common::http::HttpHeader("Upgrade", "websocket"), common::http::HttpHeader("User-Agent", USER_AGENT_VALUE),
      common::http::HttpHeader("Connection", "Upgrade"), common::http::HttpHeader("Sec-WebSocket-Accept", base64)};
  for (size_t i = 0; i < extra_headers.size(); ++i) {
    const auto header = extra_headers[i];
    headers.push_back(header);
  }
  ErrnoError errn =
      SendResponse(common::http::http_protocol::HP_1_1, common::http::http_status::HS_SWITCH_PROTOCOL, headers, info);
  if (!errn) {
    step_ = ONE;
    ntoread_ = 2;
    memset(&frame_, 0, sizeof(frame_t));
  }
  return errn;
}

}  // namespace websocket
}  // namespace libev
}  // namespace common
