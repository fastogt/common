/*  Copyright (C) 2014-2021 FastoGT. All right reserved.
    This file is part of sniffer.
    sniffer is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    sniffer is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with sniffer.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace common {
namespace threads {

class ThreadPool {
 public:
  typedef std::thread thread_t;
  typedef std::vector<thread_t> workers_t;
  typedef std::function<void()> task_t;
  typedef std::queue<task_t> tasks_t;
  ThreadPool();
  ~ThreadPool();

  void Post(task_t task);
  void Start(size_t count_threads);
  void Stop();
  void Restart();

 private:
  void InitWork(size_t threads);
  void WaitFinishWork();

  void RunWork();

  workers_t workers_;
  tasks_t tasks_;
  std::mutex queue_mutex_;
  std::condition_variable condition_;
  bool stop_;
};
}  // namespace threads
}  // namespace common
