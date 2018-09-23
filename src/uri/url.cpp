/*  Copyright (C) 2014-2018 FastoGT. All right reserved.

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

#include <common/uri/url.h>

#include <common/sprintf.h>

namespace common {
namespace uri {
namespace {
const char* scheme_name(int level) {
  static const char* kSchemeNames[Url::num_schemes] = {"unknown", "http", "https", "ftp",  "file", "ws",
                                                       "wss",     "udp",  "tcp",   "rtmp", "dev"};
  if (level >= 0 && level < Url::num_schemes) {
    return kSchemeNames[level];
  }

  DNOTREACHED();
  return kSchemeNames[0];
}
}  // namespace
namespace detail {

char from_hex(char ch) {
  return std::isdigit(ch) ? ch - '0' : std::tolower(ch, std::locale()) - 'a' + 10;
}

char to_hex(char code) {
  static char hex[] = "0123456789ABCDEF";
  return hex[code & 15];
}

char* uri_encode(const char* str, size_t len) {
  if (!str || len == 0) {
    return NULL;
  }

  const char* pstr = str;
  char* buf = static_cast<char*>(malloc(len * 3 + 1));
  if (!buf) {
    return NULL;
  }

  char* pbuf = buf;

  while (*pstr) {
    if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~') {
      *pbuf++ = *pstr;
    } else if (*pstr == ' ') {
      *pbuf++ = '+';
    } else {
      *pbuf++ = '%';
      *pbuf++ = to_hex(*pstr >> 4);
      *pbuf++ = to_hex(*pstr & 15);
    }
    pstr++;
  }

  *pbuf = 0;
  return buf;
}

char* uri_decode(const char* str, size_t len) {
  if (!str || len == 0) {
    return NULL;
  }

  const char* pstr = str;
  char* buf = static_cast<char*>(malloc(len + 1));
  char* pbuf = buf;

  while (*pstr) {
    if (*pstr == '%') {
      if (pstr[1] && pstr[2]) {
        *pbuf++ = from_hex(pstr[1]) << 4 | from_hex(pstr[2]);
        pstr += 2;
      }
    } else if (*pstr == '+') {
      *pbuf++ = ' ';
    } else {
      *pbuf++ = *pstr;
    }
    pstr++;
  }

  *pbuf = 0;
  return buf;
}

bool get_schemes(const char* url_s, size_t len, Url::scheme* prot) {
  if (!prot) {
    return false;
  }

  if (url_s && len > 5) {
    if ((url_s[5] == ':' && url_s[6] == uri_separator && url_s[7] == uri_separator) ||
        (url_s[4] == ':' && url_s[5] == uri_separator && url_s[6] == uri_separator) ||
        (url_s[3] == ':' && url_s[4] == uri_separator && url_s[5] == uri_separator) ||
        (url_s[2] == ':' && url_s[3] == uri_separator && url_s[4] == uri_separator)) {
      if (url_s[0] == 'f' || url_s[0] == 'F') {
        if (url_s[1] == 'i' || url_s[1] == 'I') {
          *prot = Url::file;
          return true;
        } else {
          *prot = Url::ftp;
          return true;
        }
      } else if (url_s[0] == 'h' || url_s[0] == 'H') {
        if ((url_s[1] == 't' || url_s[1] == 'T') && (url_s[2] == 't' || url_s[2] == 'T') &&
            (url_s[3] == 'p' || url_s[3] == 'P')) {
          if (url_s[5] == ':') {
            *prot = Url::https;
          } else {
            *prot = Url::http;
          }
          return true;
        }
        return false;
      } else if (url_s[0] == 'w' || url_s[0] == 'W') {
        if (url_s[1] == 's' || url_s[1] == 'S') {
          if (url_s[2] == ':') {
            *prot = Url::ws;
          } else {
            *prot = Url::wss;
          }
          return true;
        }
        return false;
      } else if (url_s[0] == 'u' || url_s[0] == 'U') {
        *prot = Url::udp;
        return true;
      } else if (url_s[0] == 't' || url_s[0] == 'T') {
        *prot = Url::tcp;
        return true;
      } else if (url_s[0] == 'r' || url_s[0] == 'R') {
        *prot = Url::rtmp;
        return true;
      } else if (url_s[0] == 'd' || url_s[0] == 'D') {
        *prot = Url::dev;
        return true;
      }
    }
  }

  return false;
}

}  // namespace detail

Url::Url() : scheme_(unknown), host_() {
  host_.reserve(host_size);
}

Url::Url(const std::string& url_s) : scheme_(unknown), host_() {
  host_.reserve(host_size);
  Parse(url_s);
}

void Url::Parse(const std::string& url_s) {
  // ftp, http, file, ws, udp, tcp, rtmp
  size_t len = url_s.length();
  const char* data = url_s.c_str();
  if (detail::get_schemes(data, len, &scheme_)) {
    size_t start = 0;
    if (scheme_ == ftp) {
      start = 6;
    } else if (scheme_ == http) {
      start = 7;
    } else if (scheme_ == file) {
      start = 6;
    } else if (scheme_ == ws) {
      start = 5;
    } else if (scheme_ == wss) {
      start = 6;
    } else if (scheme_ == udp) {
      start = 6;
    } else if (scheme_ == tcp) {
      start = 6;
    } else if (scheme_ == rtmp) {
      start = 7;
    } else if (scheme_ == https) {
      start = 8;
    } else if (scheme_ == dev) {
      start = 5;
    }

    for (size_t i = start; i < len; ++i) {
      if (url_s[i] == uri_separator) {
        path_ = Upath(data + i + 1);
        break;
      }
      char c = std::tolower(url_s[i], std::locale());
      host_ += c;
    }
  } else {
    scheme_ = unknown;
    host_ = url_s;
  }
}

bool Url::IsValid() const {
  return scheme_ != unknown && (!host_.empty() || path_.IsValid());
}

Url::scheme Url::GetScheme() const {
  return scheme_;
}

std::string Url::GetProtocol() const {
  return scheme_name(scheme_);
}

Upath Url::GetPath() const {
  return path_;
}

void Url::SetPath(const Upath& path) {
  path_ = path;
}

const std::string& Url::GetHost() const {
  return host_;
}

void Url::SetHost(const std::string& host) {
  host_ = host;
}

std::string Url::GetUrl() const {
  DCHECK(IsValid());

  if (host_.empty()) {
    return MemSPrintf("%s://%s", GetProtocol(), path_.GetUpath());
  }

  return MemSPrintf("%s://%s/%s", GetProtocol(), host_, path_.GetUpath());
}

std::string Url::GetHroot() const {
  DCHECK(IsValid());

  return MemSPrintf("%s://%s", GetProtocol(), host_);
}

std::string Url::GetHpath() const {
  DCHECK(IsValid());

  if (host_.empty()) {
    return MemSPrintf("%s://%s", GetProtocol(), path_.GetHpath());
  }

  return MemSPrintf("%s://%s/%s", GetProtocol(), host_, path_.GetHpath());
}

bool Url::Equals(const Url& uri) const {
  if (!IsValid()) {
    return uri.IsValid() ? false : host_ == uri.GetHost();
  }

  return uri.IsValid() == IsValid() && uri.GetUrl() == GetUrl();
}

}  // namespace uri

std::string ConvertToString(const uri::Url& from) {
  return from.IsValid() ? from.GetUrl() : from.GetHost();
}

bool ConvertFromString(const std::string& from, uri::Url* out) {
  if (!out) {
    return false;
  }

  *out = uri::Url(from);
  return true;
}

}  // namespace common
