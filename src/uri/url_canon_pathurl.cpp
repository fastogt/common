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

#include <common/uri/url_canon_internal.h>

namespace common {
namespace uri {

namespace {

// Canonicalize the given |component| from |source| into |output| and
// |new_component|. If |separator| is non-zero, it is pre-pended to |output|
// prior to the canonicalized component; i.e. for the '?' or '#' characters.
template <typename CHAR, typename UCHAR>
void DoCanonicalizePathComponent(const CHAR* source,
                                 const Component& component,
                                 char separator,
                                 CanonOutput* output,
                                 Component* new_component) {
  if (component.is_valid()) {
    if (separator)
      output->push_back(separator);
    // Copy the path using path URL's more lax escaping rules (think for
    // javascript:). We convert to UTF-8 and escape non-ASCII, but leave all
    // ASCII characters alone. This helps readability of JavaStript.
    new_component->begin = output->length();
    int end = component.end();
    for (int i = component.begin; i < end; i++) {
      UCHAR uch = static_cast<UCHAR>(source[i]);
      if (uch < 0x20 || uch >= 0x80)
        AppendUTF8EscapedChar(source, &i, end, output);
      else
        output->push_back(static_cast<char>(uch));
    }
    new_component->len = output->length() - new_component->begin;
  } else {
    // Empty part.
    new_component->reset();
  }
}

template <typename CHAR, typename UCHAR>
bool DoCanonicalizePathURL(const URLComponentSource<CHAR>& source,
                           const Parsed& parsed,
                           CanonOutput* output,
                           Parsed* new_parsed) {
  // Scheme: this will append the colon.
  bool success = CanonicalizeScheme(source.scheme, parsed.scheme, output, &new_parsed->scheme);

  // We assume there's no authority for path URLs. Note that hosts should never
  // have -1 length.
  new_parsed->username.reset();
  new_parsed->password.reset();
  new_parsed->host.reset();
  new_parsed->port.reset();
  // We allow path URLs to have the path, query and fragment components, but we
  // will canonicalize each of the via the weaker path URL rules.
  //
  // Note: parsing the path part should never cause a failure, see
  // https://url.spec.whatwg.org/#cannot-be-a-base-url-path-state
  DoCanonicalizePathComponent<CHAR, UCHAR>(source.path, parsed.path, '\0', output, &new_parsed->path);
  DoCanonicalizePathComponent<CHAR, UCHAR>(source.query, parsed.query, '?', output, &new_parsed->query);
  DoCanonicalizePathComponent<CHAR, UCHAR>(source.ref, parsed.ref, '#', output, &new_parsed->ref);

  return success;
}

}  // namespace

bool CanonicalizePathURL(const char* spec,
                         int spec_len,
                         const Parsed& parsed,
                         CanonOutput* output,
                         Parsed* new_parsed) {
  UNUSED(spec_len);
  return DoCanonicalizePathURL<char, unsigned char>(URLComponentSource<char>(spec), parsed, output, new_parsed);
}

bool CanonicalizePathURL(const char16* spec,
                         int spec_len,
                         const Parsed& parsed,
                         CanonOutput* output,
                         Parsed* new_parsed) {
  UNUSED(spec_len);
  return DoCanonicalizePathURL<char16, char16>(URLComponentSource<char16>(spec), parsed, output, new_parsed);
}

bool ReplacePathURL(const char* base,
                    const Parsed& base_parsed,
                    const Replacements<char>& replacements,
                    CanonOutput* output,
                    Parsed* new_parsed) {
  URLComponentSource<char> source(base);
  Parsed parsed(base_parsed);
  SetupOverrideComponents(base, replacements, &source, &parsed);
  return DoCanonicalizePathURL<char, unsigned char>(source, parsed, output, new_parsed);
}

bool ReplacePathURL(const char* base,
                    const Parsed& base_parsed,
                    const Replacements<char16>& replacements,
                    CanonOutput* output,
                    Parsed* new_parsed) {
  RawCanonOutput<1024> utf8;
  URLComponentSource<char> source(base);
  Parsed parsed(base_parsed);
  SetupUTF16OverrideComponents(base, replacements, &utf8, &source, &parsed);
  return DoCanonicalizePathURL<char, unsigned char>(source, parsed, output, new_parsed);
}

}  // namespace uri
}  // namespace common
