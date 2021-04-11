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

#include <algorithm>
#include <mutex>
#include <vector>

#include <common/event.h>  // for event_traits, events_size_t, etc

namespace common {
namespace threads {

class DynamicEventDispatcher {
 public:
  typedef IEvent event_t;
  typedef IListener listener_t;
  typedef std::unique_lock<std::mutex> mutex_lock_t;
  typedef std::vector<listener_t*> listener_t_array_t;

  void Subscribe(listener_t* listener, events_size_t id);
  void UnSubscribe(listener_t* listener, events_size_t id);
  void UnSubscribe(listener_t* listener);

  DynamicEventDispatcher(events_size_t evt);
  ~DynamicEventDispatcher();

  void ProcessEvent(event_t* event);

 private:
  const size_t max_events_count_;
  std::mutex listeners_mutex_;
  listener_t_array_t* listeners_;
};

template <typename type_t>
class EventDispatcher {
 public:
  typedef event_traits<type_t> etraits_t;
  typedef typename etraits_t::event_t event_t;
  typedef typename etraits_t::ex_event_t ex_event_t;
  typedef typename etraits_t::listener_t listener_t;
  static const events_size_t max_events_count = etraits_t::max_count;
  static const identifier_t id = etraits_t::id;
  typedef std::unique_lock<std::mutex> mutex_lock_t;

  void Subscribe(listener_t* listener, events_size_t id) {
    if (!listener) {
      return;
    }

    events_size_t pos = id;
    if (pos >= max_events_count) {
      DNOTREACHED();
      return;
    }

    mutex_lock_t lock(listeners_mutex_);
    listeners_[pos].push_back(listener);
  }

  void UnSubscribe(listener_t* listener, events_size_t id) {
    if (!listener) {
      return;
    }

    events_size_t pos = id;
    if (pos >= max_events_count) {
      DNOTREACHED();
      return;
    }

    mutex_lock_t lock(listeners_mutex_);
    listeners_[pos].erase(std::remove(listeners_[pos].begin(), listeners_[pos].end(), listener), listeners_[pos].end());
  }

  void UnSubscribe(listener_t* listener) {
    if (!listener) {
      return;
    }

    mutex_lock_t lock(listeners_mutex_);
    for (size_t i = 0; i < max_events_count; ++i) {
      listeners_[i].erase(std::remove(listeners_[i].begin(), listeners_[i].end(), listener), listeners_[i].end());
    }
  }

  EventDispatcher() {}
  ~EventDispatcher() {}

  void ProcessEvent(event_t* event) {
    if (!event) {
      return;
    }

    events_size_t pos = event->GetEventType();
    if (pos > max_events_count) {
      destroy(&event);
      return;
    }

    if (pos == max_events_count) {
      ex_event_t* ex_event = static_cast<ex_event_t*>(event);
      event_t* levent = ex_event->GetEvent();
      events_size_t lpos = levent->GetEventType();
      mutex_lock_t lock(listeners_mutex_);
      for (size_t i = 0; i < listeners_[lpos].size(); ++i) {
        listener_t* listener = listeners_[lpos][i];
        listener->HandleExceptionEvent(levent, ex_event->GetError());
      }
      destroy(&ex_event);
      return;
    }

    mutex_lock_t lock(listeners_mutex_);
    for (size_t i = 0; i < listeners_[pos].size(); ++i) {
      listener_t* listener = listeners_[pos][i];
      listener->HandleEvent(event);
    }

    destroy(&event);
  }

 private:
  std::mutex listeners_mutex_;
  std::vector<listener_t*> listeners_[max_events_count];
};

}  // namespace threads
}  // namespace common
