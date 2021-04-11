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

#include <common/threads/thread_pool.h>

namespace common {
namespace threads {

ThreadPool::ThreadPool() : workers_(), tasks_(), queue_mutex_(), condition_(), stop_(false) {}

ThreadPool::~ThreadPool() {}

void ThreadPool::Post(task_t task) {
  {
    std::lock_guard<std::mutex> lock(queue_mutex_);
    tasks_.push(task);
  }
  condition_.notify_one();
}

void ThreadPool::Start(size_t count_threads) {
  InitWork(count_threads);
}

void ThreadPool::Stop() {
  stop_ = true;
  condition_.notify_all();
  WaitFinishWork();
}

void ThreadPool::Restart() {
  Stop();
  InitWork(workers_.size());
}

void ThreadPool::InitWork(size_t threads) {
  workers_.clear();
  tasks_t q;
  tasks_.swap(q);
  for (uint16_t i = 0; i < threads; ++i) {
    workers_.push_back(thread_t(&ThreadPool::RunWork, this));
  }
}

void ThreadPool::WaitFinishWork() {
  for (uint16_t i = 0; i < workers_.size(); ++i) {
    workers_[i].join();
  }
}

void ThreadPool::RunWork() {
  task_t task;
  while (true) {
    {
      std::unique_lock<std::mutex> lock(queue_mutex_);
      while (!stop_ && tasks_.empty()) {
        condition_.wait(lock);
      }
      if (stop_) {
        return;
      }
      task = tasks_.front();
      tasks_.pop();
    }
    task();
  }
}
}  // namespace threads
}  // namespace common
