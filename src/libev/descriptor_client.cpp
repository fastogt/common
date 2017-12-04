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

#include <common/libev/descriptor_client.h>

namespace common {
namespace libev {

DescriptorClient::DescriptorClient(IoLoop* server, descriptor_t fd, flags_t flags)
    : IoClient(server, flags), desc_(fd) {}

DescriptorClient::~DescriptorClient() {}

const char* DescriptorClient::ClassName() const {
  return "DescriptorClient";
}

descriptor_t DescriptorClient::GetFd() const {
  return desc_.GetFd();
}

Error DescriptorClient::Write(const char* data, size_t size, size_t* nwrite) {
  if (!data || !size || !nwrite) {
    return make_error_inval();
  }

  size_t total = 0;          // how many bytes we've sent
  size_t bytes_left = size;  // how many we have left to send

  while (total < size) {
    size_t n;
    ErrnoError err = desc_.Write(data, size, &n);
    if (err) {
      return make_error(err->GetDescription());
    }
    total += n;
    bytes_left -= n;
  }

  *nwrite = total;  // return number actually sent here
  return Error();
}

Error DescriptorClient::Write(const unsigned char* data, size_t size, size_t* nwrite) {
  if (!data || !size || !nwrite) {
    return make_error_inval();
  }

  size_t total = 0;          // how many bytes we've sent
  size_t bytes_left = size;  // how many we have left to send

  while (total < size) {
    size_t n;
    ErrnoError err = desc_.Write(data, size, &n);
    if (err) {
      return make_error(err->GetDescription());
    }
    total += n;
    bytes_left -= n;
  }

  *nwrite = total;  // return number actually sent here
  return Error();
}

Error DescriptorClient::Read(char* out, size_t size, size_t* nread) {
  if (!out || !size || !nread) {
    return make_error_inval();
  }

  size_t total = 0;          // how many bytes we've readed
  size_t bytes_left = size;  // how many we have left to read

  while (total < size) {
    size_t n;
    ErrnoError err = desc_.Read(out + total, bytes_left, &n);
    if (err) {
      return make_error(err->GetDescription());
    }
    total += n;
    bytes_left -= n;
  }

  *nread = total;  // return number actually readed here
  return Error();
}

Error DescriptorClient::Read(unsigned char* out, size_t size, size_t* nread) {
  if (!out || !size || !nread) {
    return make_error_inval();
  }

  size_t total = 0;          // how many bytes we've readed
  size_t bytes_left = size;  // how many we have left to read

  while (total < size) {
    size_t n;
    ErrnoError err = desc_.Read(out + total, bytes_left, &n);
    if (err) {
      return make_error(err->GetDescription());
    }
    total += n;
    bytes_left -= n;
  }

  *nread = total;  // return number actually readed here
  return Error();
}

Error DescriptorClient::CloseImpl() {
  ErrnoError err = desc_.Close();
  if (err) {
    return make_error(err->GetDescription());
  }

  return Error();
}

}  // namespace libev
}  // namespace common
