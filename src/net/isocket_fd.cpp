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

#include <common/net/isocket_fd.h>

#include <common/net/net.h>

namespace common {
namespace net {

bool ISocketFd::IsValid() const {
  return GetFd() != INVALID_SOCKET_VALUE;
}

ErrnoError ISocketFd::SetBlocking(bool block) {
  DCHECK(IsValid());
  return set_blocking_socket(GetFd(), block);
}

#if defined(OS_POSIX)
ErrnoError ISocketFd::WriteEv(const struct iovec* iovec, int count, size_t* nwrite_out) {
  DCHECK(IsValid());
  return write_ev_to_socket(GetFd(), iovec, count, nwrite_out);
}

ErrnoError ISocketFd::ReadEv(const struct iovec* iovec, int count, size_t* nwrite_out) {
  DCHECK(IsValid());
  return read_ev_to_socket(GetFd(), iovec, count, nwrite_out);
}

ErrnoError ISocketFd::WriteImpl(const void* data, size_t size, size_t* nwrite_out) {
  return write_to_socket(GetFd(), data, size, nwrite_out);
}

ErrnoError ISocketFd::ReadImpl(void* out_data, size_t max_size, size_t* nread_out) {
  return read_from_socket(GetFd(), out_data, max_size, nread_out);
}

ErrnoError ISocketFd::SendFileImpl(descriptor_t file_fd, size_t file_size) {
  const socket_descr_t fd = GetFd();
  return send_file_to_fd(fd, file_fd, 0, file_size);
}

#else
ErrnoError ISocketFd::WriteImpl(const void* data, size_t size, size_t* nwrite_out) {
  return write_to_tcp_socket(GetFd(), data, size, nwrite_out);
}

ErrnoError ISocketFd::ReadImpl(void* out_data, size_t max_size, size_t* nread_out) {
  return read_from_tcp_socket(GetFd(), out_data, max_size, nread_out);
}

ErrnoError ISocketFd::SendFileImpl(descriptor_t file_fd, size_t file_size) {
  const socket_descr_t fd = GetFd();
  return send_file_to_fd(fd, file_fd, 0, file_size);
}
#endif

ErrnoError ISocketFd::SendFile(descriptor_t file_fd, size_t file_size) {
  DCHECK(IsValid());
  const socket_descr_t fd = GetFd();
  if (file_fd == INVALID_DESCRIPTOR || fd == INVALID_SOCKET_VALUE) {
    return make_error_perror("SendFile", EINVAL);
  }

  return SendFileImpl(file_fd, file_size);
}

ErrnoError ISocketFd::CloseImpl() {
  ErrnoError err = close(GetFd());
  if (err) {
    return err;
  }

  SetFd(INVALID_DESCRIPTOR);
  return ErrnoError();
}

}  // namespace net
}  // namespace common
