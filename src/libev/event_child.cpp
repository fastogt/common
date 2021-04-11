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

#include <common/libev/event_child.h>

#if defined(OS_POSIX)
#include <sys/wait.h>
#endif

#include <ev.h>

#include <common/libev/event_loop.h>

#if EV_CHILD_ENABLE
using fasto_ev_child = ev_child;
#else
#if defined(OS_WIN)
#include "fasto_ev_child_win.h"
#else
#error "Please implement"
#endif
#endif

namespace common {
namespace libev {

LibevChild::LibevChild() : base_class(), loop_(nullptr), func_(), pid_(INVALID_PROCESS_ID) {}

LibevChild::~LibevChild() {}

bool LibevChild::Init(LibEvLoop* loop, child_loop_exec_function_t cb, process_handle_t pid) {
  if (!loop || !cb || pid == INVALID_PROCESS_ID) {
    return false;
  }

  loop->InitChild(this, child_callback, pid);
  loop_ = loop;
  func_ = cb;
  pid_ = pid;
  return true;
}

void LibevChild::Start() {
  if (!loop_) {
    return;
  }

  loop_->StartChild(this);
}

void LibevChild::Stop() {
  if (!loop_) {
    return;
  }

  loop_->StopChild(this);
}

process_handle_t LibevChild::GetPid() const {
  return pid_;
}

void LibevChild::child_callback(struct ev_loop* loop, fasto_ev_child* watcher, int revents) {
  UNUSED(loop);
  UNUSED(revents);

  LibevChild* child = reinterpret_cast<LibevChild*>(watcher->data);
  if (child) {
    int stabled_status = EXIT_SUCCESS;
    int signal_number = 0;

#if defined(OS_POSIX)
    if (WIFEXITED(watcher->rstatus)) {
      stabled_status = WEXITSTATUS(watcher->rstatus);
    } else {
      stabled_status = EXIT_FAILURE;
    }
    if (WIFSIGNALED(watcher->rstatus)) {
      signal_number = WTERMSIG(watcher->rstatus);
    }
#else
    stabled_status = watcher->rstatus;
#endif
    child->func_(child->loop_, child, stabled_status, signal_number, revents);
  }
}

}  // namespace libev
}  // namespace common
