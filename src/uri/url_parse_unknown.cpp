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

#include <common/uri/url_file.h>

namespace common {
namespace uri {

namespace {

// A subcomponent of DoParseFileURL, the input should be a local file, with the
// beginning of the path indicated by the index in |path_begin|. This will
// initialize the host, path, query, and ref, and leave the other output
// components untouched (DoInitFileURL handles these for us).
template <typename CHAR>
void DoParsePath(const CHAR* spec, int path_begin, int spec_len, Parsed* parsed) {
  ParsePathInternal(spec, MakeRange(path_begin, spec_len), &parsed->path, &parsed->query, &parsed->ref);
  if (parsed->path.is_valid()) {
    parsed->path.len--;
    parsed->path.begin++;
  }
}

template <typename CHAR>
void DoParseUnknownURL(const CHAR* spec, int spec_len, Parsed* parsed) {
  DCHECK(spec_len >= 0);

  // Get the parts we never use for file URLs out of the way.
  parsed->username.reset();
  parsed->password.reset();
  parsed->host.reset();
  parsed->port.reset();

  // Many of the code paths don't set these, so it's convenient to just clear
  // them. We'll write them in those cases we need them.
  parsed->query.reset();
  parsed->ref.reset();

  // Strip leading & trailing spaces and control characters.
  int begin = 0;
  TrimURL(spec, &begin, &spec_len);

  // Find the scheme, if any.
  int num_slashes = CountConsecutiveSlashes(spec, begin, spec_len);
  int after_scheme;
  int after_slashes;
  {
    // ExtractScheme doesn't understand the possibility of filenames with
    // colons in them, in which case it returns the entire spec up to the
    // colon as the scheme. So handle /foo.c:5 as a file but foo.c:5 as
    // the foo.c: scheme.
    if (!num_slashes && ExtractScheme(&spec[begin], spec_len - begin, &parsed->scheme)) {
      // Offset the results since we gave ExtractScheme a substring.
      parsed->scheme.begin += begin;
      after_scheme = parsed->scheme.end() + 1;
    } else {
      // No scheme found, remember that.
      parsed->scheme.reset();
      after_scheme = begin;
    }
  }

  // Handle empty specs ones that contain only whitespace or control chars,
  // or that are just the scheme (for example "unknown:").
  if (after_scheme == spec_len) {
    parsed->host.reset();
    parsed->path.reset();
    return;
  }

  num_slashes = CountConsecutiveSlashes(spec, after_scheme, spec_len);
  after_slashes = after_scheme + num_slashes;
  if (num_slashes != 2) {
    parsed->host.reset();
    parsed->path.reset();
    return;
  }

  DoParsePath(spec, after_scheme + num_slashes - 1, spec_len, parsed);
}

}  // namespace

void ParseUnknownURL(const char* url, int url_len, Parsed* parsed) {
  DoParseUnknownURL(url, url_len, parsed);
}

void ParseUnknownURL(const char16* url, int url_len, Parsed* parsed) {
  DoParseUnknownURL(url, url_len, parsed);
}

void ParseGsURL(const char* url, int url_len, Parsed* parsed) {
  DoParseUnknownURL(url, url_len, parsed);
}

void ParseGsURL(const char16* url, int url_len, Parsed* parsed) {
  DoParseUnknownURL(url, url_len, parsed);
}

}  // namespace uri
}  // namespace common
