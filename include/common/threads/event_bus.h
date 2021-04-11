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

#include <atomic>
#include <condition_variable>
#include <deque>   // for deque
#include <memory>  // for shared_ptr, __shared_ptr

#include <common/threads/event_dispatcher.h>
#include <common/threads/thread_manager.h>  // for ThreadManager, etc

namespace common {
namespace threads {

template <typename type_t>
class EventThread {
  typedef Thread<int> event_thread_t;

 public:
  friend class EventBus;

  typedef event_traits<type_t> etraits_t;
  typedef typename etraits_t::event_t event_t;
  typedef typename etraits_t::listener_t listener_t;
  static const events_size_t max_events_count = etraits_t::max_count;
  static const identifier_t id = etraits_t::id;
  typedef std::unique_lock<std::mutex> mutex_lock_t;

  ~EventThread() { thread_->Join(); }

  void Subscribe(listener_t* listener, events_size_t id) { dispatcher_.Subscribe(listener, id); }

  void UnSubscribe(listener_t* listener, events_size_t id) { dispatcher_.UnSubscribe(listener, id); }

 private:
  EventThread()
      : dispatcher_(),
        thread_(THREAD_MANAGER()->CreateThread(&EventThread::Exec, this)),
        stop_(false),
        queue_mutex_(),
        events_(),
        condition_() {}

  void PostEvent(event_t* event) {
    if (IsCurrentThread(thread_.get())) {
      dispatcher_.ProcessEvent(event);
    } else {
      mutex_lock_t lock(queue_mutex_);
      events_.push_back(event);
      condition_.notify_one();
    }
  }

  bool Start() { return thread_->Start(); }

  void Stop() {
    CHECK(!IsCurrentThread(thread_.get()));
    mutex_lock_t lock(queue_mutex_);
    stop_ = true;
    condition_.notify_one();
  }

  typename event_thread_t::result_type join() { return thread_->JoinAndGet(); }

 private:
  int Exec() {
    while (!stop_.load()) {
      event_t* event = nullptr;
      {
        mutex_lock_t lock(queue_mutex_);
        condition_.wait(lock);
        if (!stop_.load()) {
          event = events_.front();
          events_.pop_front();
        }
      }
      dispatcher_.ProcessEvent(event);
    }

    {
      mutex_lock_t lock(queue_mutex_);
      for (size_t i = 0; i < events_.size(); ++i) {
        delete events_[i];
      }
      events_.clear();
    }

    return 1;
  }

  EventDispatcher<type_t> dispatcher_;

  std::shared_ptr<event_thread_t> thread_;
  std::atomic_bool stop_;

  std::mutex queue_mutex_;
  std::deque<event_t*> events_;
  std::condition_variable condition_;
};

class EventBus : public patterns::TSSingleton<EventBus> {
 public:
  enum : identifier_t { max_events_loop = 10 };
  friend class patterns::TSSingleton<EventBus>;

  template <typename ev_t>
  void Subscribe(IListenerEx<typename ev_t::type_t>* listener) {
    if (stop_.load()) {
      return;
    }

    EventThread<typename ev_t::type_t>* thr = GetThread<typename ev_t::type_t>();
    if (!thr) {
      return;
    }

    thr->Subscribe(listener, ev_t::EventType);
  }

  template <typename ev_t>
  void UnSubscribe(typename event_traits<typename ev_t::type_t>::listener_t* listener) {
    if (stop_.load()) {
      return;
    }

    EventThread<typename ev_t::type_t>* thr = GetThread<typename ev_t::type_t>();
    if (!thr) {
      return;
    }

    thr->UnSubscribe(listener, ev_t::EventType);
  }

  template <typename type_t>
  void PostEvent(IEventEx<type_t>* event) {
    if (stop_.load()) {
      return;
    }

    EventThread<type_t>* thr = GetThread<type_t>();
    if (!thr) {
      return;
    }

    thr->PostEvent(event);
  }

  void Stop() {
    stop_ = true;
    for (identifier_t i = 0; i < max_events_loop; ++i) {
      if (registered_threads_[i]) {
        registered_threads_[i] = nullptr;
      }
    }
  }

  template <typename type_t>
  void StopEventThread(EventThread<type_t>* thread) {
    if (stop_.load()) {
      return;
    }

    if (!thread) {
      return;
    }

    thread->Stop();
  }

  template <typename type_t>
  EventThread<type_t>* CreateEventThread() {
    if (stop_.load()) {
      return nullptr;
    }

    EventThread<type_t>* thread = new EventThread<type_t>();
    RegisterThread(thread);
    return thread;
  }

  template <typename type_t>
  void JoinEventThread(EventThread<type_t>* thread) {
    if (stop_.load()) {
      return;
    }

    if (!thread) {
      return;
    }

    thread->join();
  }

 private:
  template <typename type_t>
  EventThread<type_t>* GetThread() {
    typedef event_traits<type_t> etraits_t;
    if (etraits_t::id >= max_events_loop) {
      return nullptr;
    }

    return static_cast<EventThread<type_t>*>(registered_threads_[etraits_t::id]);
  }

  template <typename type_t>
  void StartEventThread(EventThread<type_t>* thread) {
    if (stop_.load()) {
      return;
    }

    if (!thread) {
      return;
    }

    thread->Start();
  }

  template <typename type_t>
  void DestroyEventThread(EventThread<type_t>* thread) {
    if (stop_.load()) {
      return;
    }

    if (!thread) {
      return;
    }

    StopEventThread(thread);
    JoinEventThread(thread);
    UnRegisterThread(thread);
  }

  template <typename type_t>
  void RegisterThread(EventThread<type_t>* thread) {
    typedef event_traits<type_t> etraits_t;
    if (etraits_t::id >= max_events_loop) {
      return;
    }

    registered_threads_[etraits_t::id] = thread;
  }

  template <typename type_t>
  void UnRegisterThread(EventThread<type_t>* thread) {
    UNUSED(thread);
    typedef event_traits<type_t> etraits_t;
    if (etraits_t::id >= max_events_loop) {
      return;
    }

    registered_threads_[etraits_t::id] = nullptr;
  }

  void* registered_threads_[max_events_loop];
  std::atomic_bool stop_;

  EventBus() : stop_(false) {
    for (identifier_t i = 0; i < max_events_loop; ++i) {
      registered_threads_[i] = nullptr;
    }
  }

  ~EventBus() { CHECK(stop_ == true); }
};

}  // namespace threads
}  // namespace common

#define EVENT_BUS() common::threads::EventBus::GetInstance()
