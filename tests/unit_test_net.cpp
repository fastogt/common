#include <gtest/gtest.h>

#include <common/net/net.h>
#include <common/net/socket_tcp.h>
#include <common/threads/thread_manager.h>

void exec_serv(common::net::ServerSocketTcp* serv) {
  common::net::socket_info inf;
  common::Error err = serv->Accept(&inf);
  ASSERT_FALSE(err && err->IsError());
}

TEST(HostAndPort, methods) {
  common::net::HostAndPort invalid;
  ASSERT_FALSE(invalid.IsValid());
  ASSERT_FALSE(invalid.IsLocalHost());

  const common::net::HostAndPort local_host = common::net::HostAndPort::CreateLocalHost(RANDOM_PORT);
  ASSERT_TRUE(local_host.IsValid());
  ASSERT_TRUE(local_host.IsLocalHost());
  ASSERT_EQ(local_host.GetPort(), RANDOM_PORT);

  const common::net::HostAndPort local_host2("Localhost", RANDOM_PORT);
  ASSERT_TRUE(local_host2.IsValid());
  ASSERT_TRUE(local_host2.IsLocalHost());
  ASSERT_EQ(local_host2.GetPort(), RANDOM_PORT);
  ASSERT_EQ(local_host, local_host2);

  const uint16_t valid_port = 8080;
  const common::net::HostAndPort valid_host = common::net::HostAndPort("192.168.1.2", valid_port);
  ASSERT_TRUE(valid_host.IsValid());
  ASSERT_FALSE(valid_host.IsLocalHost());
  ASSERT_EQ(valid_host.GetPort(), valid_port);
}

TEST(HostAndPort, ConvertToString) {
  const uint16_t valid_port = 8080;
  const std::string host_str = common::MemSPrintf("localhost:%u", valid_port);
  common::net::HostAndPort local_host;
  ASSERT_TRUE(common::ConvertFromString(host_str, &local_host));
  ASSERT_TRUE(local_host.IsValid());
  ASSERT_TRUE(local_host.IsLocalHost());
  ASSERT_EQ(local_host.GetPort(), valid_port);
  ASSERT_EQ(host_str, common::ConvertToString(local_host));

  const std::string host_str2 = common::MemSPrintf("127.0.0.1:%u", valid_port);
  ASSERT_TRUE(common::ConvertFromString(host_str2, &local_host));
  ASSERT_TRUE(local_host.IsValid());
  ASSERT_TRUE(local_host.IsLocalHost());
  ASSERT_EQ(local_host.GetPort(), valid_port);
  ASSERT_EQ(host_str2, common::ConvertToString(local_host));
}

TEST(HostAndPortAndSlot, methods) {
  common::net::HostAndPortAndSlot invalid;
  ASSERT_FALSE(invalid.IsValid());
  ASSERT_FALSE(invalid.IsLocalHost());

  const uint16_t valid_port = 8080;
  const uint16_t valid_slot = 100;
  const common::net::HostAndPortAndSlot valid_host_slot =
      common::net::HostAndPortAndSlot("192.168.1.2", valid_port, valid_slot);
  ASSERT_TRUE(valid_host_slot.IsValid());
  ASSERT_FALSE(valid_host_slot.IsLocalHost());
  ASSERT_EQ(valid_host_slot.GetPort(), valid_port);
  ASSERT_EQ(valid_host_slot.GetSlot(), valid_slot);
}

TEST(HostAndPortAndSlot, ConvertToString) {
  const uint16_t valid_port = 8080;
  const uint16_t valid_slot = 100;
  const std::string host_str = common::MemSPrintf("localhost:%u@%u", valid_port, valid_slot);
  common::net::HostAndPortAndSlot local_host;
  ASSERT_TRUE(common::ConvertFromString(host_str, &local_host));
  ASSERT_TRUE(local_host.IsValid());
  ASSERT_TRUE(local_host.IsLocalHost());
  ASSERT_EQ(local_host.GetPort(), valid_port);
  ASSERT_EQ(local_host.GetSlot(), valid_slot);
  ASSERT_EQ(host_str, common::ConvertToString(local_host));

  const std::string host_str2 = common::MemSPrintf("127.0.0.1:%u@%u", valid_port, valid_slot);
  ASSERT_TRUE(common::ConvertFromString(host_str2, &local_host));
  ASSERT_TRUE(local_host.IsValid());
  ASSERT_TRUE(local_host.IsLocalHost());
  ASSERT_EQ(local_host.GetPort(), valid_port);
  ASSERT_EQ(host_str2, common::ConvertToString(local_host));
}

TEST(ServerSocketTcpAndClientSocketTcp, workflow) {
  using namespace common::net;
  HostAndPort host("localhost", 4567);
  ServerSocketTcp serv(host);
  common::ErrnoError err = serv.Bind(true);
  ASSERT_FALSE(err && err->IsError());
  err = serv.Listen(5);
  ASSERT_FALSE(err && err->IsError());

  ClientSocketTcp tcp(host);
  socket_info inf = tcp.GetInfo();
  ASSERT_TRUE(inf.addr_info() == NULL);
  auto ex_handler = THREAD_MANAGER()->CreateThread(&exec_serv, &serv);
  bool res = ex_handler->Start();
  DCHECK(res);
  sleep(1);
  err = tcp.Connect();
  ASSERT_FALSE(err && err->IsError());
  inf = tcp.GetInfo();
  ASSERT_FALSE(inf.addr_info() == NULL);
  ex_handler->Join();
  err = tcp.Close();
  ASSERT_FALSE(err && err->IsError());
  err = serv.Close();
  ASSERT_FALSE(err && err->IsError());
}

TEST(SocketTcp, bindRandomWorkflow) {
  using namespace common::net;
  HostAndPort host("localhost", RANDOM_PORT);
  ServerSocketTcp serv(host);

  common::ErrnoError err = serv.Bind(false);
  ASSERT_FALSE(err && err->IsError());
  err = serv.Listen(5);
  ASSERT_FALSE(err && err->IsError());

  HostAndPort chost = serv.GetHost();
  ASSERT_FALSE(chost == host);

  ClientSocketTcp tcp(chost);
  socket_info inf = tcp.GetInfo();
  ASSERT_TRUE(inf.addr_info() == NULL);
  auto ex_handler = THREAD_MANAGER()->CreateThread(&exec_serv, &serv);
  bool res = ex_handler->Start();
  DCHECK(res);
  sleep(1);
  err = tcp.Connect();
  ASSERT_FALSE(err && err->IsError());
  inf = tcp.GetInfo();
  ASSERT_FALSE(inf.addr_info() == NULL);
  ex_handler->Join();
  err = tcp.Close();
  ASSERT_FALSE(err && err->IsError());
  err = serv.Close();
  ASSERT_FALSE(err && err->IsError());
}
