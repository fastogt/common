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

#include <common/libev/tcp/tcp_client.h>

#include <common/net/socket_tcp.h>

namespace common {
namespace libev {
namespace tcp {

TcpClient::TcpClient(IoLoop* server, const net::socket_info& info, flags_t flags)
    : IoClient(server, flags), sock_(new net::TcpSocketHolder(info)) {}

TcpClient::~TcpClient() {
  destroy(&sock_);
}

net::socket_info TcpClient::GetInfo() const {
  return sock_->GetInfo();
}

descriptor_t TcpClient::GetFd() const {
  return sock_->GetFd();
}

ErrnoError TcpClient::SetBlocking(bool block) {
  return sock_->SetBlocking(block);
}

ErrnoError TcpClient::DoSingleWrite(const void* data, size_t size, size_t* nwrite_out) {
  if (!sock_) {
    return make_error_perror("TcpClient::DoSingleWrite", EINVAL);
  }

  return sock_->Write(data, size, nwrite_out);
}

ErrnoError TcpClient::DoSingleRead(void* out_data, size_t max_size, size_t* nread_out) {
  if (!sock_) {
    return make_error_perror("TcpClient::DoSingleRead", EINVAL);
  }

  return sock_->Read(static_cast<char*>(out_data), max_size, nread_out);
}

ErrnoError TcpClient::DoClose() {
  return sock_->Close();
}

}  // namespace tcp
}  // namespace libev
}  // namespace common
