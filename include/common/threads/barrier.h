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

#pragma once

#include <condition_variable>
#include <mutex>

namespace common {
namespace threads {

class barrier {
 public:
  explicit barrier(unsigned int count);

  bool Wait();

 private:
  std::mutex mutex_;
  std::condition_variable cond_;
  const unsigned int threshold_;
  unsigned int count_;
  unsigned int generation_;
};

}  // namespace threads
}  // namespace common
