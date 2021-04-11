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

#include <common/libev/event_io.h>

#include <ev.h>

#include <common/libev/event_loop.h>

namespace common {
namespace libev {

LibevIO::LibevIO() : base_class(), loop_(nullptr), func_(), fd_(INVALID_DESCRIPTOR), events_(0) {}

LibevIO::~LibevIO() {}

descriptor_t LibevIO::GetFd() const {
  return fd_;
}

flags_t LibevIO::GetEvents() const {
  return events_;
}

void LibevIO::SetEvents(int events) {
  ev_io_set(GetHandle(), fd_, events);
  events_ = events;
}

bool LibevIO::Init(LibEvLoop* loop, io_loop_exec_function_t cb, descriptor_t fd, flags_t events) {
  if (!loop || !cb || fd == INVALID_DESCRIPTOR) {
    return false;
  }

  loop->InitIO(this, io_callback, fd, events);
  loop_ = loop;
  func_ = cb;
  fd_ = fd;
  events_ = events;
  return true;
}

void LibevIO::Start() {
  if (!loop_) {
    return;
  }

  loop_->StartIO(this);
}

void LibevIO::Stop() {
  if (!loop_) {
    return;
  }

  loop_->StopIO(this);
}

void LibevIO::io_callback(struct ev_loop* loop, struct ev_io* watcher, int revents) {
  UNUSED(loop);
  UNUSED(revents);

  LibevIO* io = reinterpret_cast<LibevIO*>(watcher->data);
  if (io) {
    io->func_(io->loop_, io, revents);
  }
}

}  // namespace libev
}  // namespace common
