#include <gtest/gtest.h>

#include <common/libev/io_loop_observer.h>
#include <common/libev/tcp/tcp_server.h>

#include <common/libev/tcp/tcp_client.h>

#include <common/threads/thread_manager.h>

#include <common/net/net.h>

#include <common/utils.h>

namespace {
const common::net::HostAndPort g_hs("localhost", 8010);
}

class ServerHandler : public common::libev::IoLoopObserver {
 public:
  ServerHandler() {}

  ~ServerHandler() {}

  virtual void PreLooped(common::libev::IoLoop* server) override {
    common::libev::tcp::TcpServer* sserver = static_cast<common::libev::tcp::TcpServer*>(server);
    timeval tv = {1, 0};
    common::net::socket_info sc;
    ASSERT_EQ(g_hs, sserver->GetHost());
    common::ErrnoError err = common::net::connect(g_hs, common::net::ST_SOCK_STREAM, &tv, &sc);
    ASSERT_FALSE(err && err->IsError());
  }

  virtual void Accepted(common::libev::IoClient* client) override {
    common::libev::tcp::TcpServer* sserver = static_cast<common::libev::tcp::TcpServer*>(client->Server());
    std::vector<common::libev::IoClient*> cl = sserver->GetClients();
    ASSERT_TRUE(cl.empty());
  }

  virtual void Moved(common::libev::IoLoop* server, common::libev::IoClient* client) override {
    UNUSED(server);
    UNUSED(client);
  }

  virtual void Closed(common::libev::IoClient* client) override {
    common::libev::tcp::TcpServer* sserver = static_cast<common::libev::tcp::TcpServer*>(client->Server());
    std::vector<common::libev::IoClient*> cl = sserver->GetClients();
    ASSERT_EQ(cl.size(), 1);
  }

  virtual void TimerEmited(common::libev::IoLoop* server, common::libev::timer_id_t id) override {
    UNUSED(server);
    UNUSED(id);
  }

  virtual void DataReceived(common::libev::IoClient* client) override { UNUSED(client); }

  virtual void DataReadyToWrite(common::libev::IoClient* client) override { UNUSED(client); }

  virtual void PostLooped(common::libev::IoLoop* server) override {
    std::vector<common::libev::IoClient*> cl = server->GetClients();
    ASSERT_TRUE(cl.empty());
  }
};

void ExitServer(common::libev::tcp::TcpServer* ser) {
  common::utils::msleep(1000);
  ser->Stop();
}

TEST(Libev, IoServer) {
  ServerHandler hand;
  common::libev::tcp::TcpServer* serv = new common::libev::tcp::TcpServer(g_hs, &hand);
  common::Error err = serv->Bind();
  ASSERT_FALSE(err && err->IsError());

  err = serv->Listen(5);
  ASSERT_FALSE(err && err->IsError());

  auto tp = THREAD_MANAGER()->CreateThread(&ExitServer, serv);
  GTEST_ASSERT_EQ(tp->GetHandle(), common::threads::invalid_thread_handle());
  bool res_start = tp->Start();
  ASSERT_TRUE(res_start);

  int res_exec = serv->Exec();
  ASSERT_TRUE(res_exec == EXIT_SUCCESS);
  tp->Join();
  delete serv;
}
