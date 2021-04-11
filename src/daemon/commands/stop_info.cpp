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

#include <common/daemon/commands/stop_info.h>

#define STOP_SERVICE_INFO_DELAY_FIELD "delay"

namespace common {
namespace daemon {
namespace commands {

StopInfo::StopInfo() : StopInfo(0) {}

StopInfo::StopInfo(common::time64_t delay) : base_class(), delay_(delay) {}

common::Error StopInfo::DoDeSerialize(json_object* serialized) {
  StopInfo inf;
  json_object* jdelay = nullptr;
  json_bool jdelay_exists = json_object_object_get_ex(serialized, STOP_SERVICE_INFO_DELAY_FIELD, &jdelay);
  if (jdelay_exists) {
    inf.delay_ = json_object_get_int64(jdelay);
  }

  *this = inf;
  return common::Error();
}

common::Error StopInfo::SerializeFields(json_object* out) const {
  json_object_object_add(out, STOP_SERVICE_INFO_DELAY_FIELD, json_object_new_int64(delay_));
  return common::Error();
}

common::time64_t StopInfo::GetDelay() const {
  return delay_;
}

}  // namespace commands
}  // namespace daemon
}  // namespace common
