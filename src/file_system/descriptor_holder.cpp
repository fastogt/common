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

#include <common/file_system/descriptor_holder.h>

#include <common/file_system/file_system.h>

namespace common {
namespace file_system {

DescriptorHolder::DescriptorHolder(descriptor_t fd) : fd_(fd) {}

DescriptorHolder::~DescriptorHolder() {}

bool DescriptorHolder::IsValid() const {
  return fd_ != INVALID_DESCRIPTOR;
}

descriptor_t DescriptorHolder::GetFd() const {
  DCHECK(IsValid());
  return fd_;
}

ErrnoError DescriptorHolder::Write(const void* data, size_t size, size_t* nwrite_out) {
  DCHECK(IsValid());
  return write_to_descriptor(fd_, data, size, nwrite_out);
}

ErrnoError DescriptorHolder::Write(const char* data, size_t size, size_t* nwrite_out) {
  DCHECK(IsValid());
  return write_to_descriptor(fd_, data, size, nwrite_out);
}

ErrnoError DescriptorHolder::WriteBuffer(const std::string& data, size_t* nwrite_out) {
  DCHECK(IsValid());
  return write_to_descriptor(fd_, data.data(), data.size(), nwrite_out);
}

ErrnoError DescriptorHolder::WriteBuffer(const char_buffer_t& data, size_t* nwrite_out) {
  DCHECK(IsValid());
  return write_to_descriptor(fd_, data.data(), data.size(), nwrite_out);
}

ErrnoError DescriptorHolder::Read(void* out_data, size_t max_size, size_t* nread_out) {
  DCHECK(IsValid());
  return read_from_descriptor(fd_, out_data, max_size, nread_out);
}

ErrnoError DescriptorHolder::Close() {
  return close_descriptor(fd_);
}

ErrnoError DescriptorHolder::Lock() {
  DCHECK(IsValid());
  return lock_descriptor(fd_);
}

ErrnoError DescriptorHolder::Unlock() {
  DCHECK(IsValid());
  return unlock_descriptor(fd_);
}

ErrnoError DescriptorHolder::Seek(off_t offset, int whence) {
  DCHECK(IsValid());
  return seek_descriptor(fd_, offset, whence);
}

ErrnoError DescriptorHolder::Truncate(off_t pos) {
  DCHECK(IsValid());
  return Ftruncate(fd_, pos);
}

}  // namespace file_system
}  // namespace common
