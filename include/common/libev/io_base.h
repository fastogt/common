/*  Copyright (C) 2014-2022 FastoGT. All right reserved.

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

#include <common/patterns/crtp_pattern.h>
#include <common/sprintf.h>
#include <common/time.h>

namespace common {
namespace libev {

template <typename T>
class IoBase : IMetaClassInfo {
 public:
  typedef patterns::id_counter<T> id_t;
  explicit IoBase() : name_(), id_(), created_time_(time::current_utc_mstime()) {}
  virtual ~IoBase() {}

  void SetName(const std::string& name) { name_ = name; }
  std::string GetName() const { return name_; }

  time64_t GetCreatedTime() const { return created_time_; }
  time64_t GetTTL() const { return time::current_utc_mstime() - created_time_; }

  typename id_t::type_t GetId() const { return id_.get_id(); }

  virtual const char* ClassName() const override = 0;
  virtual std::string GetFormatedName() const { return MemSPrintf("[%s][%s(%zu)]", GetName(), ClassName(), GetId()); }

 private:
  std::string name_;
  const id_t id_;
  const time64_t created_time_;
};

}  // namespace libev
}  // namespace common
