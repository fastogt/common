/*  Copyright (C) 2014-2021 FastoGT. All right reserved.
    This file is part of fastocloud.
    fastocloud is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    fastocloud is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with fastocloud.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <common/daemon/client.h>

#include <common/time.h>

namespace common {
namespace daemon {

DaemonClient::DaemonClient(libev::IoLoop* server, const net::socket_info& info)
    : base_class(server, info), is_verified_(false), exp_time_(0) {}

bool DaemonClient::IsVerified() const {
  return is_verified_;
}

void DaemonClient::SetVerified(bool verified, time64_t exp_time) {
  is_verified_ = verified;
  exp_time_ = exp_time;
}

bool DaemonClient::IsExpired() const {
  return exp_time_ < time::current_utc_mstime();
}

bool DaemonClient::HaveFullAccess() const {
  return IsVerified() && !IsExpired();
}

const char* DaemonClient::ClassName() const {
  return "DaemonClient";
}

}  // namespace daemon
}  // namespace common
