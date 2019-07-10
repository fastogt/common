/*  Copyright (C) 2014-2018 FastoGT. All right reserved.

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

#ifdef OS_WIN
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif

#undef SetPort

#include <common/net/socket_tcp.h>

#include <errno.h>
#include <string.h>

#ifdef COMPILER_MSVC
#include <io.h>
#endif

#include <common/net/net.h>  // for bind, accept, close, etc

struct addrinfo;

namespace common {
namespace net {

TcpSocketHolder::TcpSocketHolder(const socket_info& info) : info_(info) {}

TcpSocketHolder::~TcpSocketHolder() {}

TcpSocketHolder::TcpSocketHolder(socket_descr_t fd) : info_(fd) {}

socket_info TcpSocketHolder::GetInfo() const {
  return info_;
}

void TcpSocketHolder::SetInfo(const socket_info& info) {
  DCHECK(!IsValid());
  info_ = info;
}

socket_descr_t TcpSocketHolder::GetFd() const {
  return info_.fd();
}

void TcpSocketHolder::SetFd(socket_descr_t fd) {
  info_.set_fd(fd);
}

SocketTcp::SocketTcp(const HostAndPort& host) : TcpSocketHolder(INVALID_SOCKET_VALUE), host_(host) {}

HostAndPort SocketTcp::GetHost() const {
  return host_;
}

void SocketTcp::SetHost(const HostAndPort& host) {
  host_ = host;
}

SocketTcp::~SocketTcp() {}

ClientSocketTcp::ClientSocketTcp(const HostAndPort& host) : SocketTcp(host) {}

ErrnoError ClientSocketTcp::Connect(struct timeval* tv) {
  socket_info inf;
  ErrnoError err = net::connect(GetHost(), ST_SOCK_STREAM, tv, &inf);
  if (err) {
    return err;
  }

  SetInfo(inf);
  return ErrnoError();
}

ErrnoError ClientSocketTcp::Disconnect() {
  return Close();
}

bool ClientSocketTcp::IsConnected() const {
  return IsValid();
}

ServerSocketTcp::ServerSocketTcp(const HostAndPort& host) : SocketTcp(host) {}

ErrnoError ServerSocketTcp::Bind(bool reuseaddr) {
  socket_info linfo;
  ErrnoError err = socket(IP_DOMAIN, ST_SOCK_STREAM, 0, &linfo);  // init fd
  if (err) {
    return err;
  }

  const HostAndPort hs = GetHost();
  sockaddr_t addr;
  memset(&addr, 0, sizeof(sockaddr_t));
#ifdef IPV6_ENABLED
  addr.sin6_family = IP_DOMAIN;
  addr.sin6_port = htons(hs.GetPort());
  addr.sin6_addr = in6addr_any;
  bool is_random_port = addr.sin6_port == 0;
#else
  addr.sin_family = IP_DOMAIN;
  addr.sin_port = htons(hs.GetPort());
  addr.sin_addr.s_addr = INADDR_ANY;
  bool is_random_port = addr.sin_port == 0;
#endif

  socket_descr_t fd = linfo.fd();
  addrinfo* ainf = linfo.addr_info();

  if (is_random_port) {  // random port
    socket_info lbinfo;
    err = bind(fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(sockaddr_t), ainf, reuseaddr,
               &lbinfo);  // init sockaddr
    if (err) {
      return err;
    }

    sockaddr_t addr2;
    memset(&addr2, 0, sizeof(sockaddr_t));
    err = getsockname(fd, reinterpret_cast<struct sockaddr*>(&addr2), sizeof(sockaddr_t), &lbinfo);  // init sockaddr
    if (err) {
      return err;
    }

    struct sockaddr* saddr = reinterpret_cast<struct sockaddr*>(&addr2);
    HostAndPort new_hs = hs;
    const auto port = ntohs(get_in_port(saddr));
    new_hs.SetPort(port);

    SetInfo(lbinfo);
    SetHost(new_hs);
    return ErrnoError();
  }

  socket_info lbinfo;
  err = bind(fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(sockaddr_t), ainf, reuseaddr,
             &lbinfo);  // init sockaddr
  if (err) {
    return err;
  }

  SetInfo(lbinfo);
  return ErrnoError();
}

ErrnoError ServerSocketTcp::Listen(int backlog) {
  DCHECK(IsValid());
  return listen(GetInfo(), backlog);
}

ErrnoError ServerSocketTcp::Accept(socket_info* info) {
  DCHECK(IsValid());
  return accept(GetInfo(), info);
}

}  // namespace net
}  // namespace common
