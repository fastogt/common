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

#pragma once

#include <stddef.h>  // for size_t
#include <stdint.h>  // for uint16_t
#include <unistd.h>  // for socklen_t, ssize_t, off_t

#include <string>  // for string

#include <common/error.h>      // for ErrnoError
#include <common/macros.h>     // for WARN_UNUSED_RESULT
#include <common/net/types.h>  // for socket_descr_t, etc

struct addrinfo;
struct sockaddr;

namespace common {
namespace net {

ErrnoError socket(int domain, socket_t type, int protocol, socket_info* out_info) WARN_UNUSED_RESULT;
ErrnoError bind(socket_descr_t fd,
                const struct sockaddr* addr,
                socklen_t addr_len,
                const addrinfo* ainf,
                bool reuseaddr,
                socket_info* out_info) WARN_UNUSED_RESULT;
ErrnoError getsockname(socket_descr_t fd, struct sockaddr* addr, socklen_t addr_len, socket_info* out_info)
    WARN_UNUSED_RESULT;
uint16_t get_in_port(struct sockaddr* sa);
ErrnoError listen(const socket_info& info, int backlog) WARN_UNUSED_RESULT;
ErrnoError accept(const socket_info& info, socket_info* out_info) WARN_UNUSED_RESULT;

ErrnoError connect(const HostAndPort& from, socket_t socktype, struct timeval* timeout, socket_info* out_info)
    WARN_UNUSED_RESULT;
ErrnoError connect(const socket_info& info, struct timeval* timeout, socket_info* out_info) WARN_UNUSED_RESULT;

ErrnoError close(socket_descr_t fd) WARN_UNUSED_RESULT;

ErrnoError set_blocking_socket(socket_descr_t sock, bool blocking) WARN_UNUSED_RESULT;

ErrnoError write_to_socket(socket_descr_t fd, const char* data, size_t size, size_t* nwritten_out) WARN_UNUSED_RESULT;
ErrnoError read_from_socket(socket_descr_t fd, char* out, size_t len, size_t* nread_out) WARN_UNUSED_RESULT;

ErrnoError sendto(socket_descr_t fd,
                  const char* data,
                  uint16_t len,
                  struct sockaddr* addr,
                  socklen_t addr_len,
                  ssize_t* nwritten_out) WARN_UNUSED_RESULT;
ErrnoError recvfrom(socket_descr_t fd,
                    char* out_data,
                    uint16_t max_size,
                    sockaddr* addr,
                    socklen_t* addr_len,
                    ssize_t* nread_out) WARN_UNUSED_RESULT;

ErrnoError send_file_to_fd(socket_descr_t sock, int fd, off_t offset, off_t size) WARN_UNUSED_RESULT;
ErrnoError send_file(const std::string& path, const HostAndPort& to) WARN_UNUSED_RESULT;

}  // namespace net
}  // namespace common
