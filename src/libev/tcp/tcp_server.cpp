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

#include <common/libev/tcp/tcp_server.h>

#include <errno.h>
#include <signal.h>
#include <stdlib.h>

#include <common/libev/default_event_loop.h>
#include <common/libev/event_io.h>
#include <common/libev/io_child.h>
#include <common/libev/io_loop.h>

namespace {

#if defined(OS_WIN)
struct WinsockInit {
  WinsockInit() {
    WSADATA d;
    WORD version = MAKEWORD(2, 2);
    int res = WSAStartup(version, &d);
    if (res != 0) {
      DEBUG_MSG_PERROR("WSAStartup", res, common::logging::LOG_LEVEL_ERR);
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
      DEBUG_MSG_PERROR("signal", errno, common::logging::LOG_LEVEL_ERR);
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
TcpServer::TcpServer(const net::HostAndPort& host, bool is_default, IoLoopObserver* observer)
    : IoLoop(is_default ? new LibEvDefaultLoop : new LibEvLoop, observer), sock_(host), accept_io_(new LibevIO) {
  accept_io_->SetUserData(this);
}

TcpServer::~TcpServer() {
  destroy(&accept_io_);
}

IoLoop* TcpServer::FindExistServerByHost(const net::HostAndPort& host) {
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

IoClient* TcpServer::RegisterClient(const net::socket_info& info) {
  IoClient* client = CreateClient(info);
  bool registered = base_class::RegisterClient(client);
  if (registered) {
    return client;
  }
  ignore_result(client->Close());
  delete client;
  return nullptr;
}

IoClient* TcpServer::CreateClient(const net::socket_info& info) {
  return new TcpClient(this, info);
}

IoChild* TcpServer::CreateChild() {
  return new IoChild(this);
}

void TcpServer::PreLooped(LibEvLoop* loop) {
  net::socket_descr_t fd = sock_.GetFd();
  bool is_inited = accept_io_->Init(loop, accept_cb, fd, EV_READ);
  if (!is_inited) {
    DNOTREACHED();
    IoLoop::PreLooped(loop);
    return;
  }

  accept_io_->Start();
  IoLoop::PreLooped(loop);
}

void TcpServer::PostLooped(LibEvLoop* loop) {
  IoLoop::PostLooped(loop);
}

void TcpServer::Stopped(LibEvLoop* loop) {
  loop->StopIO(accept_io_);
  IoLoop::Stopped(loop);

  ErrnoError err = sock_.Close();
  DCHECK(!err) << err->GetDescription();
}

ErrnoError TcpServer::Bind(bool reuseaddr) {
  return sock_.Bind(reuseaddr);
}

ErrnoError TcpServer::Listen(int backlog) {
  return sock_.Listen(backlog);
}

const char* TcpServer::ClassName() const {
  return "TcpServer";
}

net::HostAndPort TcpServer::GetHost() const {
  return sock_.GetHost();
}

bool TcpServer::IsCanBeRegistered(IoClient* client) const {
  if (!client) {
    return false;
  }
  return true;
}

ErrnoError TcpServer::Accept(net::socket_info* info) {
  return sock_.Accept(info);
}

void TcpServer::accept_cb(LibEvLoop* loop, LibevIO* io, int revents) {
  TcpServer* pserver = reinterpret_cast<TcpServer*>(io->GetUserData());
  if (!pserver) {
    DNOTREACHED();
    return;
  }

  CHECK(pserver->loop_ == loop);
  if (EV_ERROR & revents) {
    DNOTREACHED();
    return;
  }

  net::socket_info sinfo;
  ErrnoError err = pserver->Accept(&sinfo);
  if (err) {
    DNOTREACHED() << err->GetDescription();
    return;
  }

  ignore_result(pserver->RegisterClient(sinfo));
}

}  // namespace tcp
}  // namespace libev
}  // namespace common
