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

#include <stdint.h>  // for uintptr_t

#include <functional>  // for function

#include <common/threads/event.h>            // for Event
#include <common/threads/platform_thread.h>  // for PlatformThread, etc

namespace common {
namespace threads {

template <typename RT>
class Thread;

template <typename RT>
void WrapThread(Thread<RT>* thread);
template <typename RT>
void UnWrapThread(Thread<RT>* thread);
template <typename RT>
bool IsCurrentThread(Thread<RT>* thread);

class ThreadBase {
 public:
  PlatformThreadHandle GetHandle() const { return handle_; }

 protected:
  ThreadBase() : handle_() {}

  PlatformThreadHandle handle_;
};

template <typename RT>
class Thread final : public ThreadBase {
 public:
  friend class ThreadManager;
  typedef ThreadBase base_class;
  typedef RT result_type;
  typedef std::function<result_type()> function_type;

  bool IsRunning() { return event_.Wait(0) && base_class::handle_.GetTid() != invalid_tid; }

  // Sets the thread's priority. Must be called before start().
  ThreadPriority GetPriority() const { return priority_; }

  bool SetPriority(ThreadPriority priority) {
    if (IsRunning()) {
      return false;
    }

    priority_ = priority;
    return true;
  }

  bool Start() WARN_UNUSED_RESULT {
    if (!func_) {
      return false;
    }

    bool created = PlatformThread::Create(&(base_class::handle_), &thread_start, this, priority_);
    if (!created) {
      DNOTREACHED();
      return false;
    }

    event_.Set();
    return true;
  }

  void Join() {
    void* res = nullptr;
    if (PlatformThread::Join(&(handle_), &res)) {
      event_.Reset();
    }
  }

  result_type JoinAndGet() {
    void* res = nullptr;
    if (PlatformThread::Join(&handle_, &res)) {
      event_.Reset();
    }

    return res_;
  }

 private:
  static void* thread_start(void* arg) {
    Thread* thr = static_cast<Thread*>(arg);
    WrapThread(thr);
    thr->Run();
    UnWrapThread(thr);
    return nullptr;
  }

  Thread() : res_(), func_(function_type()), event_(1, 0), priority_(PRIORITY_NORMAL) {}
  Thread(function_type func) : res_(), func_(func), event_(1, 0), priority_(PRIORITY_NORMAL) {}

  void Run() { res_ = func_(); }

  result_type res_;
  const function_type func_;
  Event event_;
  ThreadPriority priority_;
};

template <>
class Thread<void> : public ThreadBase {
 public:
  friend class ThreadManager;
  typedef ThreadBase base_class;
  typedef void result_type;
  typedef std::function<result_type()> function_type;

  bool IsRunning() { return event_.Wait(0) && base_class::handle_.GetTid() != invalid_tid; }

  // Sets the thread's priority. Must be called before start().
  ThreadPriority GetPriority() const { return priority_; }

  bool SetPriority(ThreadPriority priority) {
    if (IsRunning()) {
      return false;
    }

    priority_ = priority;
    return true;
  }

  bool Start() WARN_UNUSED_RESULT {
    if (!func_) {
      return false;
    }

    bool created = PlatformThread::Create(&(base_class::handle_), &thread_start, this, priority_);
    if (!created) {
      DNOTREACHED();
      return false;
    }

    event_.Set();
    return true;
  }

  void Join() {
    void* res = nullptr;
    if (PlatformThread::Join(&(handle_), &res)) {
      event_.Reset();
    }
  }

 private:
  static void* thread_start(void* arg) {
    Thread* thr = static_cast<Thread*>(arg);
    WrapThread(thr);
    thr->Run();
    UnWrapThread(thr);
    return nullptr;
  }

  Thread() : func_(function_type()), event_(1, 0), priority_(PRIORITY_NORMAL) {}
  Thread(function_type func) : func_(func), event_(1, 0), priority_(PRIORITY_NORMAL) {}

  void Run() { func_(); }

  const function_type func_;
  Event event_;
  ThreadPriority priority_;
};

}  // namespace threads
}  // namespace common
