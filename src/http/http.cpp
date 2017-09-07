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

#include <common/http/http.h>

#include <common/convert2string.h>  // for ConvertFromString
#include <common/uri/url.h>

namespace common {

std::string ConvertToString(http::http_protocols protocol) {
  if (protocol == http::HP_1_0) {
    return HTTP_1_0_PROTOCOL_NAME;
  } else if (protocol == http::HP_1_1) {
    return HTTP_1_1_PROTOCOL_NAME;
  } else if (protocol == http::HP_2_0) {
    return HTTP_2_0_PROTOCOL_NAME;
  }

  NOTREACHED();
  return "UNKNOWN";
}

bool ConvertFromString(const std::string& from, http::http_protocols* out) {
  if (!out) {
    return false;
  }

  if (from == HTTP_1_0_PROTOCOL_NAME) {
    *out = http::HP_1_0;
    return true;
  } else if (from == HTTP_1_1_PROTOCOL_NAME) {
    *out = http::HP_1_1;
    return true;
  } else if (from == HTTP_2_0_PROTOCOL_NAME) {
    *out = http::HP_2_0;
    return true;
  }

  NOTREACHED();
  return false;
}

namespace http {

HttpHeader::HttpHeader() : key(), value() {}

HttpHeader::HttpHeader(const std::string& key, const std::string& value) : key(key), value(value) {}

bool HttpHeader::IsValid() const {
  return !key.empty();
}

http_request::http_request() : method_(), path_(), protocol_(), headers_(), body_() {}

http_request::http_request(http_method method,
                           const uri::Upath& path,
                           const std::string& protocol,
                           const headers_t& headers,
                           const std::string& body)
    : method_(method), path_(path), protocol_(protocol), headers_(headers), body_(body) {}

http::http_protocols http_request::protocol() const {
  http::http_protocols p;
  if (!ConvertFromString(protocol_, &p)) {
    return HP_1_0;
  }

  return p;
}

http_request::headers_t http_request::headers() const {
  return headers_;
}

uri::Upath http_request::path() const {
  return path_;
}

void http_request::setPath(const uri::Upath& path) {
  path_ = path;
}

http::http_method http_request::method() const {
  return method_;
}

std::string http_request::body() const {
  return body_;
}

bool http_request::findHeaderByKeyAndChange(const std::string& key, bool caseSensitive, header_t newValue) {
  for (size_t i = 0; i < headers_.size(); ++i) {
    std::string curKey = headers_[i].key;
    if (EqualsASCII(curKey, key, caseSensitive)) {
      headers_[i] = newValue;
      return true;
    }
  }

  return false;
}

void http_request::removeHeaderByKey(const std::string& key, bool caseSensitive) {
  for (size_t i = 0; i < headers_.size(); ++i) {
    std::string curKey = headers_[i].key;
    if (EqualsASCII(curKey, key, caseSensitive)) {
      headers_.erase(headers_.begin(), headers_.begin() + i);
      return;
    }
  }
}

header_t http_request::findHeaderByKey(const std::string& key, bool caseSensitive) const {
  for (size_t i = 0; i < headers_.size(); ++i) {
    std::string curKey = headers_[i].key;
    if (EqualsASCII(curKey, key, caseSensitive)) {
      return headers_[i];
    }
  }

  return header_t();
}

header_t http_request::findHeaderByValue(const std::string& value, bool caseSensitive) const {
  for (size_t i = 0; i < headers_.size(); ++i) {
    std::string curKey = headers_[i].value;
    if (EqualsASCII(curKey, value, caseSensitive)) {
      return headers_[i];
    }
  }

  return header_t();
}

std::pair<http_status, Error> parse_http_request(const std::string& request, http_request* req_out) {
  if (request.empty() || !req_out) {
    return std::make_pair(HS_FORBIDDEN, make_inval_error_value(ERROR_TYPE));
  }

  typedef std::string::size_type string_size_t;

  const string_size_t len = request.size();

  http_method lmethod = HM_GET;
  uri::Upath lpath;
  std::string lprotocol;
  std::string lbody;
  http_request::headers_t lheaders;

  string_size_t pos = 0;
  string_size_t start = 0;
  uint8_t line_count = 0;
  while ((pos = request.find("\r\n", start)) != std::string::npos) {
    std::string line = request.substr(start, pos - start);
    if (line_count == 0) {  // GET //POST //HEAD
      string_size_t space = line.find_first_of(' ');
      if (space != std::string::npos) {
        std::string method = line.substr(0, space);
        if (method == "GET" || method == "HEAD" || method == "POST") {
          if (method == "GET") {
            lmethod = HM_GET;
          } else if (method == "HEAD") {
            lmethod = HM_HEAD;
          } else {
            lmethod = HM_POST;
          }

          std::string protcolAndPath = line.substr(space + 1);
          size_t spaceP = protcolAndPath.find_first_of("HTTP");
          if (spaceP != std::string::npos) {
            std::string path = protcolAndPath.substr(1, spaceP - 2);
            if (protcolAndPath[0] != '/') {  // must start with /
              return std::make_pair(HS_BAD_REQUEST, make_error_value("Bad filename.", ERROR_TYPE));
            }

            lprotocol = protcolAndPath.substr(spaceP);
            lpath = uri::Upath(path);
          } else {
            return std::make_pair(HS_FORBIDDEN, make_error_value("Not allowed.", ERROR_TYPE));
          }
        } else {
          return std::make_pair(HS_NOT_IMPLEMENTED,
                                make_error_value("That method is not implemented.", ERROR_TYPE));
        }
      } else {
        return std::make_pair(HS_NOT_IMPLEMENTED,
                              make_error_value("That method is not implemented.", ERROR_TYPE));
      }
    } else {
      string_size_t delem = line.find_first_of(':');
      if (delem != std::string::npos) {
        std::string key = line.substr(0, delem);
        std::string value = line.substr(delem + 1);
        std::string trimkey;
        TrimWhitespaceASCII(key, TRIM_ALL, &trimkey);

        std::string trimvalue;
        TrimWhitespaceASCII(value, TRIM_ALL, &trimvalue);
        HttpHeader header(trimkey, trimvalue);
        lheaders.push_back(header);
      }
    }
    line_count++;
    start = pos + 2;
  }

  if (len != start && line_count != 0) {
    const char* request_str = request.c_str() + start;
    lbody = uri::detail::uri_decode(request_str, strlen(request_str));
  }

  if (line_count == 0) {
    return std::make_pair(HS_BAD_REQUEST, make_error_value("Not found CRLF", ERROR_TYPE));
  }

  *req_out = http_request(lmethod, lpath, lprotocol, lheaders, lbody);
  return std::make_pair(HS_OK, Error());
}

}  // namespace http

std::string ConvertToString(http::http_method method) {
  if (method == http::HM_GET) {
    return "GET";
  } else if (method == http::HM_HEAD) {
    return "HEAD";
  } else {
    return "POST";
  }
}

bool ConvertFromString(const std::string& from, http::http_method* out) {
  if (!out) {
    return false;
  }

  if (from == "GET") {
    *out = http::HM_GET;
    return true;
  } else if (from == "HEAD") {
    *out = http::HM_HEAD;
    return true;
  } else if (from == "POST") {
    *out = http::HM_POST;
    return true;
  }

  return false;
}

std::string ConvertToString(http::http_status status) {
  switch (status) {
    case http::HS_CONTINUE:
      return "Continue";
    case http::HS_SWITCH_PROTOCOL:
      return "Switching Protocols";
    case http::HS_OK:
      return "OK";
    case http::HS_CREATED:
      return "Created";
    case http::HS_ACCEPTED:
      return "Accepted";
    case http::HS_NON_AUTH_INFO:
      return "Non-Authoritative Information";
    case http::HS_NO_CONTENT:
      return "No Content";
    case http::HS_RESET_CONTENT:
      return "Reset Content";
    case http::HS_PARTIAL_CONTENT:
      return "Partial Content";
    case http::HS_MULTIPLE_CHOICES:
      return "Multiple Choices";
    case http::HS_MOVED_PERMANENTLY:
      return "Moved Permanently";
    case http::HS_FOUND:
      return "Found";
    case http::HS_SEE_OTHER:
      return "See Other";
    case http::HS_NOT_MODIFIED:
      return "Not Modified";
    case http::HS_USE_PROXY:
      return "Use Proxy";
    case http::HS_TEMPORARY_REDIRECT:
      return "Temporary Redirect";
    case http::HS_PERMANENT_REDIRECT:
      return "Permanent Redirect";
    case http::HS_BAD_REQUEST:
      return "Bad Request";
    case http::HS_UNAUTHORIZED:
      return "Unauthorized";
    case http::HS_PYMENT_REQUIRED:
      return "Payment Required";
    case http::HS_FORBIDDEN:
      return "Forbidden";
    case http::HS_NOT_FOUND:
      return "Not Found";
    case http::HS_NOT_ALLOWED:
      return "Method Not Allowed";
    case http::HS_NOT_ACCEPTABLE:
      return "Not Acceptable";
    case http::HS_PROXY_AUTH_REQUIRED:
      return "Proxy Authentication Required";
    case http::HS_REQUEST_TIMEOUT:
      return "Request Timeout";
    case http::HS_CONFLICT:
      return "Conflict";
    case http::HS_GONE:
      return "Gone";
    case http::HS_LENGTH_REQUIRED:
      return "Length Required";
    case http::HS_PRECONDITION_FAILED:
      return "Precondition Failed";
    case http::HS_PAYLOAD_TOO_LARGE:
      return "Payload Too Large";
    case http::HS_URI_TOO_LONG:
      return "URI Too Long";
    case http::HS_UNSUPPORTED_MEDIA_TYPE:
      return "Unsupported Media Type";
    case http::HS_REQUESTED_RANGE_NOT_SATISFIABLE:
      return "Requested Range Not Satisfiable";
    case http::HS_EXPECTATION_FAILED:
      return "Expectation Failed";
    case http::HS_MISDIRECTED_REQUEST:
      return "Misdirected Request";
    case http::HS_UPGRADE_REQUIRED:
      return "Upgrade Required";
    case http::HS_PRECONDITION_REQUIRED:
      return "Precondition Required";
    case http::HS_TOO_MANY_REQUESTS:
      return "Too Many Requests";
    case http::HS_REQUEST_HEADER_FIELDS_TOO_LARGE:
      return "Request Header Fields Too Large";
    case http::HS_INTERNAL_ERROR:
      return "Internal Server Error";
    case http::HS_NOT_IMPLEMENTED:
      return "Not Implemented";
    case http::HS_BAD_GATEWAY:
      return "Bad Gateway";
    case http::HS_SERVICE_UNAVAILIBLE:
      return "Service Unavailable";
    case http::HS_GATEWAY_TIMEOUT:
      return "Gateway Timeout";
    case http::HS_HTTP_VERSION_NOT_SUPPORTED:
      return "HTTP Version Not Supported";
    case http::HS_NETWORK_AUTH_REQUIRED:
      return "Network Authentication Required";
    default:
      NOTREACHED();
      return "Unknown";
  }
}

std::string ConvertToString(http::HttpHeader header) {
  if (!header.IsValid()) {
    return std::string();
  }

  return header.key + ":" + header.value;
}

std::string ConvertToString(http::http_request request) {
  uri::Upath upath = request.path();
  http::http_method method = request.method();

  std::string headerout = MemSPrintf("%s /%s %s\r\n", ConvertToString(method), upath.GetPath(),
                                     ConvertToString(request.protocol()));  // "GET /hello.htm HTTP/1.1\r\n"
  http::http_request::headers_t headers = request.headers();
  for (size_t i = 0; i < headers.size(); ++i) {
    http::header_t header = headers[i];
    std::string headerStr = ConvertToString(header);
    if (!headerStr.empty()) {
      headerout += headerStr + "\r\n";
    }
  }
  headerout += "\r\n";

  return headerout;
}

buffer_t ConvertToBytes(http::http_request request) {
  std::string request_str = ConvertToString(request);
  buffer_t res;
  if (!ConvertFromString(request_str, &res)) {
    return buffer_t();
  }
  return res;
}

}  // namespace common
