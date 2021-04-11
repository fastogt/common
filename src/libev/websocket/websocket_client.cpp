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

#include <common/libev/websocket/websocket_client.h>

#include <common/convert2string.h>
#include <common/hash/sha1.h>
#include <common/utils.h>

namespace common {
namespace libev {
namespace websocket {

WebSocketClient::WebSocketClient(libev::IoLoop* server, const net::socket_info& info) : HttpClient(server, info) {}

WebSocketClient::~WebSocketClient() {}

const char* WebSocketClient::ClassName() const {
  return "WebSocketClient";
}

ErrnoError WebSocketClient::StartHandshake(const uri::GURL& url, const http::HttpServerInfo& info) {
  UNUSED(info);
  if (!url.is_valid()) {
    return make_errno_error_inval();
  }

  if (!url.SchemeIsWSOrWSS()) {
    return make_errno_error_inval();
  }

  common::http::headers_t headers = {common::http::HttpHeader("Upgrade", "websocket"),
                                     common::http::HttpHeader("User-Agent", USER_AGENT_VALUE),
                                     common::http::HttpHeader("Connection", "Upgrade"),
                                     common::http::HttpHeader("Sec-WebSocket-Key", "dGhlIHNhbXBsZSBub25jZQ=="),
                                     common::http::HttpHeader("Sec-WebSocket-Version", "13")};
  return SendRequest(common::http::HM_GET, url, common::http::HP_1_1, headers);
}

ErrnoError WebSocketClient::SendSwitchProtocolsResponse(const std::string& key,
                                                        const std::string& protocol,
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
  if (!protocol.empty()) {
    headers.push_back(common::http::HttpHeader("Sec-WebSocket-Protocol", protocol));
  }
  return SendResponse(common::http::http_protocol::HP_1_1, common::http::http_status::HS_SWITCH_PROTOCOL, headers,
                      info);
}

}  // namespace websocket
}  // namespace libev
}  // namespace common
