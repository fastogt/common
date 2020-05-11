/*  Copyright (C) 2014-2020 FastoGT. All right reserved.

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

#include <common/file_system/file.h>
#include <common/file_system/file_system.h>

#define USER_AGENT_VALUE PROJECT_COMPANYNAME "/" PROJECT_NAME "/" PROJECT_VERSION

namespace common {
namespace net {

namespace {
bool GetHttpHostAndPort(const std::string& host, HostAndPort* out) {
  if (host.empty() || !out) {
    return false;
  }

  HostAndPort http_server;
  size_t del = host.find_last_of(':');
  if (del != std::string::npos) {
    http_server.SetHost(host.substr(0, del));
    std::string port_str = host.substr(del + 1);
    uint16_t lport;
    if (ConvertFromString(port_str, &lport)) {
      http_server.SetPort(lport);
    }
  } else {
    http_server.SetHost(host);
    http_server.SetPort(80);
  }
  *out = http_server;
  return true;
}

bool GetPostServerFromUrl(const uri::GURL& url, HostAndPort* out) {
  if (!url.is_valid() || !out) {
    return false;
  }

  const std::string host_str = url.HostNoBrackets();
  return GetHttpHostAndPort(host_str, out);
}
}  // namespace

Error IHttpClient::PostFile(const url_t& path, const file_system::ascii_file_string_path& file_path) {
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

  auto req = http::MakePostRequest(path, http::HP_1_1, {header, type, disp, cont, user});
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

Error IHttpClient::Get(const url_t& path) {
  const HostAndPort hs = GetHost();
  http::HttpHeader header("Host", ConvertToString(hs));
  http::HttpHeader user("User-Agent", USER_AGENT_VALUE);
  auto req = http::MakeGetRequest(path, http::HP_1_1, {header, user});
  if (!req) {
    return make_error("Can't make request.");
  }
  return SendRequest(*req);
}

Error IHttpClient::Head(const url_t& path) {
  const HostAndPort hs = GetHost();
  http::HttpHeader header("Host", ConvertToString(hs));
  http::HttpHeader user("User-Agent", USER_AGENT_VALUE);
  auto req = http::MakeHeadRequest(path, http::HP_1_1, {header, user});
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

  static const size_t kHeaderBufInitialSize = 4 * 1024;  // 4K
  char* data_head = new char[kHeaderBufInitialSize];
  size_t nread_head;
  ErrnoError err = sock_->Read(data_head, kHeaderBufInitialSize, &nread_head);
  if (err) {
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

  http::header_t cont;
  if (!response->FindHeaderByKey("Content-Length", false, &cont)) {  // try to get body
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
    response->SetBody(body);
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

IHttpClient::IHttpClient(ISocket* sock) : sock_(sock) {
  CHECK(sock) << "Socket must be passed!";
}

ISocket* IHttpClient::GetSocket() const {
  return sock_;
}

HttpClient::HttpClient(const HostAndPort& host) : IHttpClient(new ClientSocketTcp(host)) {}

ErrnoError HttpClient::Connect(struct timeval* tv) {
  ClientSocketTcp* sock = static_cast<ClientSocketTcp*>(GetSocket());
  return sock->Connect(tv);
}

bool HttpClient::IsConnected() const {
  ClientSocketTcp* sock = static_cast<ClientSocketTcp*>(GetSocket());
  return sock->IsConnected();
}

ErrnoError HttpClient::Disconnect() {
  ClientSocketTcp* sock = static_cast<ClientSocketTcp*>(GetSocket());
  return sock->Disconnect();
}

ErrnoError HttpClient::SendFile(descriptor_t file_fd, size_t file_size) {
  ClientSocketTcp* sock = static_cast<ClientSocketTcp*>(GetSocket());
  return sock->SendFile(file_fd, file_size);
}

HostAndPort HttpClient::GetHost() const {
  ClientSocketTcp* sock = static_cast<ClientSocketTcp*>(GetSocket());
  return sock->GetHost();
}

Error PostHttpFile(const file_system::ascii_file_string_path& file_path, const uri::GURL& url) {
  HostAndPort http_server_address;
  if (!GetPostServerFromUrl(url, &http_server_address)) {
    return make_error_inval();
  }

  HttpClient cl(http_server_address);
  ErrnoError errn = cl.Connect();
  if (errn) {
    return make_error_from_errno(errn);
  }

  const auto path = url.PathForRequest();
  Error err = cl.PostFile(path, file_path);
  if (err) {
    cl.Disconnect();
    return err;
  }

  http::HttpResponse lresp;
  err = cl.ReadResponse(&lresp);
  if (err) {
    cl.Disconnect();
    return err;
  }

  if (lresp.IsEmptyBody()) {
    cl.Disconnect();
    return make_error("Empty body");
  }

  cl.Disconnect();
  return Error();
}

}  // namespace net
}  // namespace common
