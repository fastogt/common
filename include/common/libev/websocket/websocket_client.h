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

#include <common/libev/http/http_client.h>
#include <common/uri/gurl.h>

namespace common {
namespace libev {
namespace websocket {

enum WsStep {
  ZERO,   // before websocket handshake
  ONE,    // 0-2 bytes, fin, opcode, mask, payload length
  TWO,    // extended payload length
  THREE,  // masking-key
  FOUR,   // payload data
  UNKNOWN
};

typedef struct Frame {
  uint8_t fin;
  uint8_t opcode;
  uint8_t mask;
  uint64_t payload_len;
  unsigned char masking_key[4];
} frame_t;

class WebSocketClient : public http::HttpClient {
 public:
  WebSocketClient(libev::IoLoop* server, const net::socket_info& info);
  virtual ~WebSocketClient();

  const char* ClassName() const override;

  virtual ErrnoError SendFrame(const char* data, size_t size) WARN_UNUSED_RESULT;
  ErrnoError SendEOS() WARN_UNUSED_RESULT;
  ErrnoError StartHandshake(const uri::GURL& url,
                            const common::http::headers_t& extra_headers,
                            const http::HttpServerInfo& info) WARN_UNUSED_RESULT;
};

class WebSocketServerClient : public WebSocketClient {
 public:
  WebSocketServerClient(libev::IoLoop* server, const net::socket_info& info);
  virtual ~WebSocketServerClient();

  const char* ClassName() const override;

  ErrnoError SendSwitchProtocolsResponse(const std::string& key,
                                         const common::http::headers_t& extra_headers,
                                         const http::HttpServerInfo& info) WARN_UNUSED_RESULT;

  WsStep Step() const;

  ErrnoError SendFrame(const char* data, size_t size) override WARN_UNUSED_RESULT;

  ErrnoError ProcessFrame(std::function<void(char*, size_t)> pred) WARN_UNUSED_RESULT;

 private:
  ErrnoError SendPong() WARN_UNUSED_RESULT;

  frame_t frame_;
  size_t ntoread_;
  WsStep step_;
};

}  // namespace websocket
}  // namespace libev
}  // namespace common
