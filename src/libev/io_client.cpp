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

#include <common/libev/io_client.h>

#include <common/libev/event_io.h>
#include <common/libev/io_loop.h>

namespace common {
namespace libev {

IoClient::IoClient(IoLoop* server, flags_t flags)
    : base_class(), server_(server), read_write_io_(new LibevIO), flags_(flags), wrote_bytes_(), read_bytes_() {
  read_write_io_->SetUserData(this);
}

IoClient::~IoClient() {
  destroy(&read_write_io_);
}

ErrnoError IoClient::Close() {
  if (server_) {
    server_->CloseClient(this);
  }
  return DoClose();
}

IoLoop* IoClient::GetServer() const {
  return server_;
}

flags_t IoClient::GetFlags() const {
  return flags_;
}

size_t IoClient::GetWroteBytes() const {
  return wrote_bytes_;
}

size_t IoClient::GetReadBytes() const {
  return read_bytes_;
}

const char* IoClient::ClassName() const {
  return "IoClient";
}

ErrnoError IoClient::Write(const void* data, size_t size, size_t* nwrite_out) {
  if (!data || !size || !nwrite_out) {
    return make_errno_error_inval();
  }

  size_t total = 0;          // how many bytes we've sent
  size_t bytes_left = size;  // how many we have left to send

  while (total < size) {
    size_t n;
    ErrnoError err = SingleWrite(data, size, &n);
    if (err || n == 0) {
      *nwrite_out = 0;
      return err;
    }
    total += n;
    bytes_left -= n;
  }

  *nwrite_out = total;  // return number actually sent here
  return ErrnoError();
}

ErrnoError IoClient::Read(void* out_data, size_t max_size, size_t* nread_out) {
  if (!out_data || !max_size || !nread_out) {
    return make_errno_error_inval();
  }

  size_t total = 0;              // how many bytes we've readed
  size_t bytes_left = max_size;  // how many we have left to read

  while (total < max_size) {
    size_t n;
    ErrnoError err = SingleRead(static_cast<char*>(out_data) + total, bytes_left, &n);
    if (err || n == 0) {
      *nread_out = 0;
      return err;
    }
    total += n;
    bytes_left -= n;
  }

  CHECK_EQ(total, max_size);
  *nread_out = total;  // return number actually readed here
  return ErrnoError();
}

ErrnoError IoClient::SingleWrite(const void* data, size_t size, size_t* nwrite_out) {
  if (!data || !size || !nwrite_out) {
    return make_errno_error_inval();
  }

  ErrnoError err = DoSingleWrite(data, size, nwrite_out);
  if (!err) {
    wrote_bytes_ += *nwrite_out;
  }
  return err;
}

ErrnoError IoClient::SingleRead(void* out_data, size_t max_size, size_t* nread_out) {
  if (!out_data || !max_size || !nread_out) {
    return make_errno_error_inval();
  }

  ErrnoError err = DoSingleRead(out_data, max_size, nread_out);
  if (!err) {
    read_bytes_ += *nread_out;
  }
  return err;
}

}  // namespace libev
}  // namespace common
