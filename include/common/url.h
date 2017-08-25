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

#include <common/macros.h>

namespace common {
namespace uri {

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
  std::string GetFilename() const;             // filename
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

class Uri {
 public:
  enum schemes {
    unknown = 0,  // defualt value
    http = 1,
    https = 2,
    ftp = 3,
    file = 4,
    ws = 5,
    udp = 6,
    rtmp = 7,
    dev = 8
  };
  enum {
    host_size = 64,
  };

  Uri();
  explicit Uri(const std::string& url_s);

  bool IsValid() const;

  schemes GetScheme() const;
  std::string GetProtocol() const;
  const std::string& GetHost() const;
  void SetHost(const std::string& host);
  Upath GetPath() const;
  void SetPath(const Upath& path);

  std::string GetUrl() const;    // IsValid == true
  std::string GetHroot() const;  // IsValid == true
  std::string GetHpath() const;  // IsValid == true
  bool Equals(const Uri& uri) const;

 private:
  void Parse(const std::string& url_s);

  schemes schemes_;
  std::string host_;
  Upath path_;
};

inline bool operator==(const Uri& left, const Uri& right) {
  return left.Equals(right);
}

namespace detail {
bool get_schemes(const char* url_s, size_t len, Uri::schemes* prot);

/* Returns a url-encoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
char* uri_encode(const char* str, size_t len);

/* Returns a url-decoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
char* uri_decode(const char* str, size_t len);

const char* get_mime_type(const char* name);
}  // namespace detail

}  // namespace uri

std::string ConvertToString(const uri::Uri& value);  // save
bool ConvertFromString(const std::string& from, uri::Uri* out) WARN_UNUSED_RESULT;

}  // namespace common
