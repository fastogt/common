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

#include <common/net/isocket.h>
#include <common/net/socket_info.h>

namespace common {
namespace net {

class ISocketFd : public ISocket {
 public:
  virtual socket_descr_t GetFd() const = 0;
  virtual void SetFd(socket_descr_t fd) = 0;

  ErrnoError SetBlocking(bool block) WARN_UNUSED_RESULT;

#if defined(OS_POSIX)
  ErrnoError WriteEv(const struct iovec* iovec, int count, size_t* nwrite_out) WARN_UNUSED_RESULT;
  ErrnoError ReadEv(const struct iovec* iovec, int count, size_t* nwrite_out) WARN_UNUSED_RESULT;
#endif

  ErrnoError SendFile(descriptor_t file_fd, size_t file_size) WARN_UNUSED_RESULT;

  bool IsValid() const override;

 private:
  ErrnoError WriteImpl(const void* data, size_t size, size_t* nwrite_out) override WARN_UNUSED_RESULT;
  ErrnoError ReadImpl(void* out_data, size_t max_size, size_t* nread_out) override WARN_UNUSED_RESULT;
  virtual ErrnoError SendFileImpl(descriptor_t file_fd, size_t file_size) WARN_UNUSED_RESULT;
  ErrnoError CloseImpl() override WARN_UNUSED_RESULT;
};

}  // namespace net
}  // namespace common
