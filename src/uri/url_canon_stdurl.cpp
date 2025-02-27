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
#include <common/uri/url_constants.h>

namespace common {
namespace uri {

namespace {

template <typename CHAR, typename UCHAR>
bool DoCanonicalizeStandardURL(const URLComponentSource<CHAR>& source,
                               const Parsed& parsed,
                               SchemeType scheme_type,
                               CharsetConverter* query_converter,
                               CanonOutput* output,
                               Parsed* new_parsed) {
  // Scheme: this will append the colon.
  bool success = CanonicalizeScheme(source.scheme, parsed.scheme, output, &new_parsed->scheme);

  bool scheme_supports_user_info = (scheme_type == SCHEME_WITH_HOST_PORT_AND_USER_INFORMATION);
  bool scheme_supports_ports =
      (scheme_type == SCHEME_WITH_HOST_PORT_AND_USER_INFORMATION || scheme_type == SCHEME_WITH_HOST_AND_PORT);

  // Authority (username, password, host, port)
  bool have_authority;
  if ((scheme_supports_user_info && (parsed.username.is_valid() || parsed.password.is_valid())) ||
      parsed.host.is_nonempty() || (scheme_supports_ports && parsed.port.is_valid())) {
    have_authority = true;

    // Only write the authority separators when we have a scheme.
    if (parsed.scheme.is_valid()) {
      output->push_back('/');
      output->push_back('/');
    }

    // User info: the canonicalizer will handle the : and @.
    if (scheme_supports_user_info) {
      success &= CanonicalizeUserInfo(source.username, parsed.username, source.password, parsed.password, output,
                                      &new_parsed->username, &new_parsed->password);
    } else {
      new_parsed->username.reset();
      new_parsed->password.reset();
    }

    success &= CanonicalizeHost(source.host, parsed.host, output, &new_parsed->host);

    // Host must not be empty for standard URLs.
    if (!parsed.host.is_nonempty())
      success = false;

    // Port: the port canonicalizer will handle the colon.
    if (scheme_supports_ports) {
      int default_port = DefaultPortForScheme(&output->data()[new_parsed->scheme.begin], new_parsed->scheme.len);
      success &= CanonicalizePort(source.port, parsed.port, default_port, output, &new_parsed->port);
    } else {
      new_parsed->port.reset();
    }
  } else {
    // No authority, clear the components.
    have_authority = false;
    new_parsed->host.reset();
    new_parsed->username.reset();
    new_parsed->password.reset();
    new_parsed->port.reset();
    success = false;  // Standard URLs must have an authority.
  }

  // Path
  if (parsed.path.is_valid()) {
    success &= CanonicalizePath(source.path, parsed.path, output, &new_parsed->path);
  } else if (have_authority || parsed.query.is_valid() || parsed.ref.is_valid()) {
    // When we have an empty path, make up a path when we have an authority
    // or something following the path. The only time we allow an empty
    // output path is when there is nothing else.
    new_parsed->path = Component(output->length(), 1);
    output->push_back('/');
  } else {
    // No path at all
    new_parsed->path.reset();
  }

  // Query
  CanonicalizeQuery(source.query, parsed.query, query_converter, output, &new_parsed->query);

  // Ref: ignore failure for this, since the page can probably still be loaded.
  CanonicalizeRef(source.ref, parsed.ref, output, &new_parsed->ref);

  return success;
}

}  // namespace

// Returns the default port for the given canonical scheme, or PORT_UNSPECIFIED
// if the scheme is unknown.
int DefaultPortForScheme(const char* scheme, int scheme_len) {
  int default_port = PORT_UNSPECIFIED;
  switch (scheme_len) {
    case 4:
      if (!strncmp(scheme, kHttpScheme, scheme_len))
        default_port = 80;
      else if (!strncmp(scheme, kRtmpScheme, scheme_len))
        default_port = 1935;
      else if (!strncmp(scheme, kRtspScheme, scheme_len))
        default_port = 554;
      break;
    case 5:
      if (!strncmp(scheme, kHttpsScheme, scheme_len))
        default_port = 443;
      break;
    case 6:
      if (!strncmp(scheme, kWebRTCScheme, scheme_len))
        default_port = 80;
      break;
    case 7:
      if (!strncmp(scheme, kWebRTCsScheme, scheme_len))
        default_port = 443;
      break;
    case 3:
      if (!strncmp(scheme, kFtpScheme, scheme_len))
        default_port = 21;
      else if (!strncmp(scheme, kWssScheme, scheme_len))
        default_port = 443;
      break;
    case 2:
      if (!strncmp(scheme, kWsScheme, scheme_len))
        default_port = 80;
      break;
  }
  return default_port;
}

bool CanonicalizeStandardURL(const char* spec,
                             int spec_len,
                             const Parsed& parsed,
                             SchemeType scheme_type,
                             CharsetConverter* query_converter,
                             CanonOutput* output,
                             Parsed* new_parsed) {
  UNUSED(spec_len);
  return DoCanonicalizeStandardURL<char, unsigned char>(URLComponentSource<char>(spec), parsed, scheme_type,
                                                        query_converter, output, new_parsed);
}

bool CanonicalizeStandardURL(const char16* spec,
                             int spec_len,
                             const Parsed& parsed,
                             SchemeType scheme_type,
                             CharsetConverter* query_converter,
                             CanonOutput* output,
                             Parsed* new_parsed) {
  UNUSED(spec_len);
  return DoCanonicalizeStandardURL<char16, char16>(URLComponentSource<char16>(spec), parsed, scheme_type,
                                                   query_converter, output, new_parsed);
}

// It might be nice in the future to optimize this so unchanged components don't
// need to be recanonicalized. This is especially true since the common case for
// ReplaceComponents is removing things we don't want, like reference fragments
// and usernames. These cases can become more efficient if we can assume the
// rest of the URL is OK with these removed (or only the modified parts
// recanonicalized). This would be much more complex to implement, however.
//
// You would also need to update DoReplaceComponents in url_util.cc which
// relies on this re-checking everything (see the comment there for why).
bool ReplaceStandardURL(const char* base,
                        const Parsed& base_parsed,
                        const Replacements<char>& replacements,
                        SchemeType scheme_type,
                        CharsetConverter* query_converter,
                        CanonOutput* output,
                        Parsed* new_parsed) {
  URLComponentSource<char> source(base);
  Parsed parsed(base_parsed);
  SetupOverrideComponents(base, replacements, &source, &parsed);
  return DoCanonicalizeStandardURL<char, unsigned char>(source, parsed, scheme_type, query_converter, output,
                                                        new_parsed);
}

// For 16-bit replacements, we turn all the replacements into UTF-8 so the
// regular code path can be used.
bool ReplaceStandardURL(const char* base,
                        const Parsed& base_parsed,
                        const Replacements<char16>& replacements,
                        SchemeType scheme_type,
                        CharsetConverter* query_converter,
                        CanonOutput* output,
                        Parsed* new_parsed) {
  RawCanonOutput<1024> utf8;
  URLComponentSource<char> source(base);
  Parsed parsed(base_parsed);
  SetupUTF16OverrideComponents(base, replacements, &utf8, &source, &parsed);
  return DoCanonicalizeStandardURL<char, unsigned char>(source, parsed, scheme_type, query_converter, output,
                                                        new_parsed);
}

}  // namespace uri
}  // namespace common
