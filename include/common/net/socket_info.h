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

#pragma once

#if defined(OS_POSIX)
#include <netinet/in.h>
#include <sys/socket.h>
#else
#include <ws2tcpip.h>
#endif

#include <common/config.h>
#include <common/macros.h>  // for INVALID_DESCRIPTOR

struct addrinfo;

#define RANDOM_PORT 0

#if defined(OS_WIN)
#define INVALID_SOCKET_VALUE INVALID_SOCKET
#else
#define INVALID_SOCKET_VALUE INVALID_DESCRIPTOR
#endif

namespace common {
namespace net {

#if defined(OS_WIN)
typedef SOCKET socket_descr_t;
#else
typedef int socket_descr_t;
#endif

/* Types of sockets.  */

#if defined(COMPILER_MINGW)
enum socket_t {
  ST_SOCK_STREAM = 1,   /* Sequenced, reliable, connection-based
                           byte streams.  */
  ST_SOCK_DGRAM = 2,    /* Connectionless, unreliable datagrams
                           of fixed maximum length.  */
  ST_SOCK_RAW = 3,      /* Raw protocol interface.  */
  ST_SOCK_RDM = 4,      /* Reliably-delivered messages.  */
  ST_SOCK_SEQPACKET = 5 /* Sequenced, reliable, connection-based,
                           datagrams of fixed maximum length.  */
};
#else
enum socket_t {
  ST_SOCK_STREAM = 1,    /* Sequenced, reliable, connection-based
                            byte streams.  */
  ST_SOCK_DGRAM = 2,     /* Connectionless, unreliable datagrams
                            of fixed maximum length.  */
  ST_SOCK_RAW = 3,       /* Raw protocol interface.  */
  ST_SOCK_RDM = 4,       /* Reliably-delivered messages.  */
  ST_SOCK_SEQPACKET = 5, /* Sequenced, reliable, connection-based,
                            datagrams of fixed maximum length.  */
  ST_SOCK_DCCP = 6,      /* Datagram Congestion Control Protocol.  */
  ST_SOCK_PACKET = 10,   /* Linux specific way of getting packets
                            at the dev level. For writing rarp and
                            other similar things on the user level. */
};
#endif

class socket_info {
 public:
  socket_info();
  explicit socket_info(socket_descr_t fd);
  socket_info(socket_descr_t fd, struct addrinfo* info);
  socket_info(const socket_info& other);
  socket_info& operator=(const socket_info& other);

  socket_info(socket_info&& other);
  socket_info& operator=(socket_info&& other);

  ~socket_info();

  bool is_valid() const;

  void set_fd(socket_descr_t fd);
  socket_descr_t fd() const;

  void set_addrinfo(const struct addrinfo* info);
  struct addrinfo* addr_info() const;

  void set_sockaddr(const struct sockaddr* addr, socklen_t addr_len);

  const char* host() const;
  void set_host(const char* host);

  uint16_t port() const;
  void set_port(uint16_t port);

 private:
  socket_descr_t fd_;
  struct addrinfo* addr_;
  char* host_;
  uint16_t port_;
};

sockaddr* alloc_sockaddr(socklen_t addr_len);
struct sockaddr* copy_sockaddr(const struct sockaddr* addr, socklen_t addr_len);
void free_sockaddr(struct sockaddr** addr);

struct addrinfo* alloc_addrinfo();
struct addrinfo* copy_addrinfo(const struct addrinfo* info);
void freeaddrinfo_ex(addrinfo** info);

}  // namespace net
}  // namespace common
