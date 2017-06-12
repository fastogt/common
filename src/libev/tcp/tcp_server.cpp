/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

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

#include <common/libev/tcp/tcp_server.h>

#include <errno.h>   // for errno
#include <signal.h>  // for signal, SIGPIPE, SIG_ERR
#include <stdlib.h>  // for exit, EXIT_FAILURE

#include <common/logger.h>  // for COMPACT_LOG_FILE_CRIT

#include <common/error.h>   // for Error, DEBUG_MSG_ERROR
#include <common/macros.h>  // for CHECK, DNOTREACHED

#include <common/net/types.h>  // for HostAndPort, operator==

#include <common/libev/event_io.h>        // for LibevIO
#include <common/libev/event_loop.h>      // for LibEvLoop
#include <common/libev/io_loop.h>         // for IoLoop
#include <common/libev/tcp/tcp_client.h>  // for TcpClient
#include <common/libev/types.h>           // for ::EV_ERROR

namespace common {
namespace libev {
class IoClient;
}
}  // namespace common
namespace common {
namespace libev {
class IoLoopObserver;
}
}  // namespace common

namespace {

#ifdef OS_WIN
struct WinsockInit {
  WinsockInit() {
    WSADATA d;
    int res = WSAStartup(MAKEWORD(2, 2), &d);
    if (res != 0) {
      DEBUG_MSG_PERROR("WSAStartup", res);
      exit(EXIT_FAILURE);
    }
  }
  ~WinsockInit() { WSACleanup(); }
} winsock_init;
#endif
struct SigIgnInit {
  SigIgnInit() {
#if defined(COMPILER_MINGW)
#elif defined(COMPILER_MSVC)
#else
    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR) {
      DEBUG_MSG_PERROR("signal", errno);
      exit(EXIT_FAILURE);
    }
#endif
  }
} sig_init;

}  // namespace

namespace common {
namespace libev {
namespace tcp {

// server
TcpServer::TcpServer(const common::net::HostAndPort& host, IoLoopObserver* observer)
    : IoLoop(observer), sock_(host), accept_io_(new LibevIO) {
  accept_io_->SetUserData(this);
}

TcpServer::~TcpServer() {
  destroy(&accept_io_);
}

IoLoop* TcpServer::FindExistServerByHost(const common::net::HostAndPort& host) {
  if (!host.IsValid()) {
    return nullptr;
  }

  auto find_by_host = [host](IoLoop* loop) -> bool {
    TcpServer* server = static_cast<TcpServer*>(loop);
    if (!server) {
      return false;
    }

    return server->GetHost() == host;
  };

  return FindExistLoopByPredicate(find_by_host);
}

TcpClient* TcpServer::CreateClient(const common::net::socket_info& info) {
  return new TcpClient(this, info);
}

void TcpServer::PreLooped(LibEvLoop* loop) {
  net::socket_descr_t fd = sock_.GetFd();
  accept_io_->Init(loop, accept_cb, fd, EV_READ);
  accept_io_->Start();
  IoLoop::PreLooped(loop);
}

void TcpServer::PostLooped(LibEvLoop* loop) {
  IoLoop::PostLooped(loop);
}

void TcpServer::Stoped(LibEvLoop* loop) {
  common::Error err = sock_.Close();
  if (err && err->IsError()) {
    DEBUG_MSG_ERROR(err);  // FIX ME, don't write to log
  }

  loop->StopIO(accept_io_);
  IoLoop::Stoped(loop);
}

common::Error TcpServer::Bind() {
  return sock_.Bind();
}

common::Error TcpServer::Listen(int backlog) {
  return sock_.Listen(backlog);
}

const char* TcpServer::ClassName() const {
  return "TcpServer";
}

common::net::HostAndPort TcpServer::GetHost() const {
  return sock_.GetHost();
}

common::Error TcpServer::Accept(common::net::socket_info* info) {
  return sock_.Accept(info);
}

void TcpServer::accept_cb(LibEvLoop* loop, LibevIO* io, int revents) {
  TcpServer* pserver = reinterpret_cast<TcpServer*>(io->UserData());
  CHECK(pserver && pserver->loop_ == loop);

  if (EV_ERROR & revents) {
    DNOTREACHED();
    return;
  }

  common::net::socket_info sinfo;
  common::Error err = pserver->Accept(&sinfo);

  if (err && err->IsError()) {
    DEBUG_MSG_ERROR(err);  // FIX ME, don't write to log
    return;
  }

  IoClient* pclient = pserver->CreateClient(sinfo);
  pserver->RegisterClient(pclient);
}

}  // namespace tcp
}  // namespace libev
}  // namespace common
