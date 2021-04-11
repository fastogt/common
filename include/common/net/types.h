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

#include <string>

#include <common/macros.h>

namespace common {
namespace net {

class HostAndPort {
 public:
  typedef std::string host_t;
  typedef uint16_t port_t;

  HostAndPort();
  HostAndPort(host_t host, port_t port);
  bool IsValid() const;
  bool IsLocalHost() const;
  bool IsDefaultRoute() const;

  static HostAndPort CreateLocalHostIPV4(uint16_t port);
  static HostAndPort CreateLocalHostIPV6(uint16_t port);
  static HostAndPort CreateDefaultRouteIPV4(uint16_t port);
  static HostAndPort CreateDefaultRouteIPV6(uint16_t port);

  bool Equals(const HostAndPort& other) const;
  bool IsSameHost(const host_t& host) const;

  host_t GetHost() const;
  void SetHost(host_t host);

  host_t GetHostNoBrackets() const;

  port_t GetPort() const;
  void SetPort(port_t port);

 private:
  host_t host_;
  port_t port_;
};

inline bool operator==(const HostAndPort& left, const HostAndPort& right) {
  return left.Equals(right);
}

inline bool operator!=(const HostAndPort& left, const HostAndPort& right) {
  return !(left == right);
}

class HostAndPortAndSlot : public HostAndPort {
 public:
  typedef uint16_t slot_t;

  HostAndPortAndSlot();
  HostAndPortAndSlot(const std::string& host, uint16_t port, uint16_t slot);

  bool Equals(const HostAndPortAndSlot& other) const;

  slot_t GetSlot() const;
  void SetSlot(slot_t slot);

 private:
  slot_t slot_;
};

inline bool operator==(const HostAndPortAndSlot& left, const HostAndPortAndSlot& right) {
  return left.Equals(right);
}

inline bool operator!=(const HostAndPortAndSlot& left, const HostAndPortAndSlot& right) {
  return !operator==(left, right);
}

std::string StableHost(std::string host);
bool IsLocalHost(const std::string& host);
bool IsDefaultRoute(const std::string& host);
}  // namespace net

std::string ConvertToString(const net::HostAndPort& from);
bool ConvertFromString(const std::string& from, net::HostAndPort* out) WARN_UNUSED_RESULT;

std::string ConvertToString(const net::HostAndPortAndSlot& from);
bool ConvertFromString(const std::string& from, net::HostAndPortAndSlot* out) WARN_UNUSED_RESULT;

}  // namespace common
