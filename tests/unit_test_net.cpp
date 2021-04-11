#include <gtest/gtest.h>

#include <common/net/net.h>
#include <common/net/socket_tcp.h>
#include <common/sprintf.h>
#include <common/threads/thread_manager.h>

void exec_serv(common::net::ServerSocketTcp* serv) {
  common::net::socket_info inf;
  common::ErrnoError err = serv->Accept(&inf);
  ASSERT_FALSE(err);
}

TEST(HostAndPort, methods) {
  common::net::HostAndPort invalid;
  ASSERT_FALSE(invalid.IsValid());
  ASSERT_FALSE(invalid.IsLocalHost());

  const common::net::HostAndPort local_host = common::net::HostAndPort::CreateLocalHostIPV4(RANDOM_PORT);
  ASSERT_TRUE(local_host.IsValid());
  ASSERT_TRUE(local_host.IsLocalHost());
  ASSERT_EQ(local_host.GetPort(), RANDOM_PORT);

  const common::net::HostAndPort local_host2("127.0.0.1", RANDOM_PORT);
  ASSERT_TRUE(local_host2.IsValid());
  ASSERT_TRUE(local_host2.IsLocalHost());
  ASSERT_EQ(local_host2.GetPort(), RANDOM_PORT);
  ASSERT_EQ(local_host, local_host2);

  const common::net::HostAndPort local_host3("::1", RANDOM_PORT);
  ASSERT_TRUE(local_host3.IsValid());
  ASSERT_TRUE(local_host3.IsLocalHost());
  ASSERT_EQ(local_host3.GetPort(), RANDOM_PORT);

  const uint16_t valid_port = 8080;
  const common::net::HostAndPort valid_host = common::net::HostAndPort("192.168.1.2", valid_port);
  ASSERT_TRUE(valid_host.IsValid());
  ASSERT_FALSE(valid_host.IsLocalHost());
  ASSERT_EQ(valid_host.GetPort(), valid_port);

  const common::net::HostAndPort fastotv("fastotv.com", 5999);
  ASSERT_TRUE(fastotv.IsSameHost("2a03:b0c0:2:d0::383:2001"));
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

  const std::string host_str3 = common::MemSPrintf("[1fff:0:a88:85a3::ac1f]:%u", valid_port);
  ASSERT_TRUE(common::ConvertFromString(host_str3, &local_host));
  ASSERT_TRUE(local_host.IsValid());
  ASSERT_FALSE(local_host.IsLocalHost());
  ASSERT_FALSE(local_host.IsDefaultRoute());
  ASSERT_EQ(local_host.GetPort(), valid_port);
  ASSERT_EQ(local_host.GetHost(), "[1fff:0:a88:85a3::ac1f]");
  ASSERT_EQ(host_str3, common::ConvertToString(local_host));

  const std::string host_str4 = common::MemSPrintf("[::]:%u", valid_port);
  ASSERT_TRUE(common::ConvertFromString(host_str4, &local_host));
  ASSERT_TRUE(local_host.IsValid());
  ASSERT_FALSE(local_host.IsLocalHost());
  ASSERT_TRUE(local_host.IsDefaultRoute());
  ASSERT_EQ(local_host.GetPort(), valid_port);
  ASSERT_EQ(local_host.GetHost(), "[::]");
  ASSERT_EQ(host_str4, common::ConvertToString(local_host));

  ASSERT_TRUE(common::net::HostAndPort("localhost", 0).IsLocalHost());
  ASSERT_TRUE(common::net::HostAndPort("127.0.0.1", 0).IsLocalHost());
  ASSERT_TRUE(common::net::HostAndPort("::1", 0).IsLocalHost());
  ASSERT_TRUE(common::net::HostAndPort("0.0.0.0", 0).IsDefaultRoute());
  ASSERT_TRUE(common::net::HostAndPort("::", 0).IsDefaultRoute());
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

TEST(ServerSocketTcpAndClientSocketTcpIpv4, workflow) {
  using namespace common::net;
  HostAndPort host("127.0.0.1", 4567);
  ServerSocketTcp serv(host);
  common::ErrnoError err = serv.Bind(true);
  ASSERT_FALSE(err);
  err = serv.Listen(5);
  ASSERT_FALSE(err);

  ClientSocketTcp tcp(host);
  socket_info inf = tcp.GetInfo();
  ASSERT_TRUE(inf.addr_info() == NULL);
  auto ex_handler = THREAD_MANAGER()->CreateThread(&exec_serv, &serv);
  bool res = ex_handler->Start();
  DCHECK(res);
  sleep(1);
  err = tcp.Connect();
  ASSERT_FALSE(err);
  inf = tcp.GetInfo();
  ASSERT_FALSE(inf.addr_info() == NULL);
  ex_handler->Join();
  err = tcp.Close();
  ASSERT_FALSE(err);
  err = serv.Close();
  ASSERT_FALSE(err);
}

TEST(SocketTcpIpv4, bindRandomWorkflow) {
  using namespace common::net;
  HostAndPort host("127.0.0.1", RANDOM_PORT);
  ServerSocketTcp serv(host);

  common::ErrnoError err = serv.Bind(false);
  ASSERT_FALSE(err);
  err = serv.Listen(5);
  ASSERT_FALSE(err);

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
  ASSERT_FALSE(err);
  inf = tcp.GetInfo();
  ASSERT_FALSE(inf.addr_info() == NULL);
  ex_handler->Join();
  err = tcp.Close();
  ASSERT_FALSE(err);
  err = serv.Close();
  ASSERT_FALSE(err);
}

TEST(ServerSocketTcpAndClientSocketTcpIpv6, workflow) {
  using namespace common::net;
  HostAndPort host("[::]", 4567);
  ServerSocketTcp serv(host);
  common::ErrnoError err = serv.Bind(true);
  ASSERT_FALSE(err);
  err = serv.Listen(5);
  ASSERT_FALSE(err);

  ClientSocketTcp tcp(host);
  socket_info inf = tcp.GetInfo();
  ASSERT_TRUE(inf.addr_info() == NULL);
  auto ex_handler = THREAD_MANAGER()->CreateThread(&exec_serv, &serv);
  bool res = ex_handler->Start();
  DCHECK(res);
  sleep(1);
  err = tcp.Connect();
  ASSERT_FALSE(err);
  inf = tcp.GetInfo();
  ASSERT_FALSE(inf.addr_info() == NULL);
  ex_handler->Join();
  err = tcp.Close();
  ASSERT_FALSE(err);
  err = serv.Close();
  ASSERT_FALSE(err);
}

TEST(SocketTcpIpv6, bindRandomWorkflow) {
  using namespace common::net;
  HostAndPort host("[::1]", RANDOM_PORT);
  ServerSocketTcp serv(host);

  common::ErrnoError err = serv.Bind(false);
  ASSERT_FALSE(err);
  err = serv.Listen(5);
  ASSERT_FALSE(err);

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
  ASSERT_FALSE(err);
  inf = tcp.GetInfo();
  ASSERT_FALSE(inf.addr_info() == NULL);
  ex_handler->Join();
  err = tcp.Close();
  ASSERT_FALSE(err);
  err = serv.Close();
  ASSERT_FALSE(err);
}
