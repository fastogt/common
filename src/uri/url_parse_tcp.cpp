/*  Copyright (C) 2014-2021 FastoGT. All right reserved.

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

#include <common/uri/url_parse_internal.h>

namespace common {
namespace uri {

namespace {

// Backend for the external functions that operates on either char type.
// Handles cases where there is a scheme, but also when handed the first
// character following the "file:" at the beginning of the spec. If so,
// this is usually a slash, but needn't be; we allow paths like "file:c:\foo".
template <typename CHAR>
void DoParseTcpURL(const CHAR* spec, int spec_len, Parsed* parsed) {
  parsed->username.reset();
  parsed->password.reset();
  parsed->path.reset();
  parsed->query.reset();
  parsed->ref.reset();

  // Strip leading & trailing spaces and control characters.
  // Strip leading & trailing spaces and control characters.
  int scheme_begin = 0;
  TrimURL(spec, &scheme_begin, &spec_len);

  // Handle empty specs or ones that contain only whitespace or control chars.
  if (scheme_begin == spec_len) {
    return;
  }

  if (ExtractScheme(&spec[scheme_begin], spec_len - scheme_begin, &parsed->scheme)) {
    // Offset the results since we gave ExtractScheme a substring.
    parsed->scheme.begin += scheme_begin;
    int after_scheme = parsed->scheme.end() + 1;
    int num_slashes = CountConsecutiveSlashes(spec, after_scheme, spec_len);
    int after_slashes = after_scheme + num_slashes;

    Component username;
    Component password;
    ParseAuthority(spec, MakeRange(after_slashes, spec_len), &username, &password, &parsed->host, &parsed->port);
  }
}

}  // namespace

void ParseTcpURL(const char* url, int url_len, Parsed* parsed) {
  DoParseTcpURL(url, url_len, parsed);
}

void ParseTcpURL(const char16* url, int url_len, Parsed* parsed) {
  DoParseTcpURL(url, url_len, parsed);
}

}  // namespace uri
}  // namespace common
