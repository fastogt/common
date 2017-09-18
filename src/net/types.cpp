/*  Copyright (C) 2014-2017 FastoGT. All right reserved.

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

#include <common/net/types.h>

#include <sstream>

#include <common/convert2string.h>  // for ConvertFromString

namespace {
const char localhost_text[] = "localhost";
const char localhost_digits[] = "127.0.0.1";
}  // namespace

namespace common {
namespace net {

HostAndPort::HostAndPort() : host_(), port_(0) {}

HostAndPort::HostAndPort(host_t host, port_t port) : host_(), port_(port) {
  SetHost(host);
}

bool HostAndPort::IsValid() const {
  return !host_.empty();
}

bool HostAndPort::IsLocalHost() const {
  return net::IsLocalHost(host_);
}

std::string StableHost(std::string host) {
  std::transform(host.begin(), host.end(), host.begin(), ::tolower);
  return host;
}

bool IsLocalHost(const std::string& host) {
  if (host.empty()) {
    return false;
  }

  COMPILE_ASSERT(SIZEOFMASS(localhost_text) == SIZEOFMASS(localhost_digits),
                 "Size of constat should be same, because 1 condition.");
  if (host.size() != SIZEOFMASS(localhost_text) - 1) {
    return false;
  }

  return host == localhost_text || host == localhost_digits;
}

HostAndPort HostAndPort::CreateLocalHost(uint16_t port) {
  return HostAndPort(localhost_text, port);
}

bool HostAndPort::Equals(const HostAndPort& other) const {
  return host_ == other.host_ && port_ == other.port_;
}

HostAndPort::host_t HostAndPort::GetHost() const {
  return host_;
}

void HostAndPort::SetHost(host_t host) {
  host_ = StableHost(host);
}

HostAndPort::port_t HostAndPort::GetPort() const {
  return port_;
}

void HostAndPort::SetPort(port_t port) {
  port_ = port;
}

HostAndPortAndSlot::HostAndPortAndSlot() : HostAndPort(), slot_(0) {}

HostAndPortAndSlot::HostAndPortAndSlot(const std::string& host, uint16_t port, uint16_t slot)
    : HostAndPort(host, port), slot_(slot) {}

bool HostAndPortAndSlot::Equals(const HostAndPortAndSlot& other) const {
  return HostAndPort::Equals(other) && slot_ == other.slot_;
}

HostAndPortAndSlot::slot_t HostAndPortAndSlot::GetSlot() const {
  return slot_;
}

void HostAndPortAndSlot::SetSlot(slot_t slot) {
  slot_ = slot;
}

}  // namespace net

std::string ConvertToString(const net::HostAndPort& host) {
  if (!host.IsValid()) {
    return std::string();
  }

  std::ostringstream bw;
  bw << host.GetHost() << ":" << host.GetPort();
  return bw.str();
}

bool ConvertFromString(const std::string& from, net::HostAndPort* out) {
  if (!out) {
    return false;
  }

  net::HostAndPort res;
  size_t del = from.find_first_of(':');
  if (del != std::string::npos) {
    res.SetHost(from.substr(0, del));
    uint16_t lport;
    bool ok = ConvertFromString(from.substr(del + 1), &lport);
    if (ok) {
      res.SetPort(lport);
    }
  }

  *out = res;
  return true;
}

std::string ConvertToString(const net::HostAndPortAndSlot& host) {
  if (!host.IsValid()) {
    return std::string();
  }

  std::ostringstream bw;
  bw << host.GetHost() << ":" << host.GetPort() << "@" << host.GetSlot();
  return bw.str();
}

bool ConvertFromString(const std::string& from, net::HostAndPortAndSlot* out) {
  if (!out) {
    return false;
  }

  net::HostAndPortAndSlot lout;
  size_t del = from.find_first_of(':');
  if (del != std::string::npos) {
    lout.SetHost(from.substr(0, del));
    size_t del_s = from.find_first_of('@');
    if (del_s != std::string::npos) {
      uint16_t lport;
      bool res = ConvertFromString(from.substr(del + 1, del_s - del - 1), &lport);
      if (res) {
        lout.SetPort(lport);
      }

      uint16_t lslot;
      res = ConvertFromString(from.substr(del_s + 1), &lslot);
      if (res) {
        lout.SetSlot(lslot);
      }
    } else {
      uint16_t lport;
      bool res = ConvertFromString(from.substr(del + 1), &lport);
      if (res) {
        lout.SetPort(lport);
      }
    }
  }

  *out = lout;
  return true;
}

}  // namespace common
