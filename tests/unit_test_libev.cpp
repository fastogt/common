#include <gtest/gtest.h>

#include <common/libev/tcp/tcp_server.h>

TEST(Libev, IoServe) {
  common::net::HostAndPort hs("localhost", 8010);
  common::libev::tcp::TcpServer* serv = new common::libev::tcp::TcpServer(hs);
  delete serv;
}
