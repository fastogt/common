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

#include <common/net/socket_tcp.h>

typedef struct ssl_st SSL;
typedef struct ssl_ctx_st SSL_CTX;

namespace common {
namespace net {

#if defined(HAVE_OPENSSL)
class TcpTlsSocketHolder : public TcpSocketHolder {
 public:
  typedef TcpSocketHolder base_class;

  explicit TcpTlsSocketHolder(const socket_info& info, SSL* ssl);
  explicit TcpTlsSocketHolder(socket_descr_t fd, SSL* ssl);

  socket_descr_t GetFd() const override;

  bool IsValid() const override;

 protected:
  void SetSSL(SSL* ssl);

  ErrnoError WriteImpl(const void* data, size_t size, size_t* nwrite_out) override;
  ErrnoError ReadImpl(void* out_data, size_t max_size, size_t* nread_out) override;

  ErrnoError SendFileImpl(descriptor_t file_fd, size_t file_size) override;

  ErrnoError CloseImpl() override;

 private:
  void SetFd(socket_descr_t fd) override;

  SSL* ssl_;

  DISALLOW_COPY_AND_ASSIGN(TcpTlsSocketHolder);
};

class SocketTcpTls : public TcpTlsSocketHolder {
 public:
  explicit SocketTcpTls(const HostAndPort& host);

  HostAndPort GetHost() const;
  void SetHost(const HostAndPort& host);

  ~SocketTcpTls() override;

 private:
  HostAndPort host_;

  DISALLOW_COPY_AND_ASSIGN(SocketTcpTls);
};

class ClientSocketTcpTls : public SocketTcpTls {
 public:
  typedef SocketTcpTls base_class;

  explicit ClientSocketTcpTls(const HostAndPort& host);

  ErrnoError Connect(struct timeval* tv = nullptr) WARN_UNUSED_RESULT;
  ErrnoError Disconnect() WARN_UNUSED_RESULT;
  bool IsConnected() const;

 private:
  DISALLOW_COPY_AND_ASSIGN(ClientSocketTcpTls);
};

class ServerSocketTcpTls : public SocketTcpTls {
 public:
  explicit ServerSocketTcpTls(const HostAndPort& host);

  bool IsValid() const override;
  socket_descr_t GetFd() const override;

  ErrnoError LoadCertificates(const std::string& cert, const std::string& key);

  ErrnoError Bind(bool reuseaddr) WARN_UNUSED_RESULT;
  ErrnoError Listen(int backlog) WARN_UNUSED_RESULT;
  ErrnoError Accept(socket_info* info, SSL** out) WARN_UNUSED_RESULT;

  ~ServerSocketTcpTls() override;

 private:
  SSL_CTX* ctx_;

  DISALLOW_COPY_AND_ASSIGN(ServerSocketTcpTls);
};

class ServerSocketEvTcpTls : public IServerSocketEv {
 public:
  ServerSocketEvTcpTls(const HostAndPort& host);

  socket_descr_t GetFd() const override;

  ErrnoError Close() override WARN_UNUSED_RESULT;

  HostAndPort GetHost() const override;

  ErrnoError Bind(bool reuseaddr) override WARN_UNUSED_RESULT;

  ErrnoError Listen(int backlog) override WARN_UNUSED_RESULT;

  ErrnoError Accept(socket_info* info, void** user) override WARN_UNUSED_RESULT;

  ErrnoError LoadCertificates(const std::string& cert, const std::string& key);

 private:
  ServerSocketTcpTls sock_;
};

#endif

}  // namespace net
}  // namespace common
