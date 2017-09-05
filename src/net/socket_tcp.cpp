/*  Copyright (C) 2014-2017 FastoGT. All right reserved.

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

#ifdef COMPILER_MSVC
#include <io.h>
#endif

#include <common/net/net.h>  // for bind, accept, close, etc

struct addrinfo;

namespace common {
namespace net {

SocketHolder::SocketHolder(const socket_info& info) : info_(info) {}

SocketHolder::~SocketHolder() {}

SocketHolder::SocketHolder(socket_descr_t fd) : info_(fd) {}

#ifdef OS_POSIX
ErrnoError SocketHolder::WriteEv(const struct iovec* iovec, int count, size_t* nwrite_out) {
  return net::write_ev_to_socket(info_.fd(), iovec, count, nwrite_out);
}

ErrnoError SocketHolder::ReadEv(const struct iovec* iovec, int count, size_t* nwrite_out) {
  return net::read_ev_to_socket(info_.fd(), iovec, count, nwrite_out);
}
#endif

ErrnoError SocketHolder::Write(const char* data, size_t size, size_t* nwrite_out) {
  return net::write_to_socket(info_.fd(), data, size, nwrite_out);
}

ErrnoError SocketHolder::Read(char* out, size_t len, size_t* nread_out) {
  return net::read_from_socket(info_.fd(), out, len, nread_out);
}

socket_info SocketHolder::GetInfo() const {
  return info_;
}

socket_descr_t SocketHolder::GetFd() const {
  return info_.fd();
}

ErrnoError SocketHolder::Close() {
  const socket_descr_t fd = info_.fd();
  ErrnoError err = close(fd);
  if (err && err->IsError()) {
    DNOTREACHED();
    return err;
  }

  info_.set_fd(INVALID_DESCRIPTOR);
  return ErrnoError();
}

SocketTcp::SocketTcp(const HostAndPort& host) : SocketHolder(INVALID_DESCRIPTOR), host_(host) {}

HostAndPort SocketTcp::GetHost() const {
  return host_;
}

SocketTcp::~SocketTcp() {}

ClientSocketTcp::ClientSocketTcp(const HostAndPort& host) : SocketTcp(host) {}

ErrnoError ClientSocketTcp::Connect() {
  return net::connect(host_, ST_SOCK_STREAM, NULL, &info_);
}

ServerSocketTcp::ServerSocketTcp(const HostAndPort& host) : SocketTcp(host) {}

ErrnoError ServerSocketTcp::Bind(bool reuseaddr) {
  socket_info linfo;
  ErrnoError err = socket(IP_DOMAIN, ST_SOCK_STREAM, 0, &linfo);  // init fd
  if (err && err->IsError()) {
    return err;
  }

  sockaddr_t addr;
  memset(&addr, 0, sizeof(sockaddr_t));
#ifdef IPV6_ENABLED
  addr.sin6_family = IP_DOMAIN;
  addr.sin6_port = htons(host_.GetPort());
  addr.sin6_addr = in6addr_any;
  bool is_random_port = addr.sin6_port == 0;
#else
  addr.sin_family = IP_DOMAIN;
  addr.sin_port = htons(host_.GetPort());
  addr.sin_addr.s_addr = INADDR_ANY;
  bool is_random_port = addr.sin_port == 0;
#endif

  socket_descr_t fd = linfo.fd();
  addrinfo* ainf = linfo.addr_info();

  if (is_random_port) {  // random port
    err = bind(fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(sockaddr_t), ainf, reuseaddr,
               &info_);  // init sockaddr
    if (err && err->IsError()) {
      return err;
    }

    sockaddr_t addr2;
    memset(&addr2, 0, sizeof(sockaddr_t));
    err = getsockname(fd, reinterpret_cast<struct sockaddr*>(&addr2), sizeof(sockaddr_t),
                      &info_);  // init sockaddr
    if (err && err->IsError()) {
      return err;
    }

    struct sockaddr* saddr = reinterpret_cast<struct sockaddr*>(&addr2);
    host_.SetPort(ntohs(get_in_port(saddr)));
    return ErrnoError();
  }

  return bind(fd, reinterpret_cast<struct sockaddr*>(&addr), sizeof(sockaddr_t), ainf, reuseaddr,
              &info_);  // init sockaddr
}

ErrnoError ServerSocketTcp::Listen(int backlog) {
  return listen(info_, backlog);
}

ErrnoError ServerSocketTcp::Accept(socket_info* info) {
  return accept(info_, info);
}

}  // namespace net
}  // namespace common
