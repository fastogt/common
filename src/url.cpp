/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

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

#include <common/url.h>

#include <ctype.h>   // for isalnum
#include <stdlib.h>  // for NULL
#include <string.h>  // for strcmp, strrchr
#include <string>    // for string

#include <common/file_system.h>
#include <common/logger.h>  // for COMPACT_LOG_FILE_CRIT
#include <common/macros.h>  // for DCHECK

namespace {
const char* schemes_array[] = {"unknown", "http", "https", "ftp", "file", "ws", "udp", "rtmp", "dev"};
const char uri_separator = '/';
const std::string uri_separator_string = "/";
}  // namespace

namespace common {
namespace {
template <typename CharT>
inline std::basic_string<CharT> stable_path(std::basic_string<CharT> path) {
  size_t lenght = path.length();
  if (lenght > 1 && path[lenght - 1] != file_system::get_separator<CharT>()) {
    path += file_system::get_separator<CharT>();
  }
  return path;
}
}  // namespace
namespace uri {
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
  char* buf = reinterpret_cast<char*>(malloc(len * 3 + 1));
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
      *pbuf++ = '%', *pbuf++ = to_hex(*pstr >> 4), *pbuf++ = to_hex(*pstr & 15);
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
  char* buf = reinterpret_cast<char*>(malloc(len + 1));
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

bool get_schemes(const char* url_s, size_t len, Uri::schemes* prot) {
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
          *prot = Uri::file;
          return true;
        } else {
          *prot = Uri::ftp;
          return true;
        }
      } else if (url_s[0] == 'h' || url_s[0] == 'H') {
        if (url_s[5] == ':') {
          *prot = Uri::https;
        } else {
          *prot = Uri::http;
        }
        return true;
      } else if (url_s[0] == 'w' || url_s[0] == 'W') {
        *prot = Uri::ws;
        return true;
      } else if (url_s[0] == 'u' || url_s[0] == 'U') {
        *prot = Uri::udp;
        return true;
      } else if (url_s[0] == 'r' || url_s[0] == 'R') {
        *prot = Uri::rtmp;
        return true;
      } else if (url_s[0] == 'd' || url_s[0] == 'D') {
        *prot = Uri::dev;
        return true;
      }
    }
  }

  return false;
}

const char* get_mime_type(const char* name) {
  const char* dot = strrchr(name, '.');
  if (dot == NULL) {
    return "text/plain; charset=UTF-8";
  }
  if (strcmp(dot, ".html") == 0 || strcmp(dot, ".htm") == 0) {
    return "text/html; charset=UTF-8";
  }
  if (strcmp(dot, ".xhtml") == 0 || strcmp(dot, ".xht") == 0) {
    return "application/xhtml+xml; charset=UTF-8";
  }
  if (strcmp(dot, ".jpg") == 0 || strcmp(dot, ".jpeg") == 0) {
    return "image/jpeg";
  }
  if (strcmp(dot, ".gif") == 0) {
    return "image/gif";
  }
  if (strcmp(dot, ".png") == 0) {
    return "image/png";
  }
  if (strcmp(dot, ".css") == 0) {
    return "text/css";
  }
  if (strcmp(dot, ".xml") == 0 || strcmp(dot, ".xsl") == 0) {
    return "text/xml; charset=UTF-8";
  }
  if (strcmp(dot, ".au") == 0) {
    return "audio/basic";
  }
  if (strcmp(dot, ".wav") == 0) {
    return "audio/wav";
  }
  if (strcmp(dot, ".avi") == 0) {
    return "video/x-msvideo";
  }
  if (strcmp(dot, ".mov") == 0 || strcmp(dot, ".qt") == 0) {
    return "video/quicktime";
  }
  if (strcmp(dot, ".mpeg") == 0 || strcmp(dot, ".mpe") == 0) {
    return "video/mpeg";
  }
  if (strcmp(dot, ".vrml") == 0 || strcmp(dot, ".wrl") == 0) {
    return "model/vrml";
  }
  if (strcmp(dot, ".midi") == 0 || strcmp(dot, ".mid") == 0) {
    return "audio/midi";
  }
  if (strcmp(dot, ".mp3") == 0) {
    return "audio/mpeg";
  }
  if (strcmp(dot, ".ogg") == 0) {
    return "application/ogg";
  }
  if (strcmp(dot, ".pac") == 0) {
    return "application/x-ns-proxy-autoconfig";
  }

  return "text/plain; charset=UTF-8";
}

}  // namespace detail

Upath::Upath() : path_(), query_() {}

Upath::Upath(const std::string& url_sp) : path_(), query_() {
  Parse(url_sp);
}

bool Upath::IsValid() const {
  return !path_.empty();
}

bool Upath::IsRoot() const {
  return path_ == uri_separator_string;
}

std::string Upath::Path() const {
  return path_;
}

void Upath::SetPath(const std::string& path) {
  path_ = path;
}

std::string Upath::Query() const {
  return query_;
}

std::string Upath::Hpath() const {
  size_t slash = path_.find_last_of(uri_separator_string);
  if (slash != std::string::npos) {
    return path_.substr(0, slash + 1);
  }

  return stable_path(path_);
}

std::string Upath::Filename() const {
  size_t slash = path_.find_last_of(uri_separator_string);
  if (slash != std::string::npos) {
    return path_.substr(slash + 1);
  }

  return std::string();
}

std::string Upath::Mime() const {
  return detail::get_mime_type(path_.c_str());
}

std::string Upath::HpathLevel(size_t lv) const {
  std::vector<std::string> tokens;
  size_t count = Tokenize(path_, uri_separator_string, &tokens);
  if (lv > count) {
    return std::string();
  }

  std::string result;
  for (size_t i = 0; i < lv; i++) {
    result += tokens[i];
    result += uri_separator_string;
  }

  return result;
}

size_t Upath::Levels() const {
  std::vector<std::string> tokens;
  size_t sz = Tokenize(path_, uri_separator_string, &tokens);
  if (sz == 0) {
    return 0;
  }

  return sz - 1;
}

std::string Upath::GetUpath() const {
  if (query_.empty()) {
    return path_;
  } else {
    return path_ + "?" + query_;
  }
}

bool Upath::Equals(const Upath& path) const {
  if (!IsValid()) {
    return path.IsValid() ? false : GetUpath() == path.GetUpath();
  }

  return path.IsValid() == IsValid() && GetUpath() == path.GetUpath();
}

void Upath::Parse(const std::string& url_s) {
  Parse(url_s.c_str(), url_s.length());
}

void Upath::Parse(const char* url_s, size_t len) {
  if (!url_s || len == 0) {
    return;
  }

  if (len == 2 && strcmp(url_s, "..") == 0) {
    return;
  }

  if (len >= 3 && strncmp(url_s, "../", 3) == 0) {
    return;
  }

  if (len >= 3 && strcmp((url_s + len - 3), "/..") == 0) {
    return;
  }

  if (len >= 4 && strstr(url_s, "/../") != NULL) {
    return;
  }

  std::string* cur_member = &path_;
  for (size_t i = 0; i < len; ++i) {
    char ch = url_s[i];
    if (ch == '?') {
      if (cur_member == &path_) {
        cur_member = &query_;
        continue;
      }
    }
    cur_member->operator+=(ch);
  }
}

Uri::Uri() : schemes_(unknown), host_() {
  host_.reserve(host_size);
}

Uri::Uri(const std::string& url_s) : schemes_(unknown), host_() {
  host_.reserve(host_size);
  Parse(url_s);
}

void Uri::Parse(const std::string& url_s) {
  // ftp, http, file, ws, udp, rtmp
  size_t len = url_s.length();
  const char* data = url_s.c_str();
  size_t start = 0;
  if (detail::get_schemes(data, len, &schemes_)) {
    if (schemes_ == ftp) {
      start = 6;
    } else if (schemes_ == http) {
      start = 7;
    } else if (schemes_ == file) {
      start = 6;
    } else if (schemes_ == ws) {
      start = 5;
    } else if (schemes_ == udp) {
      start = 6;
    } else if (schemes_ == rtmp) {
      start = 7;
    } else if (schemes_ == https) {
      start = 8;
    } else if (schemes_ == dev) {
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
    schemes_ = unknown;
    host_ = url_s;
  }
}

bool Uri::IsValid() const {
  return schemes_ != unknown && (!host_.empty() || path_.IsValid());
}

Uri::schemes Uri::Scheme() const {
  return schemes_;
}

std::string Uri::Protocol() const {
  return schemes_array[schemes_];
}

Upath Uri::Path() const {
  return path_;
}

void Uri::SetPath(const Upath& path) {
  path_ = path;
}

const std::string& Uri::Host() const {
  return host_;
}

void Uri::SetHost(const std::string& host) {
  host_ = host;
}

std::string Uri::Url() const {
  DCHECK(IsValid());

  if (host_.empty()) {
    return MemSPrintf("%s://%s", Protocol(), path_.GetUpath());
  }

  return MemSPrintf("%s://%s/%s", Protocol(), host_, path_.GetUpath());
}

std::string Uri::Hroot() const {
  DCHECK(IsValid());

  return MemSPrintf("%s://%s", Protocol(), host_);
}

std::string Uri::Hpath() const {
  DCHECK(IsValid());

  if (host_.empty()) {
    return MemSPrintf("%s://%s", Protocol(), path_.Hpath());
  }

  return MemSPrintf("%s://%s/%s", Protocol(), host_, path_.Hpath());
}

bool Uri::Equals(const Uri& uri) const {
  if (!IsValid()) {
    return uri.IsValid() ? false : host_ == uri.Host();
  }

  return uri.IsValid() == IsValid() && uri.Url() == Url();
}

}  // namespace uri

std::string ConvertToString(const uri::Uri& value) {
  return value.IsValid() ? value.Url() : value.Host();
}

bool ConvertFromString(const std::string& from, uri::Uri* out) {
  if (!out) {
    return false;
  }

  *out = uri::Uri(from);
  return true;
}

}  // namespace common
