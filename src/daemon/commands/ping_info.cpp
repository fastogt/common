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

#include <common/daemon/commands/ping_info.h>

#include <common/time.h>

#define SERVER_INFO_TIMESTAMP_FIELD "timestamp"
#define CLIENT_INFO_TIMESTAMP_FIELD "timestamp"

namespace common {
namespace daemon {
namespace commands {

ServerPingInfo::ServerPingInfo() : timestamp_(common::time::current_utc_mstime()) {}

common::Error ServerPingInfo::SerializeFields(json_object* deserialized) const {
  json_object_object_add(deserialized, SERVER_INFO_TIMESTAMP_FIELD, json_object_new_int64(timestamp_));
  return common::Error();
}

common::Error ServerPingInfo::DoDeSerialize(json_object* serialized) {
  json_object* jtimestamp = nullptr;
  json_bool jtimestamp_exists = json_object_object_get_ex(serialized, SERVER_INFO_TIMESTAMP_FIELD, &jtimestamp);
  ServerPingInfo inf;
  if (jtimestamp_exists) {
    inf.timestamp_ = json_object_get_int64(jtimestamp);
  }

  *this = inf;
  return common::Error();
}

common::time64_t ServerPingInfo::GetTimeStamp() const {
  return timestamp_;
}

void ServerPingInfo::SetTimestamp(time64_t time) {
  timestamp_ = time;
}

bool ServerPingInfo::Equals(const ServerPingInfo& ping) const {
  return timestamp_ == ping.timestamp_;
}

ClientPingInfo::ClientPingInfo() : timestamp_(common::time::current_utc_mstime()) {}

common::Error ClientPingInfo::SerializeFields(json_object* deserialized) const {
  json_object_object_add(deserialized, CLIENT_INFO_TIMESTAMP_FIELD, json_object_new_int64(timestamp_));
  return common::Error();
}

common::Error ClientPingInfo::DoDeSerialize(json_object* serialized) {
  json_object* jtimestamp = nullptr;
  json_bool jtimestamp_exists = json_object_object_get_ex(serialized, CLIENT_INFO_TIMESTAMP_FIELD, &jtimestamp);
  ClientPingInfo inf;
  if (jtimestamp_exists) {
    inf.timestamp_ = json_object_get_int64(jtimestamp);
  }

  *this = inf;
  return common::Error();
}

time64_t ClientPingInfo::GetTimeStamp() const {
  return timestamp_;
}

bool ClientPingInfo::Equals(const ClientPingInfo& ping) const {
  return timestamp_ == ping.timestamp_;
}

}  // namespace commands
}  // namespace daemon
}  // namespace common
