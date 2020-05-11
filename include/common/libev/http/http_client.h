/*  Copyright (C) 2014-2020 FastoGT. All right reserved.

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

#include <time.h>

#include <vector>

#include <common/http/http.h>

#include <common/uri/gurl.h>

#include <common/libev/http/http_server_info.h>
#include <common/libev/tcp/tcp_client.h>

namespace common {
namespace libev {
namespace http {

class HttpClient : public libev::tcp::TcpClient {
 public:
  HttpClient(libev::IoLoop* server, const net::socket_info& info);

  virtual ErrnoError Get(const uri::GURL& url, bool is_keep_alive) WARN_UNUSED_RESULT;
  virtual ErrnoError Head(const uri::GURL& url, bool is_keep_alive) WARN_UNUSED_RESULT;

  ErrnoError SendOk(common::http::http_protocol protocol,
                    const char* extra_header,
                    const char* text,
                    bool is_keep_alive,
                    const HttpServerInfo& info) WARN_UNUSED_RESULT;
  virtual ErrnoError SendError(common::http::http_protocol protocol,
                               common::http::http_status status,
                               const char* extra_header,
                               const char* text,
                               bool is_keep_alive,
                               const HttpServerInfo& info) WARN_UNUSED_RESULT;
  virtual ErrnoError SendFileByFd(common::http::http_protocol protocol, int fdesc, off_t size) WARN_UNUSED_RESULT;
  virtual ErrnoError SendHeaders(common::http::http_protocol protocol,
                                 common::http::http_status status,
                                 const char* extra_header,
                                 const char* mime_type,
                                 off_t* length,
                                 time_t* mod,
                                 bool is_keep_alive,
                                 const HttpServerInfo& info) WARN_UNUSED_RESULT;
  virtual ErrnoError SendRequest(common::http::http_method method,
                                 const uri::GURL& url,
                                 common::http::http_protocol protocol,
                                 const char* extra_header,
                                 bool is_keep_alive) WARN_UNUSED_RESULT;

  const char* ClassName() const override;

  void SetIsAuthenticated(bool auth);
  bool IsAuthenticated() const;

 private:
  bool isAuth_;
};

}  // namespace http
}  // namespace libev
}  // namespace common
