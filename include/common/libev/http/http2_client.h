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

#include <time.h>

#include <vector>

#include <common/libev/http/http_client.h>

#include <common/libev/http/http_streams.h>

namespace common {
namespace libev {
namespace http {

class Http2Client : public HttpClient {
 public:
  typedef StreamSPtr stream_t;
  typedef std::vector<stream_t> streams_t;

  Http2Client(IoLoop* server, const net::socket_info& info);

  ErrnoError Get(const uri::GURL& url, bool is_keep_alive) override WARN_UNUSED_RESULT;
  ErrnoError Head(const uri::GURL& url, bool is_keep_alive) override WARN_UNUSED_RESULT;

  ErrnoError SendError(common::http::http_protocol protocol,
                       common::http::http_status status,
                       const common::http::headers_t& extra_headers,
                       const char* text,
                       bool is_keep_alive,
                       const HttpServerInfo& info) override WARN_UNUSED_RESULT;
  ErrnoError SendFileByFd(common::http::http_protocol protocol, int fdesc, off_t size) override WARN_UNUSED_RESULT;
  ErrnoError SendHeaders(common::http::http_protocol protocol,
                         common::http::http_status status,
                         const common::http::headers_t& extra_headers,
                         const char* mime_type,
                         off_t* length,
                         time_t* mod,
                         bool is_keep_alive,
                         const HttpServerInfo& info) override WARN_UNUSED_RESULT;

  ErrnoError SendRequest(common::http::http_method method,
                         const uri::GURL& url,
                         common::http::http_protocol protocol,
                         const common::http::headers_t& extra_headers,
                         bool is_keep_alive) override WARN_UNUSED_RESULT;

  void ProcessFrames(const http2::frames_t& frames);

  bool IsSettingNegotiated() const;

  const char* ClassName() const override;

 private:
  bool IsHttp2() const;
  StreamSPtr FindStreamByStreamID(IStream::stream_id_t stream_id) const;
  StreamSPtr FindStreamByType(http2::frame_t type) const;
  streams_t streams_;
};

}  // namespace http
}  // namespace libev
}  // namespace common
