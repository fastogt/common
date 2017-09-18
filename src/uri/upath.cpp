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

#include <common/uri/upath.h>

#include <common/file_system/types.h>
#include <common/string_util.h>

namespace common {
namespace uri {

const char uri_separator = '/';
const char uri_separator_string[] = "/";

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

std::string Upath::GetPath() const {
  return path_;
}

void Upath::SetPath(const std::string& path) {
  path_ = path;
}

std::string Upath::GetQuery() const {
  return query_;
}

std::string Upath::GetHpath() const {
  size_t slash = path_.find_last_of(uri_separator_string);
  if (slash != std::string::npos) {
    return path_.substr(0, slash + 1);
  }

  return file_system::stable_dir_path_separator(path_);
}

std::string Upath::GetFileName() const {
  size_t slash = path_.find_last_of(uri_separator_string);
  if (slash != std::string::npos) {
    return path_.substr(slash + 1);
  }

  return std::string();
}

std::string Upath::GetMime() const {
  return detail::get_mime_type(path_.c_str());
}

std::string Upath::GetHpathLevel(size_t lv) const {
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

size_t Upath::GetLevels() const {
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

namespace detail {
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

}  // namespace uri
}  // namespace common
