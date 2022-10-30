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

#include <common/net/socket_tcp_tls.h>

#if defined(HAVE_OPENSSL)
#include <openssl/err.h>
#include <openssl/ssl.h>
#endif

#if defined(HAVE_OPENSSL)
#include <common/net/net.h>  // for bind, accept, close, etc

#define BUF_SIZE 8192

namespace {

SSL_CTX* InitContext() {
  SSL_library_init();
  SSLeay_add_ssl_algorithms();
  SSL_load_error_strings();

#if OPENSSL_VERSION_NUMBER < 0x10100000L
  const SSL_METHOD* method = TLSv1_2_client_method();
#else
  const SSL_METHOD* method = TLS_client_method();
#endif
  if (!method) {
    return nullptr;
  }

  return SSL_CTX_new(method);
}

SSL_CTX* InitServerContext() {
  SSL_CTX* ctx = InitContext();
  if (!ctx) {
    return nullptr;
  }

  SSL_CTX_set_cipher_list(ctx, "ALL:eNULL");
  return ctx;
}

bool LoadCertificatesContext(SSL_CTX* ctx, const std::string& cert, const std::string& key) {
  if (cert.empty() || key.empty()) {
    return false;
  }

  const char* c = cert.c_str();
  const char* k = key.c_str();
  // New lines
  if (SSL_CTX_load_verify_locations(ctx, c, k) != 1) {
    return false;
  }

  if (SSL_CTX_set_default_verify_paths(ctx) != 1) {
    return false;
  }

  /* set the local certificate from CertFile */
  if (SSL_CTX_use_certificate_file(ctx, c, SSL_FILETYPE_PEM) <= 0) {
    return false;
  }

  /* set the private key from KeyFile (may be the same as CertFile) */
  if (SSL_CTX_use_PrivateKey_file(ctx, k, SSL_FILETYPE_PEM) <= 0) {
    return false;
  }

  /* verify private key */
  if (!SSL_CTX_check_private_key(ctx)) {
    return false;
  }

  return true;
}

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
TcpTlsSocketHolder::TcpTlsSocketHolder(const socket_info& info, SSL* ssl) : base_class(info), ssl_(ssl) {}

TcpTlsSocketHolder::TcpTlsSocketHolder(socket_descr_t fd, SSL* ssl) : base_class(fd), ssl_(ssl) {}

bool TcpTlsSocketHolder::IsValid() const {
  return ssl_ != nullptr && IsValid();
}

common::net::socket_descr_t TcpTlsSocketHolder::GetFd() const {
  if (!ssl_) {
    return INVALID_SOCKET_VALUE;
  }
  return base_class::GetFd();
}

void TcpTlsSocketHolder::SetFd(socket_descr_t fd) {
  base_class::SetFd(fd);
}

void TcpTlsSocketHolder::SetSSL(SSL* ssl) {
  ssl_ = ssl;
}

common::ErrnoError TcpTlsSocketHolder::WriteImpl(const void* data, size_t size, size_t* nwrite_out) {
  return SSLWrite(ssl_, data, size, nwrite_out);
}

common::ErrnoError TcpTlsSocketHolder::ReadImpl(void* out_data, size_t max_size, size_t* nread_out) {
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

ErrnoError TcpTlsSocketHolder::SendFileImpl(descriptor_t file_fd, size_t file_size) {
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

common::ErrnoError TcpTlsSocketHolder::CloseImpl() {
  if (ssl_) {
    SSL_free(ssl_);
    ssl_ = nullptr;
  }

  return base_class::CloseImpl();
}

SocketTcpTls::SocketTcpTls(const HostAndPort& host) : TcpTlsSocketHolder(INVALID_SOCKET_VALUE, nullptr), host_(host) {}

HostAndPort SocketTcpTls::GetHost() const {
  return host_;
}

void SocketTcpTls::SetHost(const HostAndPort& host) {
  host_ = host;
}

SocketTcpTls::~SocketTcpTls() {}

ClientSocketTcpTls::ClientSocketTcpTls(const HostAndPort& host) : base_class(host) {}

common::ErrnoError ClientSocketTcpTls::Connect(struct timeval* tv) {
  common::net::ClientSocketTcp hs(GetHost());
  common::ErrnoError err = hs.Connect(tv);
  if (err) {
    return err;
  }

  auto ctx = InitContext();
  if (!ctx) {
    ignore_result(hs.Disconnect());
    return common::make_errno_error_inval();
  }

  SSL* ssl = SSL_new(ctx);
  SSL_CTX_free(ctx);
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

  SetInfo(hs.GetInfo());
  SetSSL(ssl);
  return common::ErrnoError();
}

common::ErrnoError ClientSocketTcpTls::Disconnect() {
  return Close();
}

bool ClientSocketTcpTls::IsConnected() const {
  return IsValid();
}

ServerSocketTcpTls::ServerSocketTcpTls(const HostAndPort& host) : SocketTcpTls(host), ctx_(nullptr) {}

ErrnoError ServerSocketTcpTls::LoadCertificates(const std::string& cert, const std::string& key) {
  auto ctx = InitServerContext();
  if (!ctx) {
    return common::make_errno_error("Failed to init ssl context", EAGAIN);
  }

  if (!LoadCertificatesContext(ctx, cert, key)) {
    return common::make_errno_error("Failed to setup key/cert for ssl context", EAGAIN);
  }

  return ErrnoError();
}

ErrnoError ServerSocketTcpTls::Bind(bool reuseaddr) {
  socket_info linfo;
  const HostAndPort hs = GetHost();
  ErrnoError err = resolve(hs, ST_SOCK_STREAM, &linfo);  // init fd
  if (err) {
    return err;
  }

  socket_descr_t fd = linfo.fd();
  addrinfo* ainf = linfo.addr_info();
  socket_info lbinfo;
  err = bind(fd, ainf, reuseaddr, &lbinfo);  // init sockaddr
  if (err) {
    return err;
  }

  bool is_random_port = linfo.port() == 0;
  if (is_random_port) {                    // random port
    err = getsockname(fd, ainf, &lbinfo);  // init sockaddr
    if (err) {
      return err;
    }

    HostAndPort new_hs = hs;
    uint16_t port = 0;
    ErrnoError errn = get_in_port(lbinfo.addr_info(), &port);
    if (!errn) {
      new_hs.SetPort(port);
    }

    SetInfo(lbinfo);
    SetHost(new_hs);
    return ErrnoError();
  }

  SetInfo(lbinfo);
  return ErrnoError();
}

ErrnoError ServerSocketTcpTls::Listen(int backlog) {
  DCHECK(IsValid());
  return listen(GetInfo(), backlog);
}

ErrnoError ServerSocketTcpTls::Accept(socket_info* info, SSL** out) {
  DCHECK(IsValid());
  ErrnoError err = accept(GetInfo(), info);
  if (err) {
    return err;
  }

  auto ssl = SSL_new(ctx_);
  SSL_set_fd(ssl, info->fd());
  int e = SSL_accept(ssl);
  if (e < 0) {
    int err = SSL_get_error(ssl, e);
    char* str = ERR_error_string(err, nullptr);
    SSL_free(ssl);
    return common::make_errno_error(str, EINTR);
  }

  *out = ssl;
  return ErrnoError();
}

ServerSocketTcpTls::~ServerSocketTcpTls() {
  if (ctx_) {
    SSL_CTX_free(ctx_);
    ctx_ = nullptr;
  }
}

ServerSocketEvTcpTls::ServerSocketEvTcpTls(const HostAndPort& host) : sock_(host) {}

socket_descr_t ServerSocketEvTcpTls::GetFd() const {
  return sock_.GetFd();
}

ErrnoError ServerSocketEvTcpTls::Close() {
  return sock_.Close();
}

HostAndPort ServerSocketEvTcpTls::GetHost() const {
  return sock_.GetHost();
}

ErrnoError ServerSocketEvTcpTls::Bind(bool reuseaddr) {
  return sock_.Bind(reuseaddr);
}

ErrnoError ServerSocketEvTcpTls::Listen(int backlog) {
  return sock_.Listen(backlog);
}

ErrnoError ServerSocketEvTcpTls::Accept(socket_info* info, void** user) {
  SSL* out = nullptr;
  ErrnoError err = sock_.Accept(info, &out);
  if (err) {
    return err;
  }
  *user = out;
  return ErrnoError();
}

ErrnoError ServerSocketEvTcpTls::LoadCertificates(const std::string& cert, const std::string& key) {
  return sock_.LoadCertificates(cert, key);
}

#endif

}  // namespace net
}  // namespace common
