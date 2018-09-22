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

#include <errno.h>
#include <string.h>

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
  DCHECK(IsValid());
  return write_ev_to_socket(info_.fd(), iovec, count, nwrite_out);
}

ErrnoError SocketHolder::ReadEv(const struct iovec* iovec, int count, size_t* nwrite_out) {
  DCHECK(IsValid());
  return read_ev_to_socket(info_.fd(), iovec, count, nwrite_out);
}
#endif

socket_info SocketHolder::GetInfo() const {
  return info_;
}

socket_descr_t SocketHolder::GetFd() const {
  return info_.fd();
}

bool SocketHolder::IsValid() const {
  return info_.is_valid();
}

ErrnoError SocketHolder::SetBlocking(bool block) {
  return set_blocking_socket(info_.fd(), block);
}

ErrnoError SocketHolder::Write(const buffer_t& data, size_t* nwrite_out) {
  DCHECK(IsValid());
  return write_to_socket(info_.fd(), data.data(), data.size(), nwrite_out);
}

ErrnoError SocketHolder::Write(const std::string& data, size_t* nwrite_out) {
  DCHECK(IsValid());
  return write_to_socket(info_.fd(), data.data(), data.size(), nwrite_out);
}

ErrnoError SocketHolder::Write(const void* data, size_t size, size_t* nwrite_out) {
  DCHECK(IsValid());
  return write_to_socket(info_.fd(), data, size, nwrite_out);
}

ErrnoError SocketHolder::Read(buffer_t* out_data, size_t max_size, size_t* nread_out) {
  DCHECK(IsValid());
  return read_from_socket(info_.fd(), out_data->data(), max_size, nread_out);
}

ErrnoError SocketHolder::Read(std::string* out_data, size_t max_size, size_t* nread_out) {
  DCHECK(IsValid());
  char* buff = new char[max_size];
  ErrnoError err = read_from_socket(info_.fd(), buff, max_size, nread_out);
  if (err) {
    delete[] buff;
    return err;
  }

  *out_data = std::string(buff, *nread_out);
  delete[] buff;
  return err;
}

ErrnoError SocketHolder::Read(void* out_data, size_t max_size, size_t* nread_out) {
  DCHECK(IsValid());
  return read_from_socket(info_.fd(), out_data, max_size, nread_out);
}

ErrnoError SocketHolder::Close() {
  const socket_descr_t fd = info_.fd();
  ErrnoError err = close(fd);
  if (err) {
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

ErrnoError ClientSocketTcp::Connect(struct timeval* tv) {
  return net::connect(host_, ST_SOCK_STREAM, tv, &info_);
}

ErrnoError ClientSocketTcp::Disconnect() {
  return Close();
}

bool ClientSocketTcp::IsConnected() const {
  const socket_descr_t fd = info_.fd();
  return fd != INVALID_SOCKET_VALUE;
}

ErrnoError ClientSocketTcp::SendFile(descriptor_t file_fd, size_t file_size) {
  const socket_descr_t fd = info_.fd();
  if (file_fd == INVALID_DESCRIPTOR || fd == INVALID_SOCKET_VALUE) {
    return make_error_perror("SendFile", EINVAL);
  }

  return send_file_to_fd(fd, file_fd, 0, file_size);
}

ServerSocketTcp::ServerSocketTcp(const HostAndPort& host) : SocketTcp(host) {}

ErrnoError ServerSocketTcp::Bind(bool reuseaddr) {
  socket_info linfo;
  ErrnoError err = socket(IP_DOMAIN, ST_SOCK_STREAM, 0, &linfo);  // init fd
  if (err) {
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
    if (err) {
      return err;
    }

    sockaddr_t addr2;
    memset(&addr2, 0, sizeof(sockaddr_t));
    err = getsockname(fd, reinterpret_cast<struct sockaddr*>(&addr2), sizeof(sockaddr_t), &info_);  // init sockaddr
    if (err) {
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
