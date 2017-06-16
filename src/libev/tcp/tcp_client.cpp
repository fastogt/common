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

#include <common/libev/tcp/tcp_client.h>

#include <common/libev/io_client.h>  // for IoClient

#include <common/net/socket_tcp.h>  // for SocketHolder

namespace common {
namespace libev {
class IoLoop;
}
}  // namespace common

namespace common {
namespace libev {
namespace tcp {

TcpClient::TcpClient(IoLoop* server, const common::net::socket_info& info, flags_t flags)
    : IoClient(server, flags), sock_(info) {}

TcpClient::~TcpClient() {}

common::net::socket_info TcpClient::Info() const {
  return sock_.GetInfo();
}

descriptor_t TcpClient::GetFd() const {
  return sock_.GetFd();
}

common::Error TcpClient::Write(const char* data, size_t size, size_t* nwrite) {
  if (!data || !size || !nwrite) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  size_t total = 0;          // how many bytes we've sent
  size_t bytes_left = size;  // how many we have left to send

  while (total < size) {
    size_t n;
    common::Error err = sock_.Write(data, size, &n);
    if (err && err->IsError()) {
      return err;
    }
    total += n;
    bytes_left -= n;
  }

  *nwrite = total;  // return number actually sent here
  return common::Error();
}

common::Error TcpClient::Read(char* out, size_t size, size_t* nread) {
  if (!out || !size || !nread) {
    return common::make_error_value("Invalid input argument(s)", common::ErrorValue::E_ERROR);
  }

  size_t total = 0;          // how many bytes we've readed
  size_t bytes_left = size;  // how many we have left to read

  while (total < size) {
    size_t n;
    common::Error err = sock_.Read(out + total, bytes_left, &n);
    if (err && err->IsError()) {
      return err;
    }
    total += n;
    bytes_left -= n;
  }

  *nread = total;  // return number actually readed here
  return common::Error();
}

void TcpClient::CloseImpl() {
  common::Error err = sock_.Close();
  if (err && err->IsError()) {
    DEBUG_MSG_ERROR(err);  // FIX ME, don't write to log
  }
}

}  // namespace tcp
}  // namespace libev
}  // namespace common