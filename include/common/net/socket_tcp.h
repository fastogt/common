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

#include <stddef.h>     // for size_t
#include <sys/types.h>  // for ssize_t

#include <common/error.h>      // for ErrnoError
#include <common/macros.h>     // for WARN_UNUSED_RESULT
#include <common/net/types.h>  // for HostAndPort, socket_info

namespace common {
namespace net {

class SocketHolder {
 public:
  explicit SocketHolder(const socket_info& info);
  explicit SocketHolder(socket_descr_t fd);

  ~SocketHolder();

  socket_info GetInfo() const;
  socket_descr_t GetFd() const;

  ErrnoError Write(const char* data, size_t size, size_t* nwrite_out) WARN_UNUSED_RESULT;
  ErrnoError Read(char* out, size_t len, size_t* nread_out) WARN_UNUSED_RESULT;
  ErrnoError Close() WARN_UNUSED_RESULT;

 protected:
  socket_info info_;

 private:
  DISALLOW_COPY_AND_ASSIGN(SocketHolder);
};

class SocketTcp : public SocketHolder {
 public:
  explicit SocketTcp(const HostAndPort& host);
  HostAndPort GetHost() const;
  ~SocketTcp();

 protected:
  HostAndPort host_;

 private:
  DISALLOW_COPY_AND_ASSIGN(SocketTcp);
};

class ClientSocketTcp : public SocketTcp {
 public:
  explicit ClientSocketTcp(const HostAndPort& host);

  ErrnoError Connect() WARN_UNUSED_RESULT;

 private:
  DISALLOW_COPY_AND_ASSIGN(ClientSocketTcp);
};

class ServerSocketTcp : public SocketTcp {
 public:
  explicit ServerSocketTcp(const HostAndPort& host);

  ErrnoError Bind() WARN_UNUSED_RESULT;
  ErrnoError Listen(int backlog) WARN_UNUSED_RESULT;
  ErrnoError Accept(socket_info* info) WARN_UNUSED_RESULT;

 private:
  DISALLOW_COPY_AND_ASSIGN(ServerSocketTcp);
};

}  // namespace net
}  // namespace common
