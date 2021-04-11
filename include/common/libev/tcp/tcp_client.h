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

#include <common/libev/io_client.h>  // for IoClient
#include <common/net/socket_info.h>

namespace common {
namespace net {
class TcpSocketHolder;
}
namespace libev {
namespace tcp {

class TcpClient : public IoClient {
 public:
  TcpClient(IoLoop* server, const net::socket_info& info, flags_t flags = EV_READ);
  ~TcpClient() override;

  net::socket_info GetInfo() const;

  ErrnoError SetBlocking(bool block) WARN_UNUSED_RESULT;

 protected:
  descriptor_t GetFd() const override;

 private:
  ErrnoError DoSingleWrite(const void* data, size_t size, size_t* nwrite_out) override WARN_UNUSED_RESULT;
  ErrnoError DoSingleRead(void* out_data, size_t max_size, size_t* nread_out) override WARN_UNUSED_RESULT;

  ErrnoError DoClose() override;

  net::TcpSocketHolder* sock_;
};

}  // namespace tcp
}  // namespace libev
}  // namespace common
