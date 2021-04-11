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

#include <common/libev/io_loop.h>         // for IoLoop
#include <common/libev/tcp/tcp_client.h>  // for TcpClient
#include <common/net/socket_tcp.h>

namespace common {
namespace libev {
namespace tcp {

class TcpServer : public IoLoop {
 public:
  typedef IoLoop base_class;
  explicit TcpServer(const net::HostAndPort& host, bool is_default, IoLoopObserver* observer = nullptr);
  ~TcpServer() override;

  ErrnoError Bind(bool reuseaddr) WARN_UNUSED_RESULT;
  ErrnoError Listen(int backlog) WARN_UNUSED_RESULT;

  const char* ClassName() const override;
  net::HostAndPort GetHost() const;

  bool IsCanBeRegistered(IoClient* client) const override WARN_UNUSED_RESULT;

  using IoLoop::RegisterClient;
  IoClient* RegisterClient(const net::socket_info& info) WARN_UNUSED_RESULT;

  static IoLoop* FindExistServerByHost(const net::HostAndPort& host);

 private:
  virtual IoClient* CreateClient(const net::socket_info& info);
  IoChild* CreateChild() override;
  void PreLooped(LibEvLoop* loop) override;
  void PostLooped(LibEvLoop* loop) override;

  void Stopped(LibEvLoop* loop) override;

  static void accept_cb(LibEvLoop* loop, LibevIO* io, int revents);

  ErrnoError Accept(net::socket_info* info) WARN_UNUSED_RESULT;

  net::ServerSocketTcp sock_;
  LibevIO* accept_io_;
};

}  // namespace tcp
}  // namespace libev
}  // namespace common
