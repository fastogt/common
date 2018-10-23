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
