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

#include <common/libev/event_loop.h>

#include <stdlib.h>

#include <ev.h>

#include <mutex>

#include <common/libev/event_async.h>
#include <common/libev/event_child.h>
#include <common/libev/event_io.h>
#include <common/libev/event_timer.h>

#if !EV_CHILD_ENABLE
#if defined(OS_WIN)
#include <windows.h>
#include "fasto_ev_child_win.h"
namespace {
const DWORD kBasicProcessAccess = PROCESS_TERMINATE | PROCESS_QUERY_INFORMATION | SYNCHRONIZE;

static void WINAPI win32_deadchild_callback(PVOID user_data, BOOLEAN) {
  fasto_ev_child* childinfo = static_cast<fasto_ev_child*>(user_data);
  if (!childinfo) {
    return;
  }

  DWORD exitcode;
  childinfo->rstatus = EXIT_FAILURE;
  if (GetExitCodeProcess(childinfo->proc_handle, &exitcode)) {
    childinfo->rstatus = static_cast<int>(exitcode);
  }
  childinfo->loop->ExecInLoopThread([childinfo]() { (childinfo->cb)(childinfo->ev_loop, childinfo, EV_CHILD); });
}
}  // namespace
#else
#error "Please implement"
#endif
#endif

namespace common {
namespace libev {

class LibEvLoop::AsyncCustom : public LibevAsync {
 public:
  typedef std::unique_lock<std::mutex> mutex_lock_t;
  AsyncCustom() : queue_mutex_(), custom_callbacks_() {}
  ~AsyncCustom() { custom_callbacks_.clear(); }
  void Push(custom_loop_exec_function_t func) {
    {
      mutex_lock_t lock(queue_mutex_);
      custom_callbacks_.push_back(func);
    }
    Notify();
  }

  static void custom_cb(LibEvLoop* loop, LibevAsync* async, flags_t revents) {
    UNUSED(loop);
    UNUSED(revents);

    AsyncCustom* custom = static_cast<AsyncCustom*>(async);
    custom->Pop();
  }

 private:
  void Pop() {
    std::vector<custom_loop_exec_function_t> copy;
    {
      mutex_lock_t lock(queue_mutex_);
      custom_callbacks_.swap(copy);
    }

    for (size_t i = 0; i < copy.size(); ++i) {
      custom_loop_exec_function_t func = copy[i];
      func();
    }
  }
  std::mutex queue_mutex_;
  std::vector<custom_loop_exec_function_t> custom_callbacks_;
};

EvLoopObserver::~EvLoopObserver() {}

LibEvLoop::LibEvLoop() : LibEvLoop(ev_loop_new(0)) {}

LibEvLoop::LibEvLoop(struct ev_loop* loop)
    : loop_(loop),
      observer_(nullptr),
      exec_id_(),
      async_stop_(new LibevAsync),
      async_custom_(new AsyncCustom),
      timers_(),
      is_running_(false) {
  CHECK(loop_) << "Must be evloop!";
  ev_set_userdata(loop_, this);
}  // namespace libev

LibEvLoop::~LibEvLoop() {
  destroy(&async_custom_);
  destroy(&async_stop_);
  ev_loop_destroy(loop_);
}

void LibEvLoop::SetObserver(EvLoopObserver* observer) {
  observer_ = observer;
}

timer_id_t LibEvLoop::CreateTimer(double sec, bool repeat) {
  CHECK(IsLoopThread()) << "Must be called in loop thread!";

  LibevTimer* timer = new LibevTimer;
  timer->Init(this, timer_cb, sec, repeat);
  timer->Start();
  timers_.push_back(timer);
  return timer->get_id();
}

void LibEvLoop::RemoveTimer(timer_id_t id) {
  CHECK(IsLoopThread()) << "Must be called in loop thread!";

  for (std::vector<LibevTimer*>::iterator it = timers_.begin(); it != timers_.end(); ++it) {
    LibevTimer* timer = *it;
    if (timer->get_id() == id) {
      timers_.erase(it);
      delete timer;
      return;
    }
  }
}

void LibEvLoop::InitAsync(LibevAsync* as, async_callback_t cb) {
  ev_async* eas = as->GetHandle();
  ev_async_init(eas, cb);
}

void LibEvLoop::StartAsync(LibevAsync* as) {
  CHECK(IsLoopThread()) << "Must be called in loop thread!";
  struct ev_async* eas = as->GetHandle();
  ev_async_start(loop_, eas);
}

void LibEvLoop::StopAsync(LibevAsync* as) {
  CHECK(IsLoopThread()) << "Must be called in loop thread!";
  struct ev_async* eas = as->GetHandle();
  ev_async_stop(loop_, eas);
}

void LibEvLoop::NotifyAsync(LibevAsync* as) {
  struct ev_async* eas = as->GetHandle();
  ev_async_send(loop_, eas);
}

void LibEvLoop::InitIO(LibevIO* io, io_callback_t cb, descriptor_t fd, flags_t events) {
  ev_io* eio = io->GetHandle();
  ev_io_init(eio, cb, fd, events);
}

void LibEvLoop::StartIO(LibevIO* io) {
  CHECK(IsLoopThread()) << "Must be called in loop thread!";
  ev_io* eio = io->GetHandle();
  ev_io_start(loop_, eio);
}

void LibEvLoop::StopIO(LibevIO* io) {
  CHECK(IsLoopThread()) << "Must be called in loop thread!";
  ev_io* eio = io->GetHandle();
  ev_io_stop(loop_, eio);
}

void LibEvLoop::InitTimer(LibevTimer* timer, timer_callback_t cb, double sec, bool repeat) {
  ev_timer* eit = timer->GetHandle();
  ev_timer_init(eit, cb, sec, repeat ? sec : 0);
}

void LibEvLoop::StartTimer(LibevTimer* timer) {
  CHECK(IsLoopThread()) << "Must be called in loop thread!";
  ev_timer* eit = timer->GetHandle();
  ev_timer_start(loop_, eit);
}

void LibEvLoop::StopTimer(LibevTimer* timer) {
  CHECK(IsLoopThread()) << "Must be called in loop thread!";
  ev_timer* eit = timer->GetHandle();
  ev_timer_stop(loop_, eit);
}

void LibEvLoop::InitChild(LibevChild* child, child_callback_t cb, process_handle_t pid) {
  CHECK(IsLoopThread()) << "Must be called in loop thread!";
  fasto_ev_child* eic = child->GetHandle();
#if EV_CHILD_ENABLE
  ev_child_init(eic, cb, pid, 0);
#else
#if defined(OS_WIN)
  eic->proc_handle = pid;
  eic->ev_loop = loop_;
  eic->loop = this;
  eic->cb = cb;
#else
#error "Please implement"
#endif
#endif
}

void LibEvLoop::StartChild(LibevChild* child) {
  CHECK(IsLoopThread()) << "Must be called in loop thread!";
  fasto_ev_child* eic = child->GetHandle();
#if EV_CHILD_ENABLE
  ev_child_start(loop_, eic);
#else
#if defined(OS_WIN)
  RegisterWaitForSingleObject(&eic->wait_handle, eic->proc_handle, win32_deadchild_callback, eic, INFINITE,
                              WT_EXECUTEONLYONCE | WT_EXECUTEINWAITTHREAD);
#else
#error "Please implement"
#endif
#endif
}

void LibEvLoop::StopChild(LibevChild* child) {
  CHECK(IsLoopThread()) << "Must be called in loop thread!";
  fasto_ev_child* eic = child->GetHandle();
#if EV_CHILD_ENABLE
  ev_child_stop(loop_, eic);
#else
#if defined(OS_WIN)
  if (eic->wait_handle) {
    UnregisterWaitEx(eic->wait_handle, nullptr);
    eic->wait_handle = nullptr;
  }
  if (eic->proc_handle) {
    CloseHandle(eic->proc_handle);
    eic->proc_handle = nullptr;
  }
#else
#error "Please implement"
#endif
#endif
}

void LibEvLoop::ExecInLoopThread(custom_loop_exec_function_t func) {
  if (IsLoopThread()) {
    func();
    return;
  }

  async_custom_->Push(func);
}

int LibEvLoop::Exec() {
  exec_id_ = threads::PlatformThread::GetCurrentId();

  async_custom_->Init(this, AsyncCustom::custom_cb);
  async_custom_->Start();
  async_stop_->Init(this, stop_cb);
  async_stop_->Start();
  if (observer_) {
    observer_->PreLooped(this);
  }
  is_running_ = true;
  Start();
  ev_loop(loop_, 0);
  is_running_ = false;
  if (observer_) {
    observer_->PostLooped(this);
  }
  return EXIT_SUCCESS;
}

bool LibEvLoop::IsLoopThread() const {
  return exec_id_ == threads::PlatformThread::GetCurrentId();
}

bool LibEvLoop::IsRunning() const {
  CHECK(IsLoopThread()) << "Must be called in loop thread!";
  return is_running_;
}

void LibEvLoop::Stop() {
  async_stop_->Notify();
}

void LibEvLoop::Start() {
  HandleStart();
}

void LibEvLoop::stop_cb(LibEvLoop* loop, LibevAsync* async, flags_t revents) {
  UNUSED(async);
  UNUSED(revents);

  loop->HandleStop();
}

void LibEvLoop::timer_cb(LibEvLoop* loop, LibevTimer* timer, flags_t revents) {
  if (EV_ERROR & revents) {
    DNOTREACHED();
    return;
  }

  DCHECK(revents & EV_TIMEOUT);
  loop->HandleTimer(timer->get_id());
}

void LibEvLoop::HandleTimer(timer_id_t id) {
  if (observer_) {
    observer_->TimerEmited(this, id);
  }
}

void LibEvLoop::HandleStart() {
  if (observer_) {
    observer_->Started(this);
  }
}

void LibEvLoop::HandleStop() {
  async_stop_->Stop();
  const std::vector<LibevTimer*> timers = timers_;
  for (size_t i = 0; i < timers.size(); ++i) {
    LibevTimer* timer = timers[i];
    RemoveTimer(timer->get_id());
  }

  if (observer_) {
    observer_->Stopped(this);
  }
  ev_unloop(loop_, EVUNLOOP_ONE);
}

}  // namespace libev
}  // namespace common
