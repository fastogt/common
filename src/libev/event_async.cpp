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

#include <common/libev/event_async.h>

#include <ev.h>

#include <common/libev/event_loop.h>

namespace common {
namespace libev {

LibevAsync::LibevAsync() : base_class(), loop_(nullptr), func_() {}

LibevAsync::~LibevAsync() {}

void LibevAsync::Init(LibEvLoop* loop, async_loop_exec_function_t cb) {
  if (!loop || !cb) {
    return;
  }

  loop->InitAsync(this, async_callback);
  loop_ = loop;
  func_ = cb;
}

void LibevAsync::Start() {
  if (!loop_) {
    return;
  }

  loop_->StartAsync(this);
}

void LibevAsync::Stop() {
  if (!loop_) {
    return;
  }

  loop_->StopAsync(this);
}

void LibevAsync::Notify() {
  if (!loop_) {
    return;
  }

  loop_->NotifyAsync(this);
}

void LibevAsync::async_callback(struct ev_loop* loop, struct ev_async* watcher, int revents) {
  UNUSED(loop);
  UNUSED(revents);

  LibevAsync* async = reinterpret_cast<LibevAsync*>(watcher->data);
  if (async) {
    async->func_(async->loop_, async, revents);
  }
}

}  // namespace libev
}  // namespace common
