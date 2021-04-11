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

#include <common/threads/event_dispatcher.h>

namespace common {
namespace threads {

void DynamicEventDispatcher::Subscribe(listener_t* listener, events_size_t id) {
  if (!listener) {
    return;
  }

  events_size_t pos = id;
  if (pos >= max_events_count_) {
    DNOTREACHED();
    return;
  }

  mutex_lock_t lock(listeners_mutex_);
  listeners_[pos].push_back(listener);
}

void DynamicEventDispatcher::UnSubscribe(listener_t* listener, events_size_t id) {
  if (!listener) {
    return;
  }

  events_size_t pos = id;
  if (pos >= max_events_count_) {
    DNOTREACHED();
    return;
  }

  mutex_lock_t lock(listeners_mutex_);
  listeners_[pos].erase(std::remove(listeners_[pos].begin(), listeners_[pos].end(), listener), listeners_[pos].end());
}

void DynamicEventDispatcher::UnSubscribe(listener_t* listener) {
  if (!listener) {
    return;
  }

  mutex_lock_t lock(listeners_mutex_);
  for (size_t i = 0; i < listeners_->size(); ++i) {
    listeners_[i].erase(std::remove(listeners_[i].begin(), listeners_[i].end(), listener), listeners_[i].end());
  }
}

DynamicEventDispatcher::DynamicEventDispatcher(events_size_t evt) : max_events_count_(evt), listeners_(nullptr) {
  listeners_ = new listener_t_array_t[max_events_count_];
}

DynamicEventDispatcher::~DynamicEventDispatcher() {
  listeners_->clear();
  delete[] listeners_;
}

void DynamicEventDispatcher::ProcessEvent(event_t* event) {
  if (!event) {
    return;
  }

  events_size_t pos = event->GetEventID();
  if (pos >= max_events_count_) {
    DNOTREACHED();
    return;
  }

  mutex_lock_t lock(listeners_mutex_);
  for (size_t i = 0; i < listeners_[pos].size(); ++i) {
    listener_t* listener = listeners_[pos][i];
    listener->HandleEvent(event);
  }

  delete event;
}
}  // namespace threads
}  // namespace common
