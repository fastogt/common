/*  Copyright (C) 2014-2018 FastoGT. All right reserved.

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

#include <common/net/http_client.h>

namespace common {
namespace net {

HttpClient::HttpClient(const HostAndPort& host) : sock_(host) {}

ErrnoError HttpClient::Connect(struct timeval* tv) {
  return sock_.Connect(tv);
}

ErrnoError HttpClient::Disconnect() {
  return sock_.Disconnect();
}

Error HttpClient::Get(const uri::Upath& path) {
  HostAndPort hs = sock_.GetHost();
  http::HttpHeader header("Host", hs.GetHost());
  http::HttpRequest req(http::HM_GET, path, http::HP_1_0, {header}, std::string());
  return SendRequest(req);
}

Error HttpClient::SendRequest(const http::HttpRequest& request_headers) {
  std::string request_str = ConvertToString(request_headers);

  size_t nwrite;
  ErrnoError err = sock_.Write(request_str, &nwrite);
  if (err) {
    return make_error_from_errno(err);
  }

  return Error();
}

Error HttpClient::ReadResponce(http::HttpResponse* responce) {
  if (!responce) {
    return make_error_inval();
  }

  static const size_t kHeaderBufInitialSize = 4 * 1024;  // 4K
  std::string data;
  size_t nread;
  ErrnoError err = sock_.Read(&data, kHeaderBufInitialSize, &nread);
  if (err) {
    return make_error_from_errno(err);
  }

  return http::parse_http_responce(data, responce);
}

HttpClient::~HttpClient() {}

}  // namespace net
}  // namespace common
