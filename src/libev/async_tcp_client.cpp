/*  Copyright (C) 2014-2022 FastoGT. All right reserved.

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

#include <common/libev/async_tcp_client.h>
#include <common/net/socket_tcp.h>

namespace common {
namespace libev {
namespace tcp {

AsyncTcpClient::AsyncTcpClient(IoLoop* server, const net::socket_info& info, flags_t flags)
    : AsyncTcpClient(server, new net::TcpSocketHolder(info), flags) {}

AsyncTcpClient::AsyncTcpClient(IoLoop* server, net::TcpSocketHolder* sock, flags_t flags)
    : AsyncIoClient(server, flags), sock_(sock) {
  // Ensure socket is non-blocking for async operation
  if (sock_) {
    sock_->SetBlocking(false);
  }
}

net::socket_info AsyncTcpClient::GetInfo() const {
  return sock_->GetInfo();
}

const char* AsyncTcpClient::ClassName() const {
  return "AsyncTcpClient";
}

descriptor_t AsyncTcpClient::GetFd() const {
  return sock_->GetFd();
}

ErrnoError AsyncTcpClient::DoAsyncWrite() {
  if (!sock_) {
    return make_error_perror("AsyncTcpClient::DoAsyncWrite", EINVAL);
  }

  if (write_buffer_.empty()) {
    return ErrnoError();
  }

  size_t n;
  ErrnoError err = sock_->Write(write_buffer_.data(), write_buffer_.size(), &n);
  if (!err) {
    write_buffer_.erase(write_buffer_.begin(), write_buffer_.begin() + n);
    wrote_bytes_ += n;
  } else if (err->GetErrorCode() == EAGAIN || err->GetErrorCode() == EWOULDBLOCK) {
    // Would block, try again later
    return ErrnoError();
  }

  return err;
}

ErrnoError AsyncTcpClient::DoAsyncRead() {
  if (!sock_) {
    return make_error_perror("AsyncTcpClient::DoAsyncRead", EINVAL);
  }

  char buffer[4096];
  size_t n;
  ErrnoError err = sock_->Read(buffer, sizeof(buffer), &n);
  if (!err && n > 0) {
    read_buffer_.insert(read_buffer_.end(), buffer, buffer + n);
    read_bytes_ += n;
  } else if (err && err->GetErrorCode() != EAGAIN && err->GetErrorCode() != EWOULDBLOCK) {
    return err;
  }

  return ErrnoError();
}

ErrnoError AsyncTcpClient::DoSendFile(descriptor_t file_fd, off_t offset, size_t file_size) {
  if (!sock_) {
    return make_error_perror("AsyncTcpClient::DoSendFile", EINVAL);
  }

  return sock_->SendFile(file_fd, offset, file_size);
}

ErrnoError AsyncTcpClient::DoClose() {
  return sock_->Close();
}

}  // namespace tcp
}  // namespace libev
}  // namespace common