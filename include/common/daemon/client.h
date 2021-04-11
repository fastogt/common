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

#pragma once

#include <common/libev/tcp/tcp_client.h>  // for TcpClient

namespace common {
namespace daemon {

class DaemonClient : public libev::tcp::TcpClient {
 public:
  typedef libev::tcp::TcpClient base_class;

  DaemonClient(libev::IoLoop* server, const net::socket_info& info);

  bool IsVerified() const;
  void SetVerified(bool verified, time64_t exp_time);

  bool IsExpired() const;

  bool HaveFullAccess() const;

  const char* ClassName() const override;

 private:
  bool is_verified_;
  time64_t exp_time_;
};

}  // namespace daemon
}  // namespace common
