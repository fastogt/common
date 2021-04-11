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

#include <common/time.h>

#include <common/serializer/json_serializer.h>

namespace common {
namespace daemon {
namespace commands {

class ServerPingInfo : public common::serializer::JsonSerializer<ServerPingInfo> {
 public:
  ServerPingInfo();

  common::time64_t GetTimeStamp() const;
  void SetTimestamp(common::time64_t time);

  bool Equals(const ServerPingInfo& ping) const;

 protected:
  common::Error DoDeSerialize(json_object* serialized) override;
  common::Error SerializeFields(json_object* deserialized) const override;

 private:
  common::time64_t timestamp_;  // utc time
};

inline bool operator==(const ServerPingInfo& lhs, const ServerPingInfo& rhs) {
  return lhs.Equals(rhs);
}

inline bool operator!=(const ServerPingInfo& x, const ServerPingInfo& y) {
  return !(x == y);
}

class ClientPingInfo : public common::serializer::JsonSerializer<ClientPingInfo> {
 public:
  ClientPingInfo();

  common::time64_t GetTimeStamp() const;

  bool Equals(const ClientPingInfo& ping) const;

 protected:
  common::Error DoDeSerialize(json_object* serialized) override;
  common::Error SerializeFields(json_object* deserialized) const override;

 private:
  common::time64_t timestamp_;  // utc time
};

inline bool operator==(const ClientPingInfo& lhs, const ClientPingInfo& rhs) {
  return lhs.Equals(rhs);
}

inline bool operator!=(const ClientPingInfo& x, const ClientPingInfo& y) {
  return !(x == y);
}

}  // namespace commands
}  // namespace daemon
}  // namespace common
