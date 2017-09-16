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

#include <string>

namespace common {
namespace uri {

extern const char uri_separator;
extern const char uri_separator_string[];

class Upath {
 public:
  Upath();
  explicit Upath(const std::string& url_sp);

  bool IsValid() const;
  bool IsRoot() const;

  std::string GetPath() const;  // hpath + filename = path
  void SetPath(const std::string& path);

  std::string GetQuery() const;

  std::string GetMime() const;  // mime
  size_t GetLevels() const;
  std::string GetHpathLevel(size_t lv) const;  // directories
  std::string GetHpath() const;                // directories
  std::string GetFileName() const;             // filename
  std::string GetUpath() const;                // path + query
  bool Equals(const Upath& path) const;

 private:
  void Parse(const std::string& url_s);
  void Parse(const char* url_s, size_t len);

  std::string path_;
  std::string query_;
};

inline bool operator==(const Upath& left, const Upath& right) {
  return left.Equals(right);
}

inline bool operator!=(const Upath& left, const Upath& right) {
  return !(left == right);
}

namespace detail {
const char* get_mime_type(const char* name);
}

}  // namespace uri
}  // namespace common
