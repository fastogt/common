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

#include <atomic>
#include <limits>

#include <common/types.h>

namespace common {
namespace patterns {

template <typename T, typename count_t = std::atomic<uint32_t>>
class object_counter {
 public:
  static count_t get_count() { return object_counter<T, count_t>::count_; }

 protected:
  object_counter() { ++object_counter<T, count_t>::count_; }

  explicit object_counter(const object_counter<T>&) { ++object_counter<T, count_t>::count_; }

  object_counter& operator=(const object_counter&) {
    ++object_counter<T, count_t>::count_;
    return *this;
  }

  ~object_counter() { --object_counter<T, count_t>::count_; }

 private:
  static count_t count_;
};

template <typename T, typename count_t>
count_t object_counter<T, count_t>::count_(0);

template <typename T, typename count_t = size_t>
class id_counter : public object_counter<T, count_t> {
 public:
  typedef std::atomic<count_t> atomic_t;
  typedef count_t type_t;

  id_counter() : id_(count_++) {}

  type_t get_id() const { return id_; }

 protected:
  const atomic_t id_;
  static atomic_t count_;
};

template <typename T, typename count_t>
typename id_counter<T, count_t>::atomic_t id_counter<T, count_t>::count_(0);

}  // namespace patterns
}  // namespace common
