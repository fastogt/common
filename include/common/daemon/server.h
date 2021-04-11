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

#include <common/libev/tcp/tcp_server.h>

namespace common {
namespace daemon {

class DaemonServer : public libev::tcp::TcpServer {
 public:
  typedef libev::tcp::TcpServer base_class;
  explicit DaemonServer(const common::net::HostAndPort& host,
                        bool is_default,
                        libev::IoLoopObserver* observer = nullptr);

 private:
  libev::tcp::TcpClient* CreateClient(const common::net::socket_info& info) override;
};

}  // namespace daemon
}  // namespace common
