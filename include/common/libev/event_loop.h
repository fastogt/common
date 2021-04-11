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

#include <vector>

#include <common/threads/platform_thread.h>

#include <common/libev/types.h>

struct ev_loop;
struct ev_io;
struct ev_timer;
struct ev_async;

namespace common {
namespace libev {

class EvLoopObserver {
 public:
  virtual ~EvLoopObserver();

  virtual void PreLooped(LibEvLoop* loop) = 0;
  virtual void Started(LibEvLoop* loop) = 0;
  virtual void Stopped(LibEvLoop* loop) = 0;
  virtual void PostLooped(LibEvLoop* loop) = 0;
  virtual void TimerEmited(LibEvLoop* loop, timer_id_t id) = 0;
};

class LibEvLoop {
 public:
  typedef void io_callback_t(struct ev_loop* loop, struct ev_io* watcher, int revents);
  typedef void async_callback_t(struct ev_loop* loop, struct ev_async* watcher, int revents);
  typedef void timer_callback_t(struct ev_loop* loop, struct ev_timer* watcher, int revents);
  typedef void child_callback_t(struct ev_loop* loop, fasto_ev_child* watcher, int revents);

  LibEvLoop();
  virtual ~LibEvLoop();

  void SetObserver(EvLoopObserver* observer);

  timer_id_t CreateTimer(double sec, bool repeat);
  void RemoveTimer(timer_id_t id);

  // async
  void InitAsync(LibevAsync* as, async_callback_t cb);
  void StartAsync(LibevAsync* as);
  void StopAsync(LibevAsync* as);
  void NotifyAsync(LibevAsync* as);

  // io
  void InitIO(LibevIO* io, io_callback_t cb, descriptor_t fd, flags_t events);
  void StartIO(LibevIO* io);
  void StopIO(LibevIO* io);

  // timer
  void InitTimer(LibevTimer* timer, timer_callback_t cb, double sec, bool repeat);
  void StartTimer(LibevTimer* timer);
  void StopTimer(LibevTimer* timer);

  void InitChild(LibevChild* child, child_callback_t cb, process_handle_t pid);
  void StartChild(LibevChild* child);
  void StopChild(LibevChild* child);

  void ExecInLoopThread(custom_loop_exec_function_t func);

  int Exec() WARN_UNUSED_RESULT;
  void Stop();

  bool IsLoopThread() const;
  bool IsRunning() const;  // can be called only in LoopThread

 protected:
  explicit LibEvLoop(struct ev_loop* loop);

 private:
  void Start();
  class AsyncCustom;

  static void stop_cb(LibEvLoop* loop, LibevAsync* async, flags_t revents);
  static void timer_cb(LibEvLoop* loop, LibevTimer* timer, flags_t revents);

  void HandleStart();
  void HandleStop();
  void HandleTimer(timer_id_t id);

  struct ev_loop* loop_;
  EvLoopObserver* observer_;
  threads::platform_thread_id_t exec_id_;
  LibevAsync* async_stop_;

  AsyncCustom* async_custom_;

  std::vector<LibevTimer*> timers_;
  bool is_running_;
};

}  // namespace libev
}  // namespace common
