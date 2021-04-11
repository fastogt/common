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

#include <common/libev/pipe_client.h>

#include <unistd.h>

#if defined(OS_WIN)
#include <fcntl.h>
#define pipe(fds) _pipe(fds, 4096, O_BINARY)
#endif

namespace common {
namespace libev {

PipeReadClient::PipeReadClient(IoLoop* server, descriptor_t fd, flags_t flags) : DescriptorClient(server, fd, flags) {}

PipeReadClient::~PipeReadClient() {}

const char* PipeReadClient::ClassName() const {
  return "PipeReadClient";
}

PipeWriteClient::PipeWriteClient(IoLoop* server, descriptor_t fd, flags_t flags)
    : DescriptorClient(server, fd, flags) {}

PipeWriteClient::~PipeWriteClient() {}

const char* PipeWriteClient::ClassName() const {
  return "PipeWriteClient";
}

ErrnoError CreatePipe(PipeReadClient** read_client, PipeWriteClient** write_client, IoLoop* server) {
  if (!read_client || !write_client) {
    return make_errno_error_inval();
  }

  int pipefd[2] = {0};
  int res = pipe(pipefd);
  if (res == ERROR_RESULT_VALUE) {
    return make_errno_error(errno);
  }

  *read_client = new PipeReadClient(server, pipefd[0]);
  *write_client = new PipeWriteClient(server, pipefd[1]);
  return ErrnoError();
}

}  // namespace libev
}  // namespace common
