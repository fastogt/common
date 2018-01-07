/*  Copyright (C) 2014-2018 FastoGT. All right reserved.

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

#include <mutex>

#include <ev.h>

#include <common/libev/event_async.h>
#include <common/libev/event_child.h>
#include <common/libev/event_io.h>
#include <common/libev/event_timer.h>

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
      timers_()
#if LIBEV_CHILD_ENABLE
      ,
      childs_()
#endif
      ,
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
  CHECK(IsLoopThread());

  LibevTimer* timer = new LibevTimer;
  timer->Init(this, timer_cb, sec, repeat);
  timer->Start();
  timers_.push_back(timer);
  return timer->get_id();
}

void LibEvLoop::RemoveTimer(timer_id_t id) {
  CHECK(IsLoopThread());

  for (std::vector<LibevTimer*>::iterator it = timers_.begin(); it != timers_.end(); ++it) {
    LibevTimer* timer = *it;
    if (timer->get_id() == id) {
      timers_.erase(it);
      delete timer;
      return;
    }
  }
}

#if LIBEV_CHILD_ENABLE
void LibEvLoop::RegisterChild(pid_t pid) {
  CHECK(IsLoopThread());

  LibevChild* ch = new LibevChild;
  ch->Init(this, child_cb, pid);
  ch->Start();
  childs_.push_back(ch);
}

void LibEvLoop::RemoveChild(pid_t pid) {
  CHECK(IsLoopThread());

  for (std::vector<LibevChild*>::iterator it = childs_.begin(); it != childs_.end(); ++it) {
    LibevChild* ch = *it;
    if (ch->GetPid() == pid) {
      childs_.erase(it);
      delete ch;
      return;
    }
  }
}
#endif

void LibEvLoop::InitAsync(LibevAsync* as, async_callback_t cb) {
  ev_async* eas = as->GetHandle();
  ev_async_init(eas, cb);
}

void LibEvLoop::StartAsync(LibevAsync* as) {
  CHECK(IsLoopThread());
  struct ev_async* eas = as->GetHandle();
  ev_async_start(loop_, eas);
}

void LibEvLoop::StopAsync(LibevAsync* as) {
  CHECK(IsLoopThread());
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
  CHECK(IsLoopThread());
  ev_io* eio = io->GetHandle();
  ev_io_start(loop_, eio);
}

void LibEvLoop::StopIO(LibevIO* io) {
  CHECK(IsLoopThread());
  ev_io* eio = io->GetHandle();
  ev_io_stop(loop_, eio);
}

void LibEvLoop::InitTimer(LibevTimer* timer, timer_callback_t cb, double sec, bool repeat) {
  ev_timer* eit = timer->GetHandle();
  ev_timer_init(eit, cb, sec, repeat ? sec : 0);
}

void LibEvLoop::StartTimer(LibevTimer* timer) {
  CHECK(IsLoopThread());
  ev_timer* eit = timer->GetHandle();
  ev_timer_start(loop_, eit);
}

void LibEvLoop::StopTimer(LibevTimer* timer) {
  CHECK(IsLoopThread());
  ev_timer* eit = timer->GetHandle();
  ev_timer_stop(loop_, eit);
}

#if LIBEV_CHILD_ENABLE
void LibEvLoop::InitChild(LibevChild* child, child_callback_t cb, pid_t pid) {
  CHECK(IsLoopThread());
  ev_child* eic = child->GetHandle();
  ev_child_init(eic, cb, pid, 0);
}

void LibEvLoop::StartChild(LibevChild* child) {
  CHECK(IsLoopThread());
  ev_child* eic = child->GetHandle();
  ev_child_start(loop_, eic);
}

void LibEvLoop::StopChild(LibevChild* child) {
  CHECK(IsLoopThread());
  ev_child* eic = child->GetHandle();
  ev_child_stop(loop_, eic);
}
#endif

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
  CHECK(IsLoopThread());
  return is_running_;
}

void LibEvLoop::Stop() {
  async_stop_->Notify();
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

#if LIBEV_CHILD_ENABLE
void LibEvLoop::child_cb(LibEvLoop* loop, LibevChild* child, int status, flags_t revents) {
  if (EV_ERROR & revents) {
    DNOTREACHED();
    return;
  }

  DCHECK(revents & EV_CHILD);
  loop->HandleChild(child->GetPid(), status);
}
#endif

void LibEvLoop::HandleTimer(timer_id_t id) {
  if (observer_) {
    observer_->TimerEmited(this, id);
  }
}

#if LIBEV_CHILD_ENABLE
void LibEvLoop::HandleChild(pid_t id, int status) {
  if (observer_) {
    observer_->ChildStatusChanged(this, id, status);
  }
}
#endif

void LibEvLoop::HandleStop() {
  async_stop_->Stop();
  const std::vector<LibevTimer*> timers = timers_;
  for (size_t i = 0; i < timers.size(); ++i) {
    LibevTimer* timer = timers[i];
    RemoveTimer(timer->get_id());
  }
#if LIBEV_CHILD_ENABLE
  const std::vector<LibevChild*> childs = childs_;
  for (size_t i = 0; i < childs.size(); ++i) {
    LibevChild* child = childs[i];
    RemoveChild(child->GetPid());
  }
#endif

  if (observer_) {
    observer_->Stoped(this);
  }
  ev_unloop(loop_, EVUNLOOP_ONE);
}

}  // namespace libev
}  // namespace common
