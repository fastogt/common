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

#include <common/net/http_client.h>

#include <string.h>
#include <unistd.h>

#if defined(HAVE_OPENSSL)
#include <openssl/err.h>
#include <openssl/ssl.h>
#endif

#include <common/net/socket_tcp.h>

#include <common/convert2string.h>
#include <common/file_system/file.h>
#include <common/file_system/file_system.h>
#include <common/http/http_chunked_decoder.h>

#undef SetPort

#define BUF_SIZE 8192

#if defined(HAVE_OPENSSL)
namespace {

common::ErrnoError SSLWrite(SSL* ssl, const void* data, size_t size, size_t* nwrite_out) {
  int len = SSL_write(ssl, data, size);
  if (len < 0) {
    int err = SSL_get_error(ssl, len);
    char* str = ERR_error_string(err, nullptr);
    return common::make_errno_error(str, EINTR);
  }

  *nwrite_out = len;
  return common::ErrnoError();
}

ssize_t sendfilessl(SSL* ssl, descriptor_t in_fd, off_t* offset, size_t count) {
  off_t orig = 0;
  char buf[BUF_SIZE] = {0};

  if (offset) {
    /* Save current file offset and set offset to value in '*offset' */

    orig = lseek(in_fd, 0, SEEK_CUR);
    if (orig == -1) {
      return ERROR_RESULT_VALUE;
    }
    if (lseek(in_fd, *offset, SEEK_SET) == -1) {
      return ERROR_RESULT_VALUE;
    }
  }

  ssize_t totSent = 0;

  while (count > 0) {
    ssize_t num_read = read(in_fd, buf, BUF_SIZE);
    if (num_read == ERROR_RESULT_VALUE) {
      return ERROR_RESULT_VALUE;
    }
    if (num_read == 0) {
      break; /* EOF */
    }

    size_t numSent = 0;
    common::ErrnoError err = SSLWrite(ssl, buf, static_cast<size_t>(num_read), &numSent);
    if (err) {
      return ERROR_RESULT_VALUE;
    }

    if (numSent == 0) {
      return ERROR_RESULT_VALUE;
    }

    count -= numSent;
    totSent += numSent;
  }

  if (offset) {
    /* Return updated file offset in '*offset', and reset the file offset
       to the value it had when we were called. */

    *offset = lseek(in_fd, 0, SEEK_CUR);
    if (*offset == -1) {
      return ERROR_RESULT_VALUE;
    }
    if (lseek(in_fd, orig, SEEK_SET) == -1) {
      return ERROR_RESULT_VALUE;
    }
  }

  return totSent;
}

}  // namespace
#endif

namespace common {
namespace net {

#if defined(HAVE_OPENSSL)
namespace {
class SocketTls : public common::net::ISocketFd {
 public:
  explicit SocketTls(const common::net::HostAndPort& host) : hs_(host), ssl_(nullptr) {
    SSL_library_init();
    SSLeay_add_ssl_algorithms();
    SSL_load_error_strings();
  }

  common::net::socket_descr_t GetFd() const override {
    if (!ssl_) {
      return INVALID_SOCKET_VALUE;
    }
    return hs_.GetFd();
  }

  void SetFd(common::net::socket_descr_t fd) override { hs_.SetFd(fd); }

  common::ErrnoError Connect(struct timeval* tv = nullptr) WARN_UNUSED_RESULT {
    common::net::ClientSocketTcp hs(hs_.GetHost());
    common::ErrnoError err = hs.Connect(tv);
    if (err) {
      return err;
    }

#if OPENSSL_VERSION_NUMBER < 0x10100000L
    const SSL_METHOD* method = TLSv1_2_client_method();
#else
    const SSL_METHOD* method = TLS_client_method();
#endif
    if (!method) {
      hs.Disconnect();
      return common::make_errno_error_inval();
    }

    SSL_CTX* ctx = SSL_CTX_new(method);
    if (!ctx) {
      ignore_result(hs.Disconnect());
      return common::make_errno_error_inval();
    }

    SSL* ssl = SSL_new(ctx);
    SSL_CTX_free(ctx);
    ctx = nullptr;
    if (!ssl) {
      SSL_free(ssl);
      ignore_result(hs.Disconnect());
      return common::make_errno_error_inval();
    }

    SSL_set_fd(ssl, hs.GetFd());
    int e = SSL_connect(ssl);
    if (e < 0) {
      int err = SSL_get_error(ssl, e);
      char* str = ERR_error_string(err, nullptr);
      SSL_free(ssl);
      ignore_result(hs.Disconnect());
      return common::make_errno_error(str, EINTR);
    }

    X509* cert = SSL_get_peer_certificate(ssl);
    if (cert == NULL) {
      SSL_free(ssl);
      ignore_result(hs.Disconnect());
      return common::make_errno_error("Could not get a certificate", EINTR);
    }

    hs_.SetInfo(hs.GetInfo());
    ssl_ = ssl;
    return common::ErrnoError();
  }

  common::ErrnoError Disconnect() WARN_UNUSED_RESULT { return Close(); }

  bool IsConnected() const { return hs_.IsConnected(); }

  common::net::HostAndPort GetHost() const { return hs_.GetHost(); }

  bool IsValid() const override { return hs_.IsValid(); }

 private:
  common::ErrnoError WriteImpl(const void* data, size_t size, size_t* nwrite_out) override {
    return SSLWrite(ssl_, data, size, nwrite_out);
  }

  common::ErrnoError ReadImpl(void* out_data, size_t max_size, size_t* nread_out) override {
    int len = SSL_read(ssl_, out_data, max_size);
    if (len < 0) {
      int err = SSL_get_error(ssl_, len);
      char* str = ERR_error_string(err, nullptr);
      return common::make_errno_error(str, EINTR);
    }

    if (len == 0) {
      return make_errno_error(ECONNRESET);
    }

    *nread_out = len;
    return common::ErrnoError();
  }

  ErrnoError SendFileImpl(descriptor_t file_fd, size_t file_size) override {
    off_t offset = 0;
    for (size_t size_to_send = file_size; size_to_send > 0;) {
      off_t off = offset;
      ssize_t sent = sendfilessl(ssl_, file_fd, &off, size_to_send);
      if (sent == ERROR_RESULT_VALUE) {
        return make_error_perror("sendfile", errno);
      } else if (sent == 0) {
        return ErrnoError();
      }

      offset += sent;
      size_to_send -= sent;
    }

    return ErrnoError();
  }

  common::ErrnoError CloseImpl() override {
    if (ssl_) {
      SSL_free(ssl_);
      ssl_ = nullptr;
    }

    return hs_.Close();
  }

  common::net::ClientSocketTcp hs_;
  SSL* ssl_;
};
}  // namespace
#endif

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
  if (!url.is_valid() || !file_path.IsValid() || !url.SchemeIs("http")) {
    return common::make_error_inval();
  }

  HostAndPort http_server_address(url.host(), url.EffectiveIntPort());
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
  cl.Disconnect();
  return err;
}

#if defined(HAVE_OPENSSL)
HttpsClient::HttpsClient(const common::net::HostAndPort& host) : base_class(new SocketTls(host)) {}

common::ErrnoError HttpsClient::Connect(struct timeval* tv) {
  SocketTls* sock = static_cast<SocketTls*>(GetSocket());
  return sock->Connect(tv);
}

bool HttpsClient::IsConnected() const {
  SocketTls* sock = static_cast<SocketTls*>(GetSocket());
  return sock->IsConnected();
}

common::ErrnoError HttpsClient::Disconnect() {
  SocketTls* sock = static_cast<SocketTls*>(GetSocket());
  return sock->Disconnect();
}

common::net::HostAndPort HttpsClient::GetHost() const {
  SocketTls* sock = static_cast<SocketTls*>(GetSocket());
  return sock->GetHost();
}

ErrnoError HttpsClient::SendFile(descriptor_t file_fd, size_t file_size) {
  SocketTls* sock = static_cast<SocketTls*>(GetSocket());
  return sock->SendFile(file_fd, file_size);
}

Error PostHttpsFile(const file_system::ascii_file_string_path& file_path, const uri::GURL& url) {
  if (!url.is_valid() || !file_path.IsValid() || !url.SchemeIs("https")) {
    return common::make_error_inval();
  }

  HostAndPort http_server_address(url.host(), url.EffectiveIntPort());
  HttpsClient cl(http_server_address);
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
  cl.Disconnect();
  return err;
}

#endif

}  // namespace net
}  // namespace common
