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

#include <common/net/socket_info.h>

#if defined(OS_POSIX)
#include <netdb.h>  // for addrinfo
#else
#include <winsock2.h>
#endif

#include <stddef.h>
#include <string.h>

namespace common {
namespace net {

socket_info::socket_info() : fd_(INVALID_SOCKET_VALUE), addr_(nullptr), host_(nullptr), port_(0) {}

socket_info::socket_info(socket_descr_t fd) : fd_(fd), addr_(nullptr), host_(nullptr), port_(0) {}

socket_info::socket_info(socket_descr_t fd, struct addrinfo* info) : fd_(fd), addr_(nullptr), host_(nullptr), port_(0) {
  addr_ = copy_addrinfo(info);
}

socket_info::socket_info(const socket_info& other)
    : fd_(other.fd_), addr_(nullptr), host_(nullptr), port_(other.port_) {
  addr_ = copy_addrinfo(other.addr_);
  if (other.host_) {
    if (host_) {
      free(host_);
    }
    host_ = strdup(other.host_);
  }
}

socket_info& socket_info::operator=(const socket_info& other) {
  if (this == &other) {
    DNOTREACHED();
    return *this;
  }

  freeaddrinfo_ex(&addr_);

  fd_ = other.fd_;
  addr_ = copy_addrinfo(other.addr_);
  if (other.host_) {
    if (host_) {
      free(host_);
    }
    host_ = strdup(other.host_);
  }
  port_ = other.port_;
  return *this;
}

socket_info::socket_info(socket_info&& other) : fd_(INVALID_SOCKET_VALUE), addr_(nullptr), host_(nullptr), port_(0) {
  fd_ = other.fd_;
  addr_ = other.addr_;
  host_ = other.host_;
  port_ = other.port_;

  other.fd_ = INVALID_SOCKET_VALUE;
  other.addr_ = nullptr;
  other.host_ = nullptr;
  other.port_ = 0;
}

socket_info& socket_info::operator=(socket_info&& other) {
  if (this == &other) {
    DNOTREACHED();
    return *this;
  }

  freeaddrinfo_ex(&addr_);

  fd_ = other.fd_;
  addr_ = other.addr_;
  host_ = other.host_;
  port_ = other.port_;

  other.fd_ = INVALID_SOCKET_VALUE;
  other.addr_ = nullptr;
  other.host_ = nullptr;
  other.port_ = 0;

  return *this;
}

socket_info::~socket_info() {
  freeaddrinfo_ex(&addr_);
  if (host_) {
    free(host_);
    host_ = nullptr;
  }
}

bool socket_info::is_valid() const {
  return fd_ != INVALID_SOCKET_VALUE;
}

void socket_info::set_fd(socket_descr_t fd) {
  fd_ = fd;
}

socket_descr_t socket_info::fd() const {
  return fd_;
}

void socket_info::set_addrinfo(const struct addrinfo* info) {
  freeaddrinfo_ex(&addr_);
  addr_ = copy_addrinfo(info);
}

struct addrinfo* socket_info::addr_info() const {
  return addr_;
}

void socket_info::set_sockaddr(const struct sockaddr* addr, socklen_t addr_len) {
  free_sockaddr(&addr_->ai_addr);
  addr_->ai_addr = copy_sockaddr(addr, addr_len);
  addr_->ai_addrlen = addr_len;
}

const char* socket_info::host() const {
  return host_;
}

void socket_info::set_host(const char* host) {
  if (host_) {
    free(host_);
    host_ = nullptr;
  }

  if (host) {
    host_ = strdup(host);
  }
}

uint16_t socket_info::port() const {
  return port_;
}

void socket_info::set_port(uint16_t port) {
  port_ = port;
}

struct sockaddr* alloc_sockaddr(socklen_t addr_len) {
  return static_cast<struct sockaddr*>(malloc(addr_len));
}

struct sockaddr* copy_sockaddr(const struct sockaddr* addr, socklen_t addr_len) {
  if (!addr) {
    return nullptr;
  }

  struct sockaddr* laddr = alloc_sockaddr(addr_len);
  if (!laddr) {
    return nullptr;
  }

  memcpy(laddr, addr, addr_len);
  return laddr;
}

void free_sockaddr(struct sockaddr** addr) {
  if (!addr) {
    return;
  }

  struct sockaddr* laddr = *addr;
  if (laddr) {
    free(laddr);
    *addr = nullptr;
  }
}

struct addrinfo* alloc_addrinfo() {
  return static_cast<struct addrinfo*>(calloc(1, sizeof(struct addrinfo)));
}

struct addrinfo* copy_addrinfo(const struct addrinfo* info) {
  if (!info) {
    return nullptr;
  }

  struct addrinfo* linfo = alloc_addrinfo();
  if (!linfo) {
    return nullptr;
  }

  if (info->ai_addr) {
    linfo->ai_addr = copy_sockaddr(info->ai_addr, info->ai_addrlen);
  }
  linfo->ai_addrlen = info->ai_addrlen;
  linfo->ai_family = info->ai_family;
  linfo->ai_flags = info->ai_flags;
  linfo->ai_protocol = info->ai_protocol;
  linfo->ai_socktype = info->ai_socktype;
  linfo->ai_next = nullptr;
  linfo->ai_canonname = nullptr;
  return linfo;
}

void freeaddrinfo_ex(struct addrinfo** info) {
  if (!info) {
    return;
  }

  struct addrinfo* linfo = *info;
  if (linfo) {
    free_sockaddr(&linfo->ai_addr);
    free(linfo);
    *info = nullptr;
  }
}

}  // namespace net
}  // namespace common
