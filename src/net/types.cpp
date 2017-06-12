/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

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

#include <common/net/types.h>

#ifdef OS_POSIX
#include <netdb.h>  // for addrinfo
#else
#include <winsock2.h>
#endif

#include <stddef.h>  // for size_t
#include <stdlib.h>  // for NULL, free, calloc, malloc
#include <string.h>  // for memcpy

#include <algorithm>  // for transform
#include <string>     // for string, basic_string, etc

#include <common/convert2string.h>  // for ConvertFromString
#include <common/macros.h>          // for NOTREACHED
#include <common/sprintf.h>         // for SNPrintf

namespace common {
namespace net {

HostAndPort::HostAndPort() : host(), port(0) {}

HostAndPort::HostAndPort(const std::string& host, uint16_t port) : host(host), port(port) {}

bool HostAndPort::IsValid() const {
  return !host.empty();
}

bool HostAndPort::IsLocalHost() const {
  return common::net::IsLocalHost(host);
}

bool IsLocalHost(const std::string& host) {
  if (host.empty()) {
    return false;
  }

  std::string lhost = host;
  std::transform(lhost.begin(), lhost.end(), lhost.begin(), ::tolower);

  return lhost == "localhost" || lhost == "127.0.0.1";
}

HostAndPort HostAndPort::CreateLocalHost(uint16_t port) {
  return HostAndPort("localhost", port);
}

bool HostAndPort::Equals(const HostAndPort& other) const {
  return host == other.host && port == other.port;
}

HostAndPortAndSlot::HostAndPortAndSlot() : HostAndPort(), slot(0) {}

HostAndPortAndSlot::HostAndPortAndSlot(const std::string& host, uint16_t port, uint16_t slot)
    : HostAndPort(host, port), slot(slot) {}

bool HostAndPortAndSlot::Equals(const HostAndPortAndSlot& other) const {
  return host == other.host && port == other.port && slot == other.slot;
}

socket_info::socket_info() : fd_(INVALID_SOCKET_VALUE), addr_(NULL), host_(NULL), port_(0) {}

socket_info::socket_info(socket_descr_t fd) : fd_(fd), addr_(NULL), host_(NULL), port_(0) {}

socket_info::socket_info(socket_descr_t fd, struct addrinfo* info) : fd_(fd), addr_(NULL), host_(NULL), port_(0) {
  addr_ = copy_addrinfo(info);
}

socket_info::socket_info(const socket_info& other) : fd_(other.fd_), addr_(NULL), host_(NULL), port_(other.port_) {
  addr_ = copy_addrinfo(other.addr_);
  if (other.host_) {
    host_ = strdup(other.host_);
  }
}

socket_info& socket_info::operator=(const socket_info& other) {
  if (this == &other) {
    NOTREACHED();
    return *this;
  }

  freeaddrinfo_ex(&addr_);

  fd_ = other.fd_;
  addr_ = copy_addrinfo(other.addr_);
  return *this;
}

socket_info::socket_info(socket_info&& other) : fd_(INVALID_SOCKET_VALUE), addr_(NULL) {
  fd_ = other.fd_;
  addr_ = other.addr_;

  other.fd_ = INVALID_SOCKET_VALUE;
  other.addr_ = NULL;
}

socket_info& socket_info::operator=(socket_info&& other) {
  if (this == &other) {
    NOTREACHED();
    return *this;
  }

  freeaddrinfo_ex(&addr_);

  fd_ = other.fd_;
  addr_ = other.addr_;

  other.fd_ = INVALID_SOCKET_VALUE;
  other.addr_ = NULL;

  return *this;
}

socket_info::~socket_info() {
  freeaddrinfo_ex(&addr_);
  if (host_) {
    free(host_);
    host_ = NULL;
  }
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
    host_ = NULL;
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
  return reinterpret_cast<struct sockaddr*>(malloc(addr_len));
}

struct sockaddr* copy_sockaddr(const struct sockaddr* addr, socklen_t addr_len) {
  if (!addr) {
    return NULL;
  }

  struct sockaddr* laddr = alloc_sockaddr(addr_len);
  if (!laddr) {
    return NULL;
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
    *addr = NULL;
  }
}

struct addrinfo* alloc_addrinfo() {
  return reinterpret_cast<struct addrinfo*>(calloc(1, sizeof(struct addrinfo)));
}

struct addrinfo* copy_addrinfo(const struct addrinfo* info) {
  if (!info) {
    return NULL;
  }

  struct addrinfo* linfo = alloc_addrinfo();
  if (!linfo) {
    return NULL;
  }

  if (info->ai_addr) {
    linfo->ai_addr = copy_sockaddr(info->ai_addr, info->ai_addrlen);
  }
  linfo->ai_addrlen = info->ai_addrlen;
  linfo->ai_family = info->ai_family;
  linfo->ai_flags = info->ai_flags;
  linfo->ai_protocol = info->ai_protocol;
  linfo->ai_socktype = info->ai_socktype;
  linfo->ai_next = NULL;
  linfo->ai_canonname = NULL;
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
    *info = NULL;
  }
}

}  // namespace net

std::string ConvertToString(const net::HostAndPort& host) {
  if (!host.IsValid()) {
    return std::string();
  }

  static const uint16_t size_buff = 512;
  char buff[size_buff] = {0};
  SNPrintf(buff, size_buff, "%s:%d", host.host, host.port);
  return buff;
}

bool ConvertFromString(const std::string& from, net::HostAndPort* out) {
  if (!out) {
    return false;
  }

  net::HostAndPort res;
  size_t del = from.find_first_of(':');
  if (del != std::string::npos) {
    res.host = from.substr(0, del);
    uint16_t lport;
    bool ok = ConvertFromString(from.substr(del + 1), &lport);
    UNUSED(ok);
    res.port = lport;
  }

  *out = res;
  return true;
}

std::string ConvertToString(const net::HostAndPortAndSlot& host) {
  if (!host.IsValid()) {
    return std::string();
  }

  static const uint16_t size_buff = 512;
  char buff[size_buff] = {0};
  SNPrintf(buff, size_buff, "%s:%d@%d", host.host, host.port, host.slot);
  return buff;
}

bool ConvertFromString(const std::string& from, net::HostAndPortAndSlot* out) {
  if (!out) {
    return false;
  }

  net::HostAndPortAndSlot lout;
  size_t del = from.find_first_of(':');
  if (del != std::string::npos) {
    lout.host = from.substr(0, del);
    size_t del_s = from.find_first_of('@');
    if (del_s != std::string::npos) {
      uint16_t lport;
      bool res = ConvertFromString(from.substr(del + 1, del_s - del - 1), &lport);
      UNUSED(res);
      lout.port = lport;

      uint16_t lslot;
      res = ConvertFromString(from.substr(del_s + 1), &lslot);
      UNUSED(res);
      lout.slot = lslot;
    } else {
      uint16_t lport;
      bool res = ConvertFromString(from.substr(del + 1), &lport);
      UNUSED(res);
      lout.port = lport;
    }
  }

  *out = lout;
  return true;
}

}  // namespace common
