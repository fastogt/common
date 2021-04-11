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

#pragma once

#include <common/net/isocket_fd.h>
#include <common/net/types.h>

namespace common {
namespace net {

class TcpSocketHolder : public ISocketFd {
 public:
  explicit TcpSocketHolder(const socket_info& info);
  explicit TcpSocketHolder(socket_descr_t fd);

  ~TcpSocketHolder() override;

  socket_info GetInfo() const;
  void SetInfo(const socket_info& info);

  socket_descr_t GetFd() const override;
  void SetFd(socket_descr_t fd) override;

 private:
  ErrnoError WriteImpl(const void* data, size_t size, size_t* nwrite_out) override WARN_UNUSED_RESULT;
  ErrnoError ReadImpl(void* out_data, size_t max_size, size_t* nread_out) override WARN_UNUSED_RESULT;

  socket_info info_;

  DISALLOW_COPY_AND_ASSIGN(TcpSocketHolder);
};

class SocketTcp : public TcpSocketHolder {
 public:
  explicit SocketTcp(const HostAndPort& host);

  HostAndPort GetHost() const;
  void SetHost(const HostAndPort& host);

  ~SocketTcp() override;

 private:
  HostAndPort host_;

  DISALLOW_COPY_AND_ASSIGN(SocketTcp);
};

class ClientSocketTcp : public SocketTcp {
 public:
  explicit ClientSocketTcp(const HostAndPort& host);

  ErrnoError Connect(struct timeval* tv = nullptr) WARN_UNUSED_RESULT;
  ErrnoError Disconnect() WARN_UNUSED_RESULT;
  bool IsConnected() const;

 private:
  DISALLOW_COPY_AND_ASSIGN(ClientSocketTcp);
};

class ServerSocketTcp : public SocketTcp {
 public:
  explicit ServerSocketTcp(const HostAndPort& host);

  ErrnoError Bind(bool reuseaddr) WARN_UNUSED_RESULT;
  ErrnoError Listen(int backlog) WARN_UNUSED_RESULT;
  ErrnoError Accept(socket_info* info) WARN_UNUSED_RESULT;

 private:
  DISALLOW_COPY_AND_ASSIGN(ServerSocketTcp);
};

}  // namespace net
}  // namespace common
