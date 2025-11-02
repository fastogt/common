/*  Copyright (C) 2014-2022 FastoGT. All right reserved.

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

#include <common/convert2string.h>
#include <common/file_system/file_system.h>
#include <common/libev/http/http_client.h>
#include <common/logger.h>
#include <common/net/net.h>
#include <common/sprintf.h>
#include <inttypes.h>

#include <string>

#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"
#define CARET_MARKER "\r\n"

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

HttpClient::HttpClient(IoLoop* server, const net::socket_info& info) : TcpClient(server, info) {}

HttpClient::HttpClient(libev::IoLoop* server, net::TcpSocketHolder* sock) : TcpClient(server, sock) {}

ErrnoError HttpClient::Get(const uri::GURL& url, bool is_keep_alive) {
  return SendRequest(common::http::HM_GET, url, common::http::HP_1_1, {}, is_keep_alive);
}

ErrnoError HttpClient::Post(const uri::GURL& url,
                            const char* mime_type,
                            const char* text,
                            size_t len,
                            bool is_keep_alive) {
  const common::http::headers_t extra_headers = {
      common::http::HttpHeader("Content-Type", mime_type),
      common::http::HttpHeader("Content-Length", common::ConvertToString(len))};

  ErrnoError err = SendRequest(common::http::HM_POST, url, common::http::HP_1_1, extra_headers, is_keep_alive);
  if (err) {
    return err;
  }

  size_t nwrite = 0;
  return Write(text, len, &nwrite);
}

ErrnoError HttpClient::PostJson(const uri::GURL& url, const char* text, size_t len, bool is_keep_alive) {
  return Post(url, "application/json", text, len, is_keep_alive);
}

ErrnoError HttpClient::Head(const uri::GURL& url, bool is_keep_alive) {
  return SendRequest(common::http::HM_HEAD, url, common::http::HP_1_1, {}, is_keep_alive);
}

ErrnoError HttpClient::SendRequest(common::http::http_method method,
                                   const uri::GURL& url,
                                   common::http::http_protocol protocol,
                                   const common::http::headers_t& extra_headers,
                                   bool is_keep_alive) {
  common::http::headers_t copy = extra_headers;
  if (!is_keep_alive) {
    copy.push_back(common::http::HttpHeader("Connection", "close"));
  } else {
    copy.push_back(common::http::HttpHeader("Connection", "keep-alive"));
  }

  return SendRequest(method, url, protocol, copy);
}

ErrnoError HttpClient::SendRequest(common::http::http_method method,
                                   const uri::GURL& url,
                                   common::http::http_protocol protocol,
                                   const common::http::headers_t& extra_headers) {
  DCHECK(protocol <= common::http::HP_1_1);

  if (!url.is_valid()) {
    return make_errno_error_inval();
  }

  bool isHttp = url.SchemeIsHTTPOrHTTPS();
  bool isWS = url.SchemeIsWSOrWSS();
  if (!(isHttp || isWS)) {
    return make_errno_error_inval();
  }

  const std::string method_str = ConvertToString(method);
  std::string host = url.HostNoBrackets();
  std::string path = url.PathForRequest();
  char header_data[2048] = {0};
  int cur_pos = SNPrintf(header_data, sizeof(header_data),
                         protocol == common::http::HP_2_0 ? "%s %s " HTTP_2_0_PROTOCOL_NAME
                                                            "\r\n"
                                                            "Host: %s\r\n"
                                                          : "%s %s " HTTP_1_0_PROTOCOL_NAME
                                                            "\r\n"
                                                            "Host: %s\r\n",
                         method_str, path, host);

  for (size_t i = 0; i < extra_headers.size(); ++i) {
    const auto header = extra_headers[i];
    const std::string header_str = header.as_string();
    int exlen = SNPrintf(header_data + cur_pos, sizeof(header_data) - cur_pos, "%s\r\n", header_str);
    cur_pos += exlen;
  }

  const int last_len = sizeof(CARET_MARKER) - 1;
  memcpy(header_data + cur_pos, CARET_MARKER, last_len);
  cur_pos += last_len;

  DCHECK(strlen(header_data) == static_cast<size_t>(cur_pos));
  size_t nwrite = 0;
  return Write(header_data, cur_pos, &nwrite);
}

HttpServerClient::HttpServerClient(IoLoop* server, const net::socket_info& info)
    : HttpClient(server, info), isAuth_(false) {}

HttpServerClient::HttpServerClient(libev::IoLoop* server, net::TcpSocketHolder* sock)
    : HttpClient(server, sock), isAuth_(false) {}

const char* HttpServerClient::ClassName() const {
  return "HttpServerClient";
}

void HttpServerClient::SetIsAuthenticated(bool auth) {
  isAuth_ = auth;
}

bool HttpServerClient::IsAuthenticated() const {
  return isAuth_;
}

ErrnoError HttpServerClient::SendJson(common::http::http_protocol protocol,
                                      common::http::http_status status,
                                      const common::http::headers_t& extra_headers,
                                      const char* text,
                                      size_t len,
                                      bool is_keep_alive,
                                      const HttpServerInfo& info) {
  DCHECK(protocol <= common::http::HP_1_1);

  ErrnoError err = SendHeaders(protocol, status, extra_headers, "application/json", &len, nullptr, is_keep_alive, info);
  if (err) {
    return err;
  }

  size_t nwrite = 0;
  return Write(text, len, &nwrite);
}

ErrnoError HttpServerClient::SendOk(common::http::http_protocol protocol,
                                    const common::http::headers_t& extra_headers,
                                    const char* text,
                                    bool is_keep_alive,
                                    const HttpServerInfo& info) {
  return SendError(protocol, common::http::HS_OK, extra_headers, text, is_keep_alive, info);
}

ErrnoError HttpServerClient::SendError(common::http::http_protocol protocol,
                                       common::http::http_status status,
                                       const common::http::headers_t& extra_headers,
                                       const char* text,
                                       bool is_keep_alive,
                                       const HttpServerInfo& info) {
  DCHECK(protocol <= common::http::HP_1_1);
  const std::string title = ConvertToString(status);

  char err_data[1024] = {0};
  ssize_t err_len = SNPrintf(err_data, sizeof(err_data), HTML_PATTERN_ISISSSS7, status, title, status, title, text,
                             info.server_url, info.server_name);
  size_t cast = err_len;
  ErrnoError err = SendHeaders(protocol, status, extra_headers, "text/html", &cast, nullptr, is_keep_alive, info);
  if (err) {
    return err;
  }

  size_t nwrite = 0;
  return Write(err_data, err_len, &nwrite);
}

ErrnoError HttpServerClient::SendFileByFd(descriptor_t fdesc, size_t size) {
  return SendFile(fdesc, 0, size);
}

ErrnoError HttpServerClient::SendHeaders(common::http::http_protocol protocol,
                                         common::http::http_status status,
                                         const common::http::headers_t& extra_headers,
                                         const char* mime_type,
                                         size_t* length,
                                         time_t* mod,
                                         bool is_keep_alive,
                                         const HttpServerInfo& info) {
  DCHECK(protocol <= common::http::HP_1_1);
  const std::string title = ConvertToString(status);

  time_t now = ::time(nullptr);
  char timebuf[100];
  strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&now));

  char header_data[2048] = {0};
  int cur_pos = SNPrintf(header_data, sizeof(header_data),
                         protocol == common::http::HP_2_0 ? HTTP_2_0_PROTOCOL_NAME
                             " %d %s\r\n"
                             "Server: %s\r\n"
                             "Date: %s\r\n"
                                                          : HTTP_1_0_PROTOCOL_NAME
                             " %d %s\r\n"
                             "Server: %s\r\n"
                             "Date: %s\r\n",
                         status, title, info.server_name, timebuf);

  for (size_t i = 0; i < extra_headers.size(); ++i) {
    const auto header = extra_headers[i];
    const std::string header_str = header.as_string();
    int exlen = SNPrintf(header_data + cur_pos, sizeof(header_data) - cur_pos, "%s\r\n", header_str);
    cur_pos += exlen;
  }
  if (mime_type) {
    int mim_t = SNPrintf(header_data + cur_pos, sizeof(header_data) - cur_pos, "Content-Type: %s\r\n", mime_type);
    cur_pos += mim_t;
  }
  if (length) {
    int len = SNPrintf(header_data + cur_pos, sizeof(header_data) - cur_pos, "Content-Length: %zu\r\n", *length);
    cur_pos += len;
  }
  if (mod) {
    strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(mod));
    int mlen = SNPrintf(header_data + cur_pos, sizeof(header_data) - cur_pos, "Last-Modified: %s\r\n", timebuf);
    cur_pos += mlen;
  }

#if 0
#define ACCEPT_RANGES "Accept-Ranges: none\r\n"
  const int last_len = sizeof(ACCEPT_RANGES) - 1;
  memcpy(header_data + cur_pos, ACCEPT_RANGES, last_len);
  cur_pos += last_len;
#endif

  if (!is_keep_alive) {
#define CONNECTION_CLOSE "Connection: close\r\n" CARET_MARKER
    const int last_len = sizeof(CONNECTION_CLOSE) - 1;
    memcpy(header_data + cur_pos, CONNECTION_CLOSE, last_len);
    cur_pos += last_len;
  } else {
#define CONNECTION_KEEP_ALIVE "Connection: keep-alive\r\n" CARET_MARKER
    const int last_len = sizeof(CONNECTION_KEEP_ALIVE) - 1;
    memcpy(header_data + cur_pos, CONNECTION_KEEP_ALIVE, last_len);
    cur_pos += last_len;
  }

  DCHECK(strlen(header_data) == static_cast<size_t>(cur_pos));
  size_t nwrite = 0;
  return Write(header_data, cur_pos, &nwrite);
}

ErrnoError HttpServerClient::SendResponse(common::http::http_protocol protocol,
                                          common::http::http_status status,
                                          const common::http::headers_t& extra_headers,
                                          const HttpServerInfo& info) {
  DCHECK(protocol <= common::http::HP_1_1);
  const std::string title = ConvertToString(status);

  time_t now = ::time(nullptr);
  char timebuf[100];
  strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&now));

  char header_data[2048] = {0};
  int cur_pos = SNPrintf(header_data, sizeof(header_data),
                         protocol == common::http::HP_2_0 ? HTTP_2_0_PROTOCOL_NAME
                             " %d %s\r\n"
                             "Server: %s\r\n"
                             "Date: %s\r\n"
                         : protocol == common::http::HP_1_0 ? HTTP_1_0_PROTOCOL_NAME
                                                            : HTTP_1_1_PROTOCOL_NAME
                             " %d %s\r\n"
                             "Server: %s\r\n"
                             "Date: %s\r\n",
                         status, title, info.server_name, timebuf);

  for (size_t i = 0; i < extra_headers.size(); ++i) {
    const auto header = extra_headers[i];
    const std::string header_str = header.as_string();
    int exlen = SNPrintf(header_data + cur_pos, sizeof(header_data) - cur_pos, "%s\r\n", header_str);
    cur_pos += exlen;
  }

  const int last_len = sizeof(CARET_MARKER) - 1;
  memcpy(header_data + cur_pos, CARET_MARKER, last_len);
  cur_pos += last_len;

  DCHECK(strlen(header_data) == static_cast<size_t>(cur_pos));
  size_t nwrite = 0;
  return Write(header_data, cur_pos, &nwrite);
}

}  // namespace http
}  // namespace libev
}  // namespace common
