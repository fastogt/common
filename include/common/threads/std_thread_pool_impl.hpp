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

#include <vector>

#include <common/threads/types.h>
#include <common/types.h>

namespace common {
namespace threads {

template <typename task_t>
class std_thread_pool_impl {
 public:
  typedef task_t task_type;
  std_thread_pool_impl() : stop_(false) {}

  ~std_thread_pool_impl() { Stop(); }

  void Post(task_type task) {
    {
      lock_guard<mutex> lock(queue_mutex);
      tasks.push_back(task);
    }
    condition.notify_one();
  }

  void Start(uint16_t count_threads) { init_work(count_threads); }

  void Stop() {
    stop_ = true;
    condition.notify_all();
    wait_finish_work();
  }

  void Restart() {
    Stop();
    InitWork(workers.size());
  }

 private:
  void InitWork(uint16_t threads) {
    workers.clear();
    tasks.clear();
    for (uint16_t i = 0; i < threads; ++i) {
      workers.push_back(thread_t(std::bind(&std_thread_pool_impl::run_work, this)));
    }
  }

  void WaitFinishWork() {
    for (uint16_t i = 0; i < workers.size(); ++i) {
      workers[i].join();
    }
  }

  void RunWork() {
    task_type task;
    while (true) {
      {
        unique_lock<mutex> lock(queue_mutex);
        while (!stop_.load() && tasks.empty()) {
          condition.wait(lock);
        }
        if (stop_) {
          return;
        }
        task = tasks.back();
        tasks.pop_back();
      }
      task();
    }
  }

  std::vector<thread> workers;
  std::vector<task_type> tasks;
  mutex queue_mutex;
  condition_variable condition;
  atomic_bool stop_;
};

}  // namespace threads
}  // namespace common
