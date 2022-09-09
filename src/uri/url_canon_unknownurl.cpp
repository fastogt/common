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

#include <common/uri/url_canon.h>

namespace common {
namespace uri {

namespace {

template <typename CHAR, typename UCHAR>
bool DounknownCanonicalizePath(const CHAR* spec, const Component& path, CanonOutput* output, Component* out_path) {
  out_path->begin = output->length();
  int after_drive = path.begin;

  if (after_drive < path.end()) {
    Component sub_path = MakeRange(after_drive, path.end());
    out_path->begin = after_drive;
    out_path->len = sub_path.len;
    for (int i = sub_path.begin; i < sub_path.begin + sub_path.len; i++) {
      output->push_back(spec[i]);
    }
    return true;
  }
  return false;
}

template <typename CHAR, typename UCHAR>
bool DoCanonicalizeUnknownURL(const URLComponentSource<CHAR>& source,
                              const Parsed& parsed,
                              CharsetConverter* query_converter,
                              CanonOutput* output,
                              Parsed* new_parsed) {
  // Things we don't set in dev: URLs.
  new_parsed->host = Component();
  new_parsed->port = Component();
  new_parsed->username = Component();
  new_parsed->password = Component();
  new_parsed->port = Component();

  // Scheme (known, so we don't bother running it through the more
  // complicated scheme canonicalizer).
  new_parsed->scheme.begin = output->length();
  output->Append("unknown://", 10);
  new_parsed->scheme.len = 7;

  // Append the host. For many dev URLs, this will be empty. For UNC, this
  // will be present.
  // TODO(brettw) This doesn't do any checking for host name validity. We
  // should probably handle validity checking of UNC hosts differently than
  // for regular IP hosts.
  bool success = DounknownCanonicalizePath<CHAR, UCHAR>(source.path, parsed.path, output, &new_parsed->path);
  CanonicalizeQuery(source.query, parsed.query, query_converter, output, &new_parsed->query);

  // Ignore failure for refs since the URL can probably still be loaded.
  CanonicalizeRef(source.ref, parsed.ref, output, &new_parsed->ref);

  return success;
}

template <typename CHAR, typename UCHAR>
bool DoCanonicalizeGsURL(const URLComponentSource<CHAR>& source,
                         const Parsed& parsed,
                         CharsetConverter* query_converter,
                         CanonOutput* output,
                         Parsed* new_parsed) {
  // Things we don't set in dev: URLs.
  new_parsed->host = Component();
  new_parsed->port = Component();
  new_parsed->username = Component();
  new_parsed->password = Component();
  new_parsed->port = Component();

  // Scheme (known, so we don't bother running it through the more
  // complicated scheme canonicalizer).
  new_parsed->scheme.begin = output->length();
  output->Append("gs://", 5);
  new_parsed->scheme.len = 2;

  // Append the host. For many dev URLs, this will be empty. For UNC, this
  // will be present.
  // TODO(brettw) This doesn't do any checking for host name validity. We
  // should probably handle validity checking of UNC hosts differently than
  // for regular IP hosts.
  bool success = DounknownCanonicalizePath<CHAR, UCHAR>(source.path, parsed.path, output, &new_parsed->path);
  CanonicalizeQuery(source.query, parsed.query, query_converter, output, &new_parsed->query);

  // Ignore failure for refs since the URL can probably still be loaded.
  CanonicalizeRef(source.ref, parsed.ref, output, &new_parsed->ref);

  return success;
}

}  // namespace

bool CanonicalizeUnknownURL(const char* spec,
                            int spec_len,
                            const Parsed& parsed,
                            CharsetConverter* query_converter,
                            CanonOutput* output,
                            Parsed* new_parsed) {
  UNUSED(spec_len);
  return DoCanonicalizeUnknownURL<char, unsigned char>(URLComponentSource<char>(spec), parsed, query_converter, output,
                                                       new_parsed);
}

bool CanonicalizeUnknownURL(const char16* spec,
                            int spec_len,
                            const Parsed& parsed,
                            CharsetConverter* query_converter,
                            CanonOutput* output,
                            Parsed* new_parsed) {
  UNUSED(spec_len);
  return DoCanonicalizeUnknownURL<char16, char16>(URLComponentSource<char16>(spec), parsed, query_converter, output,
                                                  new_parsed);
}

bool CanonicalizeGsURL(const char* spec,
                       int spec_len,
                       const Parsed& parsed,
                       CharsetConverter* query_converter,
                       CanonOutput* output,
                       Parsed* new_parsed) {
  UNUSED(spec_len);
  return DoCanonicalizeGsURL<char, unsigned char>(URLComponentSource<char>(spec), parsed, query_converter, output,
                                                  new_parsed);
}

bool CanonicalizeGsURL(const char16* spec,
                       int spec_len,
                       const Parsed& parsed,
                       CharsetConverter* query_converter,
                       CanonOutput* output,
                       Parsed* new_parsed) {
  UNUSED(spec_len);
  return DoCanonicalizeGsURL<char16, char16>(URLComponentSource<char16>(spec), parsed, query_converter, output,
                                             new_parsed);
}

}  // namespace uri
}  // namespace common
