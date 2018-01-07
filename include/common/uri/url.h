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

#pragma once

#include <common/macros.h>

#include <common/uri/upath.h>

namespace common {
namespace uri {

class Url {
 public:
  enum scheme {
    unknown = 0,  // defualt value
    http = 1,
    https = 2,
    ftp = 3,
    file = 4,
    ws = 5,
    udp = 6,
    tcp = 7,
    rtmp = 8,
    dev = 9,
    num_schemes
  };
  enum {
    host_size = 64,
  };

  Url();
  explicit Url(const std::string& url_s);

  bool IsValid() const;

  scheme GetScheme() const;
  std::string GetProtocol() const;

  const std::string& GetHost() const;
  void SetHost(const std::string& host);

  Upath GetPath() const;
  void SetPath(const Upath& path);

  std::string GetUrl() const;    // IsValid == true
  std::string GetHroot() const;  // IsValid == true
  std::string GetHpath() const;  // IsValid == true
  bool Equals(const Url& uri) const;

 private:
  void Parse(const std::string& url_s);

  scheme scheme_;
  std::string host_;
  Upath path_;
};

inline bool operator==(const Url& left, const Url& right) {
  return left.Equals(right);
}

inline bool operator!=(const Url& left, const Url& right) {
  return !(left == right);
}

namespace detail {
bool get_schemes(const char* url_s, size_t len, Url::scheme* prot);
/* Returns a url-encoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
char* uri_encode(const char* str, size_t len);

/* Returns a url-decoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
char* uri_decode(const char* str, size_t len);
}  // namespace detail

}  // namespace uri

std::string ConvertToString(const uri::Url& from);
bool ConvertFromString(const std::string& from, uri::Url* out) WARN_UNUSED_RESULT;
}  // namespace common
