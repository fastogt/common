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

#include <common/libev/event_timer.h>

#include <ev.h>

#include <common/libev/event_loop.h>

namespace common {
namespace libev {

LibevTimer::LibevTimer() : base_class(), loop_(nullptr), func_() {}

LibevTimer::~LibevTimer() {}

void LibevTimer::Init(LibEvLoop* loop, timer_loop_exec_function_t cb, double sec, bool repeat) {
  if (!loop || !cb) {
    return;
  }

  loop->InitTimer(this, timer_callback, sec, repeat);
  loop_ = loop;
  func_ = cb;
}

void LibevTimer::Start() {
  if (!loop_) {
    return;
  }

  loop_->StartTimer(this);
}

void LibevTimer::Stop() {
  if (!loop_) {
    return;
  }

  loop_->StopTimer(this);
}

void LibevTimer::timer_callback(struct ev_loop* loop, struct ev_timer* watcher, int revents) {
  UNUSED(loop);
  UNUSED(revents);

  LibevTimer* timer = reinterpret_cast<LibevTimer*>(watcher->data);
  if (timer) {
    timer->func_(timer->loop_, timer, revents);
  }
}

}  // namespace libev
}  // namespace common
