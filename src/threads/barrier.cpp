/*  Copyright (C) 2015-2017 Setplex. All right reserved.
    This file is part of Rixjob.
    Rixjob is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    Rixjob is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with Rixjob.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <common/threads/barrier.h>

namespace common {
namespace threads {

barrier::barrier(size_t count) : threshold_(count), count_(count), generation_(0) {}

void barrier::Wait() {
  std::unique_lock<std::mutex> lock(mutex_);
  size_t gen = generation_;

  if (--count_ == 0) {
    generation_++;
    count_ = threshold_;
    cond_.notify_all();
  } else {
    cond_.wait(lock, [this, gen] { return gen != generation_; });
  }
}

}  // namespace threads
}  // namespace common
