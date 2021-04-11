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

#include <string>

#include <common/error.h>  // for ErrnoError, Error
#include <common/types.h>

namespace common {
namespace file_system {

class DescriptorHolder {
 public:
  explicit DescriptorHolder(descriptor_t fd);
  ~DescriptorHolder();

  bool IsValid() const;
  descriptor_t GetFd() const;

  ErrnoError Write(const void* data, size_t size, size_t* nwrite_out) WARN_UNUSED_RESULT;
  ErrnoError Write(const char* data, size_t size, size_t* nwrite_out) WARN_UNUSED_RESULT;
  ErrnoError WriteBuffer(const std::string& data, size_t* nwrite_out) WARN_UNUSED_RESULT;
  ErrnoError WriteBuffer(const char_buffer_t& data, size_t* nwrite_out) WARN_UNUSED_RESULT;
  ErrnoError Read(void* out_data, size_t max_size, size_t* nread_out) WARN_UNUSED_RESULT;

  ErrnoError Close() WARN_UNUSED_RESULT;

  ErrnoError Lock() WARN_UNUSED_RESULT;
  ErrnoError Unlock() WARN_UNUSED_RESULT;

  ErrnoError Seek(off_t offset, int whence) WARN_UNUSED_RESULT;

  ErrnoError Truncate(off_t pos) WARN_UNUSED_RESULT;

 private:
  descriptor_t fd_;
  DISALLOW_COPY_AND_ASSIGN(DescriptorHolder);
};

template <typename Descriptor>
class DescriptorGuard : public Descriptor {
 public:
  typedef Descriptor base_class;

  template <typename... Args>
  explicit DescriptorGuard(Args... args) : base_class(args...) {}

  ~DescriptorGuard() {
    ErrnoError err = base_class::Close();
    DCHECK(!err) << "Close client error: " << err->GetDescription();
  }

 private:
  using base_class::Close;
  DISALLOW_COPY_AND_ASSIGN(DescriptorGuard);
};

}  // namespace file_system
}  // namespace common
