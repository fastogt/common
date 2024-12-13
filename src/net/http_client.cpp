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

#include <common/net/http_client.h>
#include <string.h>
#include <unistd.h>

#include <common/convert2string.h>
#include <common/file_system/file.h>
#include <common/file_system/file_system.h>
#include <common/http/http_chunked_decoder.h>
#include <common/net/socket_tcp.h>

#if defined(HAVE_OPENSSL)
#include <common/net/socket_tcp_tls.h>
#endif

#undef SetPort

namespace common {
namespace net {

Error IHttpClient::PostFile(const url_t& path,
                            const file_system::ascii_file_string_path& file_path,
                            const http::headers_t& extra_headers) {
  file_system::File file;
  ErrnoError errn = file.Open(file_path, file_system::File::FLAG_OPEN | file_system::File::FLAG_READ);
  if (errn) {
    return make_error_from_errno(errn);
  }

  off_t file_size;
  const descriptor_t fd = file.GetFd();
  errn = file_system::get_file_size_by_descriptor(fd, &file_size);
  if (errn) {
    file.Close();
    return make_error_from_errno(errn);
  }

  const HostAndPort hs = GetHost();
  http::HttpHeader header("Host", ConvertToString(hs));
  http::HttpHeader type("Content-Type", "application/octet-stream");
  http::HttpHeader disp("Content-Disposition", MemSPrintf("attachment, filename=\"%s\"", file_path.GetName()));
  http::HttpHeader cont("Content-Length", ConvertToString(file_size));
  http::HttpHeader user("User-Agent", USER_AGENT_VALUE);

  http::headers_t headers = {header, type, disp, cont, user};
  std::copy(extra_headers.begin(), extra_headers.end(), std::back_inserter(headers));
  auto req = http::MakePostRequest(path, http::HP_1_1, headers);
  if (!req) {
    return make_error("Can't make request.");
  }
  Error err = SendRequest(*req);
  if (err) {
    file.Close();
    return err;
  }

  errn = SendFile(fd, file_size);
  if (errn) {
    file.Close();
    return make_error_from_errno(errn);
  }

  return Error();
}

Error IHttpClient::Get(const url_t& path, const http::headers_t& extra_headers) {
  const HostAndPort hs = GetHost();
  http::HttpHeader header("Host", ConvertToString(hs));
  http::HttpHeader user("User-Agent", USER_AGENT_VALUE);
  http::headers_t headers = {header, user};
  std::copy(extra_headers.begin(), extra_headers.end(), std::back_inserter(headers));
  auto req = http::MakeGetRequest(path, http::HP_1_1, headers);
  if (!req) {
    return make_error("Can't make request.");
  }
  return SendRequest(*req);
}

Error IHttpClient::Head(const url_t& path, const http::headers_t& extra_headers) {
  const HostAndPort hs = GetHost();
  http::HttpHeader header("Host", ConvertToString(hs));
  http::HttpHeader user("User-Agent", USER_AGENT_VALUE);
  http::headers_t headers = {header, user};
  std::copy(extra_headers.begin(), extra_headers.end(), std::back_inserter(headers));
  auto req = http::MakeHeadRequest(path, http::HP_1_1, headers);
  if (!req) {
    return make_error("Can't make request.");
  }
  return SendRequest(*req);
}

Error IHttpClient::SendRequest(const http::HttpRequest& request_headers) {
  if (!request_headers.IsValid()) {
    return make_error_inval();
  }

  std::string request_str = ConvertToString(request_headers);
  size_t nwrite;
  ErrnoError err = sock_->Write(request_str.data(), request_str.size(), &nwrite);
  if (err) {
    last_request_ = Optional<http::HttpRequest>();
    return make_error_from_errno(err);
  }

  last_request_ = request_headers;
  return Error();
}

Error IHttpClient::ReadResponse(http::HttpResponse* response) {
  if (!response || !last_request_) {
    return make_error_inval();
  }

  static const size_t kHeaderBufInitialSize = 16 * 1024;  // 16K
  char* data_head = new char[kHeaderBufInitialSize];
  size_t nread_head = 0;
  ErrnoError err = sock_->Read(data_head, kHeaderBufInitialSize, &nread_head);
  if (err || nread_head == 0) {
    delete[] data_head;
    return make_error_from_errno(err);
  }

  size_t not_parsed;
  Error parse_error = http::parse_http_response(std::string(data_head, nread_head), response, &not_parsed);
  if (parse_error) {
    delete[] data_head;
    return parse_error;
  }

  if (last_request_->GetMethod() == http::HM_HEAD) {  // head without body
    delete[] data_head;
    return Error();
  }

  if (!response->IsEmptyBody()) {
    CHECK_EQ(not_parsed, 0);
    delete[] data_head;
    return Error();
  }

  bool chunked = false;
  http::header_t encoding;
  if (response->FindHeaderByKey("Transfer-Encoding", false, &encoding)) {
    chunked = EqualsASCII(encoding.value, "chunked", false);
  }

  http::header_t cont;
  if (chunked || !response->FindHeaderByKey("Content-Length", false, &cont)) {  // try to get body
    http::HttpResponse::body_t body;
    if (not_parsed) {
      const char* body_str = data_head + nread_head - not_parsed;
      body = MAKE_CHAR_BUFFER_SIZE(body_str, not_parsed);
    }

    size_t read_size = kHeaderBufInitialSize;
    do {
      size_t nread = 0;
      err = sock_->Read(data_head, read_size, &nread);
      if (err || nread == 0) {
        break;
      }
      body += std::string(data_head, nread);
    } while (!err);

    if (chunked) {
      http::HttpChunkedDecoder dec;
      int res;
      common::Error cerr = dec.FilterBuf(body.data(), body.size(), &res);
      if (cerr) {
        delete[] data_head;
        return cerr;
      }

      if (res == 0 && !dec.reached_eof()) {
        return common::make_error_inval();
      }
      body.resize(res);
    }

    response->SetBody(body);
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

  size_t total = 0;          // how many bytes we've readed
  size_t bytes_left = rest;  // how many we have left to read

  while (total < rest) {
    size_t n = 0;
    size_t diff = not_parsed + total;
    char* start = data + diff;
    err = sock_->Read(start, bytes_left, &n);
    if (err || n == 0) {
      delete[] data_head;
      delete[] data;
      return make_error("Invalid body read");
    }
    total += n;
    bytes_left -= n;
  }

  http::HttpResponse::body_t body(MAKE_CHAR_BUFFER_SIZE(data, body_len));
  response->SetBody(body);
  delete[] data_head;
  delete[] data;
  return Error();
}

IHttpClient::~IHttpClient() {
  delete sock_;
  sock_ = nullptr;
}

IHttpClient::IHttpClient(ISocket* sock) : sock_(sock) {
  CHECK(sock) << "Socket must be passed!";
}

ISocket* IHttpClient::GetSocket() const {
  return sock_;
}

HttpServerClient::HttpServerClient(const HostAndPort& host) : IHttpClient(new ClientSocketTcp(host)) {}

ErrnoError HttpServerClient::Connect(struct timeval* tv) {
  ClientSocketTcp* sock = static_cast<ClientSocketTcp*>(GetSocket());
  return sock->Connect(tv);
}

bool HttpServerClient::IsConnected() const {
  ClientSocketTcp* sock = static_cast<ClientSocketTcp*>(GetSocket());
  return sock->IsConnected();
}

ErrnoError HttpServerClient::Disconnect() {
  ClientSocketTcp* sock = static_cast<ClientSocketTcp*>(GetSocket());
  return sock->Disconnect();
}

ErrnoError HttpServerClient::SendFile(descriptor_t file_fd, size_t file_size) {
  ClientSocketTcp* sock = static_cast<ClientSocketTcp*>(GetSocket());
  return sock->SendFile(file_fd, file_size);
}

HostAndPort HttpServerClient::GetHost() const {
  ClientSocketTcp* sock = static_cast<ClientSocketTcp*>(GetSocket());
  return sock->GetHost();
}

Error GetHttpFile(const uri::GURL& url,
                  const file_system::ascii_file_string_path& file_path,
                  const http::headers_t& extra_headers,
                  struct timeval* tv) {
  if (!url.is_valid() || !file_path.IsValid() || !url.SchemeIs("http")) {
    return common::make_error_inval();
  }

  HostAndPort http_server_address(url.host(), url.EffectiveIntPort());
  HttpServerClient cl(http_server_address);
  ErrnoError errn = cl.Connect(tv);
  if (errn) {
    return make_error_from_errno(errn);
  }

  const auto path = url.PathForRequest();
  Error err = cl.Get(path, extra_headers);
  if (err) {
    cl.Disconnect();
    return err;
  }

  http::HttpResponse lresp;
  err = cl.ReadResponse(&lresp);
  cl.Disconnect();
  if (err) {
    return err;
  }

  errn = file_system::remove_file(file_path.GetPath());
  if (errn) {
    return make_error_from_errno(errn);
  }

  file_system::File file;
  errn = file.Open(file_path, file_system::File::FLAG_CREATE | file_system::File::FLAG_WRITE);
  if (errn) {
    return make_error_from_errno(errn);
  }

  size_t out;
  auto body = lresp.GetBody();
  errn = file.WriteBuffer(body, &out);
  file.Close();

  if (out != body.size()) {
    ignore_result(file_system::remove_file(file_path.GetPath()));
    return common::make_error("failed to save file");
  }

  if (errn) {
    ignore_result(file_system::remove_file(file_path.GetPath()));
    return make_error_from_errno(errn);
  }
  return err;
}

Error PostHttpFile(const file_system::ascii_file_string_path& file_path,
                   const uri::GURL& url,
                   const http::headers_t& extra_headers,
                   struct timeval* tv) {
  if (!url.is_valid() || !file_path.IsValid() || !url.SchemeIs("http")) {
    return common::make_error_inval();
  }

  HostAndPort http_server_address(url.host(), url.EffectiveIntPort());
  HttpServerClient cl(http_server_address);
  ErrnoError errn = cl.Connect(tv);
  if (errn) {
    return make_error_from_errno(errn);
  }

  const auto path = url.PathForRequest();
  Error err = cl.PostFile(path, file_path, extra_headers);
  if (err) {
    cl.Disconnect();
    return err;
  }

  http::HttpResponse lresp;
  err = cl.ReadResponse(&lresp);
  cl.Disconnect();
  return err;
}

#if defined(HAVE_OPENSSL)
HttpsClient::HttpsClient(const common::net::HostAndPort& host) : base_class(new ClientSocketTcpTls(host)) {}

common::ErrnoError HttpsClient::Connect(struct timeval* tv) {
  ClientSocketTcpTls* sock = static_cast<ClientSocketTcpTls*>(GetSocket());
  return sock->Connect(tv);
}

bool HttpsClient::IsConnected() const {
  ClientSocketTcpTls* sock = static_cast<ClientSocketTcpTls*>(GetSocket());
  return sock->IsConnected();
}

common::ErrnoError HttpsClient::Disconnect() {
  ClientSocketTcpTls* sock = static_cast<ClientSocketTcpTls*>(GetSocket());
  return sock->Disconnect();
}

common::net::HostAndPort HttpsClient::GetHost() const {
  ClientSocketTcpTls* sock = static_cast<ClientSocketTcpTls*>(GetSocket());
  return sock->GetHost();
}

ErrnoError HttpsClient::SendFile(descriptor_t file_fd, size_t file_size) {
  ClientSocketTcpTls* sock = static_cast<ClientSocketTcpTls*>(GetSocket());
  return sock->SendFile(file_fd, file_size);
}

Error GetHttpsFile(const uri::GURL& url,
                   const file_system::ascii_file_string_path& file_path,
                   const http::headers_t& extra_headers,
                   struct timeval* tv) {
  if (!url.is_valid() || !file_path.IsValid() || !url.SchemeIs("https")) {
    return common::make_error_inval();
  }

  HostAndPort http_server_address(url.host(), url.EffectiveIntPort());
  HttpsClient cl(http_server_address);
  ErrnoError errn = cl.Connect(tv);
  if (errn) {
    return make_error_from_errno(errn);
  }

  const auto path = url.PathForRequest();
  Error err = cl.Get(path, extra_headers);
  if (err) {
    cl.Disconnect();
    return err;
  }

  http::HttpResponse lresp;
  err = cl.ReadResponse(&lresp);
  cl.Disconnect();
  if (err) {
    return err;
  }

  errn = file_system::remove_file(file_path.GetPath());
  if (errn) {
    return make_error_from_errno(errn);
  }

  file_system::File file;
  errn = file.Open(file_path, file_system::File::FLAG_CREATE | file_system::File::FLAG_WRITE);
  if (errn) {
    return make_error_from_errno(errn);
  }

  size_t out;
  auto body = lresp.GetBody();
  errn = file.WriteBuffer(body, &out);
  file.Close();

  if (out != body.size()) {
    ignore_result(file_system::remove_file(file_path.GetPath()));
    return common::make_error("failed to save file");
  }

  if (errn) {
    ignore_result(file_system::remove_file(file_path.GetPath()));
    return make_error_from_errno(errn);
  }
  return err;
}

Error PostHttpsFile(const file_system::ascii_file_string_path& file_path,
                    const uri::GURL& url,
                    const http::headers_t& extra_headers,
                    struct timeval* tv) {
  if (!url.is_valid() || !file_path.IsValid() || !url.SchemeIs("https")) {
    return common::make_error_inval();
  }

  HostAndPort http_server_address(url.host(), url.EffectiveIntPort());
  HttpsClient cl(http_server_address);
  ErrnoError errn = cl.Connect(tv);
  if (errn) {
    return make_error_from_errno(errn);
  }

  const auto path = url.PathForRequest();
  Error err = cl.PostFile(path, file_path, extra_headers);
  if (err) {
    cl.Disconnect();
    return err;
  }

  http::HttpResponse lresp;
  err = cl.ReadResponse(&lresp);
  cl.Disconnect();
  return err;
}
#else
HttpsClient::HttpsClient(const common::net::HostAndPort& host) : base_class(new ClientSocketTcp(host)) {}

common::ErrnoError HttpsClient::Connect(struct timeval* tv) {
  ClientSocketTcp* sock = static_cast<ClientSocketTcp*>(GetSocket());
  return sock->Connect(tv);
}

bool HttpsClient::IsConnected() const {
  ClientSocketTcp* sock = static_cast<ClientSocketTcp*>(GetSocket());
  return sock->IsConnected();
}

common::ErrnoError HttpsClient::Disconnect() {
  ClientSocketTcp* sock = static_cast<ClientSocketTcp*>(GetSocket());
  return sock->Disconnect();
}

common::net::HostAndPort HttpsClient::GetHost() const {
  ClientSocketTcp* sock = static_cast<ClientSocketTcp*>(GetSocket());
  return sock->GetHost();
}

ErrnoError HttpsClient::SendFile(descriptor_t file_fd, size_t file_size) {
  ClientSocketTcp* sock = static_cast<ClientSocketTcp*>(GetSocket());
  return sock->SendFile(file_fd, file_size);
}

Error GetHttpsFile(const uri::GURL& url,
                   const file_system::ascii_file_string_path& file_path,
                   const http::headers_t& extra_headers,
                   struct timeval* tv) {
  UNUSED(extra_headers);
  UNUSED(tv);
  if (!url.is_valid() || !file_path.IsValid() || !url.SchemeIs("https")) {
    return common::make_error_inval();
  }
  return make_error("Library build without OPENSSL");
}

Error PostHttpsFile(const file_system::ascii_file_string_path& file_path,
                    const uri::GURL& url,
                    const http::headers_t& extra_headers,
                    struct timeval* tv) {
  UNUSED(extra_headers);
  UNUSED(tv);
  if (!url.is_valid() || !file_path.IsValid() || !url.SchemeIs("https")) {
    return common::make_error_inval();
  }
  return make_error("Library build without OPENSSL");
}
#endif

}  // namespace net
}  // namespace common
