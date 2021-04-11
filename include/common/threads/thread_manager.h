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

#include <memory>

#include <common/bind_simple.h>                 // for bind_simple
#include <common/patterns/singleton_pattern.h>  // for TSSingleton
#include <common/system_info/types.h>           // for lcpu_count_t

#include <common/threads/platform_thread.h>  // for PlatformThread, etc
#include <common/threads/thread.h>

namespace common {
namespace threads {

class ThreadManager : public patterns::TSSingleton<ThreadManager> {
 public:
  friend class patterns::TSSingleton<ThreadManager>;

  template <typename Callable, typename T, typename... Args>
  inline std::shared_ptr<Thread<typename std::result_of<Callable(T, Args...)>::type>> CreateThread(Callable&& func,
                                                                                                   T&& t,
                                                                                                   Args&&... args) {
    typedef Thread<typename std::result_of<Callable(T, Args...)>::type> thread_type;
    typedef std::shared_ptr<thread_type> ThreadSPtr;
    thread_type* rthr = new thread_type(std::bind(std::forward<Callable>(func), t, std::forward<Args>(args)...));
    return ThreadSPtr(rthr);
  }

  template <typename Callable, typename... Args>
  inline std::shared_ptr<Thread<typename std::result_of<Callable(Args...)>::type>> CreateThread(Callable&& func,
                                                                                                Args&&... args) {
    typedef Thread<typename std::result_of<Callable(Args...)>::type> thread_type;
    typedef std::shared_ptr<thread_type> ThreadSPtr;
    thread_type* rthr = new thread_type(utils::bind_simple(std::forward<Callable>(func), std::forward<Args>(args)...));
    return ThreadSPtr(rthr);
  }

  template <typename RT>
  Thread<RT>* CurrentThread() const {
    return static_cast<Thread<RT>*>(PlatformThread::GetTlsDataByKey(key_));
  }

  template <typename RT>
  bool IsCurrentThread(Thread<RT>* thr) const {
    return thr == CurrentThread<RT>();
  }

  bool IsMainThread() const { return main_thread_ == CurrentThread<int>(); }

  // for inner use
  template <typename RT>
  void WrapThread(Thread<RT>* thr) {
    PlatformThread::SetTlsDataByKey(key_, thr);
  }

  template <typename RT>
  void UnWrapThread(Thread<RT>*) {}

 private:
  ThreadManager();
  ~ThreadManager();

  platform_tls_t key_;
  Thread<int>* const main_thread_;
};

}  // namespace threads
}  // namespace common

#define THREAD_MANAGER() common::threads::ThreadManager::GetInstance()
