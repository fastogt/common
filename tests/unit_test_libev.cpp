#include <gtest/gtest.h>

#include <common/libev/tcp/tcp_server.h>
#include <common/threads/thread_manager.h>
#include <common/utils.h>

void ExitServer(common::libev::tcp::TcpServer* ser) {
  common::utils::msleep(1000);
  ser->Stop();
}

TEST(Libev, IoServer) {
  common::net::HostAndPort hs("localhost", 8010);
  common::libev::tcp::TcpServer* serv = new common::libev::tcp::TcpServer(hs);
  common::Error err = serv->Bind();
  ASSERT_FALSE(err && err->IsError());

  err = serv->Listen(5);
  ASSERT_FALSE(err && err->IsError());

  auto tp = THREAD_MANAGER()->CreateThread(&ExitServer, serv);
  GTEST_ASSERT_EQ(tp->GetTid(), common::threads::invalid_tid);
  bool res_start = tp->Start();
  ASSERT_TRUE(res_start);

  int res_exec = serv->Exec();
  ASSERT_TRUE(res_exec == EXIT_SUCCESS);
  delete serv;
}
