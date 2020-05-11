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

#include <common/libev/http/http_client.h>

#include <errno.h>
#include <inttypes.h>

#include <string>

#include <common/convert2string.h>
#include <common/file_system/file_system.h>
#include <common/logger.h>
#include <common/net/net.h>
#include <common/sprintf.h>

#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"

namespace {

const char* HTML_PATTERN_ISISSSS7 =
    R"(<!DOCTYPE html>
  <html>
  <head>
  <meta http-equiv="Content-type" content="text/html;charset=UTF-8">
  <title>%d %s</title>
  </head>
  <body bgcolor="#cc9999">
  <h4>%d %s</h4>%s<hr>
  <address><a href="%s">%s</a></address>
  </body>
  </html>)";

}  // namespace

namespace common {
namespace libev {
namespace http {

HttpClient::HttpClient(IoLoop* server, const net::socket_info& info) : TcpClient(server, info), isAuth_(false) {}

const char* HttpClient::ClassName() const {
  return "HttpClient";
}

void HttpClient::SetIsAuthenticated(bool auth) {
  isAuth_ = auth;
}

bool HttpClient::IsAuthenticated() const {
  return isAuth_;
}

ErrnoError HttpClient::Get(const uri::GURL& url, bool is_keep_alive) {
  return SendRequest(common::http::HM_GET, url, common::http::HP_1_1, nullptr, is_keep_alive);
}

ErrnoError HttpClient::Head(const uri::GURL& url, bool is_keep_alive) {
  return SendRequest(common::http::HM_HEAD, url, common::http::HP_1_1, nullptr, is_keep_alive);
}

ErrnoError HttpClient::SendOk(common::http::http_protocol protocol,
                              const char* extra_header,
                              const char* text,
                              bool is_keep_alive,
                              const HttpServerInfo& info) {
  return SendError(protocol, common::http::HS_OK, extra_header, text, is_keep_alive, info);
}

ErrnoError HttpClient::SendError(common::http::http_protocol protocol,
                                 common::http::http_status status,
                                 const char* extra_header,
                                 const char* text,
                                 bool is_keep_alive,
                                 const HttpServerInfo& info) {
  CHECK(protocol <= common::http::HP_1_1);
  const std::string title = ConvertToString(status);

  char err_data[1024] = {0};
  off_t err_len = SNPrintf(err_data, sizeof(err_data), HTML_PATTERN_ISISSSS7, status, title, status, title, text,
                           info.server_url, info.server_name);
  ErrnoError err = SendHeaders(protocol, status, extra_header, "text/html", &err_len, nullptr, is_keep_alive, info);
  if (err) {
    DEBUG_MSG_ERROR(err, logging::LOG_LEVEL_ERR);
  }

  size_t nwrite = 0;
  err = Write(err_data, err_len, &nwrite);
  if (err) {
    DEBUG_MSG_ERROR(err, logging::LOG_LEVEL_ERR);
  }
  return err;
}

ErrnoError HttpClient::SendFileByFd(common::http::http_protocol protocol, int fdesc, off_t size) {
  CHECK(protocol <= common::http::HP_1_1);
  return net::send_file_to_fd(GetFd(), fdesc, 0, size);
}

ErrnoError HttpClient::SendHeaders(common::http::http_protocol protocol,
                                   common::http::http_status status,
                                   const char* extra_header,
                                   const char* mime_type,
                                   off_t* length,
                                   time_t* mod,
                                   bool is_keep_alive,
                                   const HttpServerInfo& info) {
  CHECK(protocol <= common::http::HP_1_1);
  const std::string title = ConvertToString(status);

  time_t now = time(nullptr);
  char timebuf[100];
  strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&now));

  char header_data[1024] = {0};
  int cur_pos = SNPrintf(header_data, sizeof(header_data),
                         protocol == common::http::HP_2_0 ? HTTP_2_0_PROTOCOL_NAME
                             " %d %s\r\n"
                             "Server: %s\r\n"
                             "Date: %s\r\n"
                                                          : HTTP_1_1_PROTOCOL_NAME
                             " %d %s\r\n"
                             "Server: %s\r\n"
                             "Date: %s\r\n",
                         status, title, info.server_name, timebuf);

  if (extra_header) {
    int exlen = SNPrintf(header_data + cur_pos, sizeof(header_data) - cur_pos, "%s\r\n", extra_header);
    cur_pos += exlen;
  }
  if (mime_type) {
    int mim_t = SNPrintf(header_data + cur_pos, sizeof(header_data) - cur_pos, "Content-Type: %s\r\n", mime_type);
    cur_pos += mim_t;
  }
  if (length) {
    int len =
        SNPrintf(header_data + cur_pos, sizeof(header_data) - cur_pos, "Content-Length: %" PRId32 "\r\n", *length);
    cur_pos += len;
  }
  if (mod) {
    strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(mod));
    int mlen = SNPrintf(header_data + cur_pos, sizeof(header_data) - cur_pos, "Last-Modified: %s\r\n", timebuf);
    cur_pos += mlen;
  }

  if (!is_keep_alive) {
#define CONNECTION_CLOSE "Connection: close\r\n\r\n"
    const int last_len = sizeof(CONNECTION_CLOSE) - 1;
    memcpy(header_data + cur_pos, CONNECTION_CLOSE, last_len);
    cur_pos += last_len;
  } else {
#define CONNECTION_KEEP_ALIVE "Keep-Alive: timeout=15, max=100\r\n\r\n"
    const int last_len = sizeof(CONNECTION_KEEP_ALIVE) - 1;
    memcpy(header_data + cur_pos, CONNECTION_KEEP_ALIVE, last_len);
    cur_pos += last_len;
  }

  DCHECK(strlen(header_data) == static_cast<size_t>(cur_pos));
  size_t nwrite = 0;
  return Write(header_data, cur_pos, &nwrite);
}

ErrnoError HttpClient::SendRequest(common::http::http_method method,
                                   const uri::GURL& url,
                                   common::http::http_protocol protocol,
                                   const char* extra_header,
                                   bool is_keep_alive) {
  CHECK(protocol <= common::http::HP_1_1);

  if (!url.is_valid()) {
    return make_errno_error_inval();
  }

  if (!url.SchemeIsHTTPOrHTTPS()) {
    return make_errno_error_inval();
  }

  const std::string method_str = ConvertToString(method);
  time_t now = time(nullptr);
  char timebuf[100];
  strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&now));

  std::string host = url.HostNoBrackets();
  std::string path = url.PathForRequest();
  char header_data[1024] = {0};
  int cur_pos = SNPrintf(header_data, sizeof(header_data),
                         protocol == common::http::HP_2_0 ? "%s %s " HTTP_2_0_PROTOCOL_NAME
                                                            "\r\n"
                                                            "Host: %s\r\n"
                                                            "Date: %s\r\n"
                                                          : "%s %s " HTTP_1_1_PROTOCOL_NAME
                                                            "\r\n"
                                                            "Host: %s\r\n"
                                                            "Date: %s\r\n",
                         method_str, path, host, timebuf);

  if (extra_header) {
    int exlen = SNPrintf(header_data + cur_pos, sizeof(header_data) - cur_pos, "%s\r\n", extra_header);
    cur_pos += exlen;
  }

  if (!is_keep_alive) {
#define CONNECTION_CLOSE "Connection: close\r\n\r\n"
    const int last_len = sizeof(CONNECTION_CLOSE) - 1;
    memcpy(header_data + cur_pos, CONNECTION_CLOSE, last_len);
    cur_pos += last_len;
  } else {
#define CONNECTION_KEEP_ALIVE "Keep-Alive: timeout=15, max=100\r\n\r\n"
    const int last_len = sizeof(CONNECTION_KEEP_ALIVE) - 1;
    memcpy(header_data + cur_pos, CONNECTION_KEEP_ALIVE, last_len);
    cur_pos += last_len;
  }

  DCHECK(strlen(header_data) == static_cast<size_t>(cur_pos));
  size_t nwrite = 0;
  return Write(header_data, cur_pos, &nwrite);
}

}  // namespace http
}  // namespace libev
}  // namespace common
