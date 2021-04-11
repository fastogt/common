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

#include <common/threads/event.h>

#include <pthread.h>

#include <sys/time.h>  // for timeval, gettimeofday

namespace common {
namespace {

int wait_condition(pthread_cond_t* condition, pthread_mutex_t* mutex, time64_t milliseconds) {
  if (milliseconds != INFINITE_TIMEOUT_MSEC) {
    // Converting from seconds and microseconds (1e-6) plus
    // milliseconds (1e-3) to seconds and nanoseconds (1e-9).
    struct timespec ts;
    struct timeval tv;
    gettimeofday(&tv, nullptr);
    ts.tv_sec = tv.tv_sec + (milliseconds / 1000);
    ts.tv_nsec = tv.tv_usec * 1000 + (milliseconds % 1000) * 1000000;
    // Handle overflow.
    if (ts.tv_nsec >= 1000000000) {
      ts.tv_sec++;
      ts.tv_nsec -= 1000000000;
    }

    return pthread_cond_timedwait(condition, mutex, &ts);
  }

  return pthread_cond_wait(condition, mutex);
}

}  // namespace
namespace threads {

struct Event::event_t {
  event_t() : event_status(0), is_manual_reset(0) {
    pthread_mutex_init(&event_mutex, nullptr);
    pthread_cond_init(&event_cond, nullptr);
  }

  ~event_t() {
    pthread_mutex_destroy(&event_mutex);
    pthread_cond_destroy(&event_cond);
  }

  pthread_mutex_t event_mutex;
  pthread_cond_t event_cond;
  int event_status;
  int is_manual_reset;
};

Event::Event(int manual_reset, int initially_signaled) : event_(new event_t) {
  event_->is_manual_reset = manual_reset;
  event_->event_status = initially_signaled;
}

Event::~Event() {
  delete event_;
}

void Event::Set() {
  pthread_mutex_lock(&event_->event_mutex);
  event_->event_status = 1;
  pthread_cond_broadcast(&event_->event_cond);
  pthread_mutex_unlock(&event_->event_mutex);
}

void Event::Reset() {
  pthread_mutex_lock(&event_->event_mutex);
  event_->event_status = 0;
  pthread_mutex_unlock(&event_->event_mutex);
}

bool Event::Wait(time64_t milliseconds) {
  pthread_mutex_lock(&event_->event_mutex);
  int error = 0;
  while (!event_->event_status && error == 0) {
    error = wait_condition(&event_->event_cond, &event_->event_mutex, milliseconds);
  }
  // NOTE(liulk): Exactly one thread will auto-reset this event. All
  // the other threads will think it's unsignaled. This seems to be
  // consistent with auto-reset events in WEBRTC_WIN
  if (error == 0 && !event_->is_manual_reset) {
    event_->event_status = 0;
  }
  pthread_mutex_unlock(&event_->event_mutex);

  return (error == 0);
}

}  // namespace threads
}  // namespace common
