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

#if defined(OS_WIN)
#include <winsock2.h>
#else
#include <netinet/in.h>
#endif

#undef SetPort

#include <common/net/socket_tcp.h>

#include <errno.h>
#include <string.h>

#if defined(COMPILER_MSVC)
#include <io.h>
#endif

#include <common/net/net.h>  // for bind, accept, close, etc

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

ErrnoError TcpSocketHolder::WriteImpl(const void* data, size_t size, size_t* nwrite_out) {
  return write_to_tcp_socket(GetFd(), data, size, nwrite_out);
}

ErrnoError TcpSocketHolder::ReadImpl(void* out_data, size_t max_size, size_t* nread_out) {
  return read_from_tcp_socket(GetFd(), out_data, max_size, nread_out);
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
  const HostAndPort hs = GetHost();
  ErrnoError err = resolve(hs, ST_SOCK_STREAM, &linfo);  // init fd
  if (err) {
    return err;
  }

  socket_descr_t fd = linfo.fd();
  addrinfo* ainf = linfo.addr_info();
  socket_info lbinfo;
  err = bind(fd, ainf, reuseaddr, &lbinfo);  // init sockaddr
  if (err) {
    return err;
  }

  bool is_random_port = linfo.port() == 0;
  if (is_random_port) {                    // random port
    err = getsockname(fd, ainf, &lbinfo);  // init sockaddr
    if (err) {
      return err;
    }

    HostAndPort new_hs = hs;
    uint16_t port = 0;
    ErrnoError errn = get_in_port(lbinfo.addr_info(), &port);
    if (!errn) {
      new_hs.SetPort(port);
    }

    SetInfo(lbinfo);
    SetHost(new_hs);
    return ErrnoError();
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
