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

#include <common/net/isocket.h>

namespace common {
namespace net {

ErrnoError ISocket::Write(const void* data, size_t size, size_t* nwrite_out) {
  DCHECK(IsValid());
  if (!data || size == 0 || !nwrite_out) {
    return make_errno_error_inval();
  }

  return WriteImpl(data, size, nwrite_out);
}

ErrnoError ISocket::WriteBuffer(const std::string& data, size_t* nwrite_out) {
  DCHECK(IsValid());
  if (data.empty() || !nwrite_out) {
    return make_errno_error_inval();
  }

  return WriteImpl(data.data(), data.size(), nwrite_out);
}

ErrnoError ISocket::Read(void* out_data, size_t max_size, size_t* nread_out) {
  DCHECK(IsValid());
  if (!out_data || max_size == 0 || !nread_out) {
    return make_errno_error_inval();
  }

  return ReadImpl(out_data, max_size, nread_out);
}

ErrnoError ISocket::ReadToBuffer(char_buffer_t* out_data, size_t max_size) {
  DCHECK(IsValid());
  if (!out_data || max_size == 0) {
    return make_errno_error_inval();
  }

  out_data->resize(max_size);
  size_t nread_out;
  ErrnoError err = ReadImpl(out_data->data(), max_size, &nread_out);
  if (err) {
    return err;
  }

  out_data->resize(nread_out);
  return ErrnoError();
}

ErrnoError ISocket::Close() {
  DCHECK(IsValid());
  return CloseImpl();
}

ISocket::~ISocket() {}

}  // namespace net
}  // namespace common
