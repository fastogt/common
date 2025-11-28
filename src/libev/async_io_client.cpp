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

#include <common/file_system/file_system.h>
#include <common/libev/async_io_client.h>
#include <common/libev/event_io.h>
#include <common/libev/io_loop.h>
#include <common/libev/io_loop_observer.h>

#include <cstring>
#include <algorithm>

namespace common {
namespace libev {

AsyncIoClient::AsyncIoClient(IoLoop* server, flags_t flags)
    : base_class(),
      server_(server),
      read_write_io_(new LibevIO),
      flags_(flags),
      wrote_bytes_(0),
      read_bytes_(0),
      write_buffer_(),
      read_buffer_(),
      read_requested_size_(0),
      is_writing_(false) {
  read_write_io_->SetUserData(this);
}

AsyncIoClient::~AsyncIoClient() {
  destroy(&read_write_io_);
}

ErrnoError AsyncIoClient::Close() {
  if (server_) {
    server_->CloseAsyncClient(this);
  }
  return DoClose();
}

IoLoop* AsyncIoClient::GetServer() const {
  return server_;
}

flags_t AsyncIoClient::GetFlags() const {
  return flags_;
}

size_t AsyncIoClient::GetWroteBytes() const {
  return wrote_bytes_;
}

size_t AsyncIoClient::GetReadBytes() const {
  return read_bytes_;
}

const char* AsyncIoClient::ClassName() const {
  return "AsyncIoClient";
}

ErrnoError AsyncIoClient::Write(const void* data, size_t size) {
  if (!data || !size) {
    return make_errno_error_inval();
  }

  // Buffer the data
  write_buffer_.insert(write_buffer_.end(), static_cast<const char*>(data),
                       static_cast<const char*>(data) + size);

  // Start writing if not already
  StartAsyncWriteIfNeeded();

  return ErrnoError();
}

ErrnoError AsyncIoClient::Read(size_t size) {
  if (!size) {
    return make_errno_error_inval();
  }

  read_requested_size_ = size;

  // If we already have enough data, notify immediately
  if (read_buffer_.size() >= size) {
    OnReadCompleted(size);
    read_requested_size_ = 0;
  }

  return ErrnoError();
}

size_t AsyncIoClient::GetAvailableReadData() const {
  return read_buffer_.size();
}

ErrnoError AsyncIoClient::ReadAvailable(void* out_data, size_t max_size, size_t* nread_out) {
  if (!out_data || !max_size || !nread_out) {
    return make_errno_error_inval();
  }

  size_t to_read = std::min(max_size, read_buffer_.size());
  if (to_read > 0) {
    memcpy(out_data, read_buffer_.data(), to_read);
    read_buffer_.erase(read_buffer_.begin(), read_buffer_.begin() + to_read);
    *nread_out = to_read;
    read_bytes_ += to_read;
  } else {
    *nread_out = 0;
  }

  return ErrnoError();
}

ErrnoError AsyncIoClient::SendFile(descriptor_t file_fd, off_t offset, size_t file_size) {
  if (file_fd == INVALID_DESCRIPTOR) {
    return make_error_perror("SendFile", EINVAL);
  }

  return DoSendFile(file_fd, offset, file_size);
}


void AsyncIoClient::HandleReadEvent() {
  ErrnoError err = DoAsyncRead();
  if (err) {
    // Handle error - perhaps close or notify
    return;
  }

  // Check if we have enough data for the requested read
  if (read_requested_size_ > 0 && read_buffer_.size() >= read_requested_size_) {
    OnReadCompleted(read_requested_size_);
    read_requested_size_ = 0;
  }
}

void AsyncIoClient::HandleWriteEvent() {
  if (write_buffer_.empty()) {
    is_writing_ = false;
    UpdateIoFlags();
    return;
  }

  ErrnoError err = DoAsyncWrite();
  if (err && err->GetErrorCode() != EAGAIN && err->GetErrorCode() != EWOULDBLOCK) {
    // Handle error
    return;
  }

  if (write_buffer_.empty()) {
    is_writing_ = false;
    OnWriteCompleted(wrote_bytes_);  // Could track per operation
    UpdateIoFlags();
  }
}

void AsyncIoClient::StartAsyncWriteIfNeeded() {
  if (!is_writing_ && !write_buffer_.empty()) {
    is_writing_ = true;
    UpdateIoFlags();
    // The next EV_WRITE event will trigger HandleWriteEvent
  }
}

void AsyncIoClient::OnWriteCompleted(size_t bytes_written) {
  if (server_ && server_->GetObserver()) {
    server_->GetObserver()->AsyncDataWriteCompleted(this, bytes_written);
  }
}

void AsyncIoClient::OnReadCompleted(size_t bytes_read) {
  if (server_ && server_->GetObserver()) {
    server_->GetObserver()->AsyncDataReadCompleted(this, bytes_read);
  }
}

void AsyncIoClient::UpdateIoFlags() {
  // Simplified: just update flags, assume IoLoop handles re-registration if needed
  flags_t new_flags = EV_READ;
  if (!write_buffer_.empty() || is_writing_) {
    new_flags |= EV_WRITE;
  }
  flags_ = new_flags;
}

}  // namespace libev
}  // namespace common