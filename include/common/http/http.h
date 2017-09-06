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

#pragma once

#include <string>   // for string, basic_string
#include <utility>  // for pair
#include <vector>   // for vector

#include <common/error.h>      // for Error
#include <common/types.h>      // for buffer_t
#include <common/uri/upath.h>  // for Upath

#define HTTP_1_0_PROTOCOL_NAME "HTTP/1.0"
#define HTTP_1_0_PROTOCOL_NAME_LEN (sizeof(HTTP_1_0_PROTOCOL_NAME) - 1)

#define HTTP_1_1_PROTOCOL_NAME "HTTP/1.1"
#define HTTP_1_1_PROTOCOL_NAME_LEN (sizeof(HTTP_1_1_PROTOCOL_NAME) - 1)

#define HTTP_2_0_PROTOCOL_NAME "HTTP/2.0"
#define HTTP_2_0_PROTOCOL_NAME_LEN (sizeof(HTTP_2_0_PROTOCOL_NAME) - 1)

namespace common {
namespace http {

enum http_method { HM_GET, HM_HEAD, HM_POST };

enum http_status {
  HS_CONTINUE = 100,
  HS_SWITCH_PROTOCOL = 101,

  HS_OK = 200,
  HS_CREATED = 201,
  HS_ACCEPTED = 202,
  HS_NON_AUTH_INFO = 203,
  HS_NO_CONTENT = 204,
  HS_RESET_CONTENT = 205,
  HS_PARTIAL_CONTENT = 206,

  HS_MULTIPLE_CHOICES = 300,
  HS_MOVED_PERMANENTLY = 301,
  HS_FOUND = 302,
  HS_SEE_OTHER = 303,
  HS_NOT_MODIFIED = 304,
  HS_USE_PROXY = 305,
  HS_TEMPORARY_REDIRECT = 307,
  HS_PERMANENT_REDIRECT = 308,

  HS_BAD_REQUEST = 400,
  HS_UNAUTHORIZED = 401,
  HS_PYMENT_REQUIRED = 402,
  HS_FORBIDDEN = 403,
  HS_NOT_FOUND = 404,
  HS_NOT_ALLOWED = 405,
  HS_NOT_ACCEPTABLE = 406,
  HS_PROXY_AUTH_REQUIRED = 407,
  HS_REQUEST_TIMEOUT = 408,
  HS_CONFLICT = 409,
  HS_GONE = 410,
  HS_LENGTH_REQUIRED = 411,
  HS_PRECONDITION_FAILED = 412,
  HS_PAYLOAD_TOO_LARGE = 413,
  HS_URI_TOO_LONG = 414,
  HS_UNSUPPORTED_MEDIA_TYPE = 415,
  HS_REQUESTED_RANGE_NOT_SATISFIABLE = 416,
  HS_EXPECTATION_FAILED = 417,
  HS_MISDIRECTED_REQUEST = 421,
  HS_UPGRADE_REQUIRED = 426,
  HS_PRECONDITION_REQUIRED = 428,
  HS_TOO_MANY_REQUESTS = 429,
  HS_REQUEST_HEADER_FIELDS_TOO_LARGE = 431,

  HS_INTERNAL_ERROR = 500,
  HS_NOT_IMPLEMENTED = 501,
  HS_BAD_GATEWAY = 502,
  HS_SERVICE_UNAVAILIBLE = 503,
  HS_GATEWAY_TIMEOUT = 504,
  HS_HTTP_VERSION_NOT_SUPPORTED = 505,
  HS_NETWORK_AUTH_REQUIRED = 511
};

struct HttpHeader {
  HttpHeader();
  HttpHeader(const std::string& key, const std::string& value);

  bool IsValid() const;

  std::string key;
  std::string value;
};

enum http_protocols { HP_1_0, HP_1_1, HP_2_0 };

typedef HttpHeader header_t;

struct http_request {
 public:
  typedef std::vector<header_t> headers_t;

  http_request();
  http_request(http_method method,
               const uri::Upath& path,
               const std::string& protocol,
               const headers_t& headers,
               const std::string& body);

  http_protocols protocol() const;
  headers_t headers() const;

  uri::Upath path() const;
  void setPath(const uri::Upath& path);

  http::http_method method() const;
  std::string body() const;

  bool findHeaderByKeyAndChange(const std::string& key, bool caseSensitive, header_t newValue);
  void removeHeaderByKey(const std::string& key, bool caseSensitive);

  header_t findHeaderByKey(const std::string& key, bool caseSensitive) const;
  header_t findHeaderByValue(const std::string& value, bool caseSensitive) const;

 private:
  http_method method_;
  uri::Upath path_;
  std::string protocol_;
  headers_t headers_;
  std::string body_;
};

std::pair<http_status, Error> parse_http_request(const std::string& request, http_request* req_out) WARN_UNUSED_RESULT;
}  // namespace http

std::string ConvertToString(http::http_method method);
bool ConvertFromString(const std::string& from, http::http_method* out) WARN_UNUSED_RESULT;

std::string ConvertToString(http::http_status status);
std::string ConvertToString(http::HttpHeader header);
std::string ConvertToString(http::http_request request);
buffer_t ConvertToBytes(http::http_request request);

std::string ConvertToString(http::http_protocols protocol);
bool ConvertFromString(const std::string& from, http::http_protocols* out) WARN_UNUSED_RESULT;
}  // namespace common
