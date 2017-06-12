#include <gtest/gtest.h>

#include <common/net/net.h>
#include <common/net/socket_tcp.h>
#include <common/threads/thread_manager.h>

void exec_serv(common::net::ServerSocketTcp* serv) {
  common::net::socket_info inf;
  common::Error err = serv->Accept(&inf);
  ASSERT_FALSE(err && err->IsError());
}

TEST(ServerSocketTcpAndClientSocketTcp, workflow) {
  using namespace common::net;
  HostAndPort host("localhost", 4567);
  ServerSocketTcp serv(host);
  common::ErrnoError err = serv.Bind();
  ASSERT_FALSE(err && err->IsError());
  err = serv.Listen(5);
  ASSERT_FALSE(err && err->IsError());

  ClientSocketTcp tcp(host);
  socket_info inf = tcp.GetInfo();
  ASSERT_TRUE(inf.addr_info() == NULL);
  auto ex_handler = THREAD_MANAGER()->CreateThread(&exec_serv, &serv);
  ex_handler->Start();
  sleep(1);
  err = tcp.Connect();
  ASSERT_FALSE(err && err->IsError());
  inf = tcp.GetInfo();
  ASSERT_FALSE(inf.addr_info() == NULL);
  ex_handler->Join();
}

TEST(SocketTcp, bindRandomWorkflow) {
  using namespace common::net;
  HostAndPort host("localhost", RANDOM_PORT);
  ServerSocketTcp serv(host);

  common::ErrnoError err = serv.Bind();
  ASSERT_FALSE(err && err->IsError());
  err = serv.Listen(5);
  ASSERT_FALSE(err && err->IsError());

  HostAndPort chost = serv.GetHost();
  ASSERT_FALSE(chost == host);

  ClientSocketTcp tcp(chost);
  socket_info inf = tcp.GetInfo();
  ASSERT_TRUE(inf.addr_info() == NULL);
  auto ex_handler = THREAD_MANAGER()->CreateThread(&exec_serv, &serv);
  ex_handler->Start();
  sleep(1);
  err = tcp.Connect();
  ASSERT_FALSE(err && err->IsError());
  inf = tcp.GetInfo();
  ASSERT_FALSE(inf.addr_info() == NULL);
  ex_handler->Join();
}
