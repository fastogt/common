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

#include <condition_variable>
#include <mutex>
#include <queue>

namespace common {
namespace threads {

template <typename T>
class ts_queue {
 public:
  typedef std::queue<T> queue_t;
  ts_queue() : stop_(false) {}
  ~ts_queue() {}

  void Stop() {
    {
      std::unique_lock<std::mutex> lock(queue_mutex_);
      stop_ = true;
    }
    condition_.notify_all();
  }

  void Push(T task) {
    {
      std::unique_lock<std::mutex> lock(queue_mutex_);
      queue_.push(task);
    }
    condition_.notify_one();
  }

  bool IsEmpty() {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    return queue_.empty();
  }

  void Clear() {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    queue_t q;
    queue_.swap(q);
  }

  bool Pop(T* t) {
    std::unique_lock<std::mutex> lock(queue_mutex_);
    while (!stop_ && queue_.empty()) {
      condition_.wait(lock);
    }
    if (stop_) {
      return false;
    }
    *t = queue_.front();
    queue_.pop();
    return true;
  }

 private:
  std::condition_variable condition_;
  std::mutex queue_mutex_;
  queue_t queue_;
  bool stop_;
};

}  // namespace threads
}  // namespace common
