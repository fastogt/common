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

#include <gtest/gtest.h>

#include <common/libev/http/http_client.h>
#include <common/uri/gurl.h>

#include <common/libev/io_loop_observer.h>
#include <common/libev/tcp/tcp_client.h>
#include <common/libev/tcp/tcp_server.h>

#include <common/threads/thread_manager.h>

#include <common/net/net.h>

namespace {
const common::net::HostAndPort g_hs("localhost", 8013);
}

class ServerHandler : public common::libev::IoLoopObserver {
 public:
  ServerHandler() {}

  ~ServerHandler() {}

  void PreLooped(common::libev::IoLoop* server) override {}

  void Accepted(common::libev::IoClient* client) override {
    common::libev::tcp::TcpServer* sserver = static_cast<common::libev::tcp::TcpServer*>(client->GetServer());
    std::vector<common::libev::IoClient*> cl = sserver->GetClients();
  }

  void Moved(common::libev::IoLoop* server, common::libev::IoClient* client) override {
    UNUSED(server);
    UNUSED(client);
  }

  void Closed(common::libev::IoClient* client) override {
    common::libev::tcp::TcpServer* sserver = static_cast<common::libev::tcp::TcpServer*>(client->GetServer());
    std::vector<common::libev::IoClient*> cl = sserver->GetClients();
  }

  void TimerEmited(common::libev::IoLoop* server, common::libev::timer_id_t id) override {
    UNUSED(server);
    UNUSED(id);
  }

  void Accepted(common::libev::IoChild* child) override { UNUSED(child); }
  void Moved(common::libev::IoLoop* server, common::libev::IoChild* child) override {
    UNUSED(server);
    UNUSED(child);
  }

  void ChildStatusChanged(common::libev::IoChild* child, int status, int signal) override {
    UNUSED(child);
    UNUSED(status);
    UNUSED(signal);
  }

  void DataReceived(common::libev::IoClient* client) override { UNUSED(client); }

  void DataReadyToWrite(common::libev::IoClient* client) override { UNUSED(client); }

  void PostLooped(common::libev::IoLoop* server) override {
    std::vector<common::libev::IoClient*> cl = server->GetClients();
    ASSERT_TRUE(cl.empty());
  }
};

void ExitServer(common::libev::tcp::TcpServer* ser) {
  common::threads::PlatformThread::Sleep(1000);
  timeval tv = {1, 0};
  common::net::socket_info sc;
  ASSERT_EQ(g_hs, ser->GetHost());
  common::ErrnoError err = common::net::connect(g_hs, common::net::ST_SOCK_STREAM, &tv, &sc);
  ASSERT_FALSE(err);
  common::threads::PlatformThread::Sleep(1000);
  ser->Stop();
}

TEST(Libev, IoServer) {
  ServerHandler hand;
  auto sock = new common::net::ServerSocketEvTcp(g_hs);
  common::libev::tcp::TcpServer* serv = new common::libev::tcp::TcpServer(sock, false, &hand);
  common::ErrnoError err = serv->Bind(true);
  ASSERT_FALSE(err);

  err = serv->Listen(5);
  ASSERT_FALSE(err);

  auto tp = THREAD_MANAGER()->CreateThread(&ExitServer, serv);
  GTEST_ASSERT_EQ(tp->GetHandle(), common::threads::invalid_thread_handle());
  bool res_start = tp->Start();
  ASSERT_TRUE(res_start);

  int res_exec = serv->Exec();
  ASSERT_TRUE(res_exec == EXIT_SUCCESS);
  tp->Join();
  delete serv;
}

//

#define BUF_SIZE 4096
#define HOST "example.com"
namespace {
const common::uri::GURL http_url("http://" HOST);
const common::net::HostAndPort kHostAndPort(HOST, 80);
const common::libev::http::HttpServerInfo kHinf(PROJECT_NAME_TITLE, PROJECT_DOMAIN);
}  // namespace

class ServerWebHandler : public common::libev::IoLoopObserver {
  const common::libev::http::HttpServerInfo info_;

 public:
  explicit ServerWebHandler(const common::libev::http::HttpServerInfo& info) : info_(info) {}

  ~ServerWebHandler() {}

  const common::libev::http::HttpServerInfo& info() const { return info_; }

  void PreLooped(common::libev::IoLoop* server) override {
    common::libev::tcp::TcpServer* sserver = static_cast<common::libev::tcp::TcpServer*>(server);
    common::net::socket_info sc;
    common::ErrnoError errn = common::net::connect(kHostAndPort, common::net::ST_SOCK_STREAM, nullptr, &sc);
    ASSERT_FALSE(errn);

    common::libev::http::HttpServerClient* cl = new common::libev::http::HttpServerClient(sserver, sc);
    ignore_result(cl->SetBlocking(false));
    ignore_result(sserver->RegisterClient(cl));
    errn = cl->Get(http_url, true);
    ASSERT_FALSE(errn);
  }

  void Accepted(common::libev::IoClient* client) override {
    common::libev::tcp::TcpServer* sserver = static_cast<common::libev::tcp::TcpServer*>(client->GetServer());
    std::vector<common::libev::IoClient*> cl = sserver->GetClients();
    ASSERT_TRUE(cl.empty());
  }

  void Moved(common::libev::IoLoop* server, common::libev::IoClient* client) override {
    UNUSED(server);
    UNUSED(client);
  }

  void Closed(common::libev::IoClient* client) override {
    common::libev::tcp::TcpServer* sserver = static_cast<common::libev::tcp::TcpServer*>(client->GetServer());
    std::vector<common::libev::IoClient*> cl = sserver->GetClients();
    ASSERT_EQ(cl.size(), 1);
  }

  void TimerEmited(common::libev::IoLoop* server, common::libev::timer_id_t id) override {
    UNUSED(server);
    UNUSED(id);
  }

  void Accepted(common::libev::IoChild* child) override { UNUSED(child); }
  void Moved(common::libev::IoLoop* server, common::libev::IoChild* child) override {
    UNUSED(server);
    UNUSED(child);
  }

  void ChildStatusChanged(common::libev::IoChild* child, int status, int signal) override {
    UNUSED(child);
    UNUSED(status);
    UNUSED(signal);
  }

  void DataReceived(common::libev::IoClient* client) override {
    char buff[BUF_SIZE] = {0};
    size_t nread = 0;
    common::ErrnoError errn = client->Read(buff, BUF_SIZE, &nread);
    if ((errn && errn->GetErrorCode() != EAGAIN) || nread == 0) {
      ignore_result(client->Close());
      delete client;
      return;
    }

    common::libev::http::HttpServerClient* hclient = dynamic_cast<common::libev::http::HttpServerClient*>(client);
    CHECK(hclient);
    common::http::HttpResponse resp;
    size_t not_parsed;
    common::Error err = common::http::parse_http_response(std::string(buff, nread), &resp, &not_parsed);
    ASSERT_FALSE(err);
  }

  void DataReadyToWrite(common::libev::IoClient* client) override { UNUSED(client); }

  void PostLooped(common::libev::IoLoop* server) override {
    std::vector<common::libev::IoClient*> cl = server->GetClients();
    ASSERT_TRUE(cl.empty());
  }
};

void ExitHttpServer(common::libev::tcp::TcpServer* ser) {
  common::threads::PlatformThread::Sleep(1000);
  ser->Stop();
}

TEST(Libev, Http) {
  ServerWebHandler hand(kHinf);
  auto sock = new common::net::ServerSocketEvTcp(g_hs);
  common::libev::tcp::TcpServer* serv = new common::libev::tcp::TcpServer(sock, false, &hand);
  common::ErrnoError err = serv->Bind(true);
  ASSERT_FALSE(err);

  err = serv->Listen(5);
  ASSERT_FALSE(err);

  auto tp = THREAD_MANAGER()->CreateThread(&ExitHttpServer, serv);
  GTEST_ASSERT_EQ(tp->GetHandle(), common::threads::invalid_thread_handle());
  bool res_start = tp->Start();
  ASSERT_TRUE(res_start);

  int res_exec = serv->Exec();
  ASSERT_TRUE(res_exec == EXIT_SUCCESS);
  tp->Join();
  delete serv;
}
