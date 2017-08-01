/*  Copyright (C) 2014-2017 FastoGT. All right reserved.

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

#include <stddef.h>  // for NULL
#include <stdint.h>  // for uintptr_t

#include <functional>  // for function

#include <common/macros.h>             // for DNOTREACHED
#include <common/system_info/types.h>  // for lcpu_count_t

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
 protected:
  ThreadBase() : event_(1, 0) {}

  Event event_;
};

template <typename RT>
class ThreadJoinBase : public ThreadBase {
 public:
  typedef RT result_type;
  typedef std::function<result_type()> function_type;

  result_type JoinAndGet() {
    void* res = NULL;
    if (PlatformThread::Join(&handle_, &res)) {
      ThreadBase::event_.Reset();
    }

    return res_;
  }

 protected:
  explicit ThreadJoinBase(const function_type& func) : res_(), handle_(invalid_handle), func_(func) {}

  void Run() { res_ = func_(); }

  result_type res_;
  PlatformThreadHandle handle_;
  const function_type func_;
};

template <>
class ThreadJoinBase<void> : public ThreadBase {
 public:
  typedef void result_type;
  typedef std::function<result_type()> function_type;

 protected:
  explicit ThreadJoinBase(const function_type& func) : handle_(invalid_handle), func_(func) {}

  void Run() { func_(); }

  PlatformThreadHandle handle_;
  const function_type func_;
};

template <typename RT>
class Thread final : public ThreadJoinBase<RT> {
 public:
  friend class ThreadManager;
  typedef ThreadJoinBase<RT> base_class;
  typedef typename base_class::result_type result_type;
  typedef typename base_class::function_type function_type;

  platform_thread_id_t GetTid() const { return base_class::handle_.GetTid(); }

  PlatformThreadHandle GetHandle() const { return base_class::handle_; }

  bool IsRunning() { return ThreadBase::event_.Wait(0) && GetTid() != invalid_tid; }

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
    if (!base_class::func_) {
      return false;
    }

    bool created = PlatformThread::Create(&(base_class::handle_), &thread_start, this, priority_);
    if (!created) {
      DNOTREACHED();
      return false;
    }

    ThreadBase::event_.Set();
    return true;
  }

  void Join() {
    void* res = NULL;
    if (PlatformThread::Join(&(base_class::handle_), &res)) {
      ThreadBase::event_.Reset();
    }
  }

  ~Thread() { Join(); }

 private:
  static void* thread_start(void* arg) {
    Thread* thr = static_cast<Thread*>(arg);
    WrapThread(thr);
    thr->Run();
    UnWrapThread(thr);
    return NULL;
  }

  Thread() : base_class(function_type()), ptr_(0), lcpu_number_(invalid_cpu_count), priority_(PRIORITY_NORMAL) {}
  Thread(const function_type& func, uintptr_t ptr, lcpu_count_t lcpunumber)
      : base_class(func), ptr_(ptr), lcpu_number_(lcpunumber), priority_(PRIORITY_NORMAL) {}

  uintptr_t const ptr_;
  lcpu_count_t lcpu_number_;
  ThreadPriority priority_;
};

}  // namespace threads
}  // namespace common
