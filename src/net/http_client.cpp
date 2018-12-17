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

#include <string.h>

#include <common/net/socket_tcp.h>

#include <common/convert2string.h>

namespace common {
namespace net {

Error IHttpClient::Get(const uri::Upath& path) {
  const HostAndPort hs = GetHost();
  http::HttpHeader header("Host", hs.GetHost());
  http::HttpRequest req = http::MakeGetRequest(path, http::HP_1_0, {header});
  return SendRequest(req);
}

Error IHttpClient::Head(const uri::Upath& path) {
  const HostAndPort hs = GetHost();
  http::HttpHeader header("Host", hs.GetHost());
  http::HttpRequest req = http::MakeHeadRequest(path, http::HP_1_0, {header});
  return SendRequest(req);
}

Error IHttpClient::SendRequest(const http::HttpRequest& request_headers) {
  std::string request_str = ConvertToString(request_headers);

  size_t nwrite;
  ErrnoError err = sock_->Write(request_str.data(), request_str.size(), &nwrite);
  if (err) {
    last_request_ = common::Optional<http::HttpRequest>();
    return make_error_from_errno(err);
  }

  last_request_ = request_headers;
  return Error();
}

Error IHttpClient::ReadResponce(http::HttpResponse* responce) {
  if (!responce || !last_request_) {
    return make_error_inval();
  }

  static const size_t kHeaderBufInitialSize = 4 * 1024;  // 4K
  char* data_head = new char[kHeaderBufInitialSize];
  size_t nread_head;
  ErrnoError err = sock_->Read(data_head, kHeaderBufInitialSize, &nread_head);
  if (err) {
    delete[] data_head;
    return make_error_from_errno(err);
  }

  size_t not_parsed;
  Error parse_error = http::parse_http_responce(std::string(data_head, nread_head), responce, &not_parsed);
  if (parse_error) {
    delete[] data_head;
    return parse_error;
  }

  if (last_request_->GetMethod() == http::HM_HEAD) {  // head without body
    delete[] data_head;
    return Error();
  }

  if (!responce->IsEmptyBody()) {
    CHECK_EQ(not_parsed, 0);
    delete[] data_head;
    return Error();
  }

  http::header_t cont;
  if (!responce->FindHeaderByKey("Content-Length", false, &cont)) {  // try to get body
    delete[] data_head;
    return Error();
  }

  size_t body_len = 0;
  if (!(ConvertFromString(cont.value, &body_len) && body_len)) {
    delete[] data_head;
    return Error();
  }

  char* data = new char[body_len];
  const size_t rest = body_len - not_parsed;
  if (not_parsed) {
    const char* body_str = data_head + nread_head - not_parsed;
    memcpy(data, body_str, not_parsed);
  }
  size_t nread;
  err = sock_->Read(data + not_parsed, rest, &nread);  // read rest
  if (!err && nread == rest) {
    std::string body(data, body_len);
    responce->SetBody(body);
    delete[] data_head;
    delete[] data;
    return Error();
  }

  delete[] data_head;
  delete[] data;
  return make_error("Invalid body read");
}

IHttpClient::~IHttpClient() {
  delete sock_;
  sock_ = nullptr;
}

IHttpClient::IHttpClient(net::ISocket* sock) : sock_(sock) {
  CHECK(sock) << "Socket must be passed!";
}

net::ISocket* IHttpClient::GetSocket() const {
  return sock_;
}

HttpClient::HttpClient(const HostAndPort& host) : IHttpClient(new net::ClientSocketTcp(host)) {}

ErrnoError HttpClient::Connect(struct timeval* tv) {
  net::ClientSocketTcp* sock = static_cast<net::ClientSocketTcp*>(GetSocket());
  return sock->Connect(tv);
}

bool HttpClient::IsConnected() const {
  net::ClientSocketTcp* sock = static_cast<net::ClientSocketTcp*>(GetSocket());
  return sock->IsConnected();
}

ErrnoError HttpClient::Disconnect() {
  net::ClientSocketTcp* sock = static_cast<net::ClientSocketTcp*>(GetSocket());
  return sock->Disconnect();
}

HostAndPort HttpClient::GetHost() const {
  net::ClientSocketTcp* sock = static_cast<net::ClientSocketTcp*>(GetSocket());
  return sock->GetHost();
}

}  // namespace net
}  // namespace common
