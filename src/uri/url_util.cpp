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

#include <common/string_util.h>
#include <common/uri/url_canon_internal.h>
#include <common/uri/url_constants.h>
#include <common/uri/url_parse_internal.h>
#include <common/uri/url_util.h>

#include <algorithm>
#include <atomic>
#include <vector>

namespace common {
namespace uri {

namespace {

// A pair for representing a standard scheme name and the SchemeType for it.
struct SchemeWithType {
  std::string scheme;
  SchemeType type;
};

// List of currently registered schemes and associated properties.
struct SchemeRegistry {
  // Standard format schemes (see header for details).
  std::vector<SchemeWithType> standard_schemes = {
      {kHttpsScheme, SCHEME_WITH_HOST_PORT_AND_USER_INFORMATION},
      {kHttpScheme, SCHEME_WITH_HOST_PORT_AND_USER_INFORMATION},
      // Yes, file URLs can have a hostname, so file URLs should be handled as
      // "standard". File URLs never have a port as specified by the SchemeType
      // field.  Unlike other SCHEME_WITH_HOST schemes, the 'host' in a file
      // URL may be empty, a behavior which is special-cased during
      // canonicalization.
      {kFileScheme, SCHEME_WITH_HOST},
      {kDevScheme, SCHEME_WITH_HOST},
      {kGsScheme, SCHEME_WITH_HOST},
      {kS3Scheme, SCHEME_WITH_HOST},
      {kUnknownScheme, SCHEME_WITH_HOST},
      {kUdpScheme, SCHEME_WITH_HOST_AND_PORT},
      {kRtpScheme, SCHEME_WITH_HOST_AND_PORT},
      {kSrtScheme, SCHEME_WITH_HOST_PORT_AND_USER_INFORMATION},
      {kTcpScheme, SCHEME_WITH_HOST_AND_PORT},
      {kWebRTCScheme, SCHEME_WITH_HOST_AND_PORT},
      {kWebRTCsScheme, SCHEME_WITH_HOST_AND_PORT},
      {kRtmpScheme, SCHEME_WITH_HOST_AND_PORT},
      {kRtmpsScheme, SCHEME_WITH_HOST_AND_PORT},
      {kRtmpeScheme, SCHEME_WITH_HOST_AND_PORT},
      {kRtmpsScheme, SCHEME_WITH_HOST_AND_PORT},
      {kRtmfpScheme, SCHEME_WITH_HOST_AND_PORT},
      {kRtspScheme, SCHEME_WITH_HOST_PORT_AND_USER_INFORMATION},
      {kFtpScheme, SCHEME_WITH_HOST_PORT_AND_USER_INFORMATION},
      {kWssScheme, SCHEME_WITH_HOST_PORT_AND_USER_INFORMATION},  // WebSocket secure.
      {kWsScheme, SCHEME_WITH_HOST_PORT_AND_USER_INFORMATION},   // WebSocket.
  };

  // Schemes that are allowed for referrers.
  std::vector<SchemeWithType> referrer_schemes = {
      {kHttpsScheme, SCHEME_WITH_HOST_PORT_AND_USER_INFORMATION},
      {kHttpScheme, SCHEME_WITH_HOST_PORT_AND_USER_INFORMATION},
  };

  // Schemes that do not trigger mixed content warning.
  std::vector<std::string> secure_schemes = {
      kHttpsScheme,
      kDataScheme,
      kWssScheme,
  };

  // Schemes that normal pages cannot link to or access (i.e., with the same
  // security rules as those applied to "file" URLs).
  std::vector<std::string> local_schemes = {kFileScheme, kDevScheme, kGsScheme, kS3Scheme, kUnknownScheme};

  // Schemes that cause pages loaded with them to not have access to pages
  // loaded with any other URL scheme.
  std::vector<std::string> no_access_schemes = {
      kDataScheme,
  };

  // Schemes that can be sent CORS requests.
  std::vector<std::string> cors_enabled_schemes = {
      kHttpsScheme,
      kHttpScheme,
      kDataScheme,
  };

  // Schemes that can be used by web to store data (local storage, etc).
  std::vector<std::string> web_storage_schemes = {
      kHttpsScheme, kHttpScheme, kFileScheme, kFtpScheme, kWssScheme, kWsScheme,
  };

  // Schemes that can bypass the Content-Security-Policy (CSP) checks.
  std::vector<std::string> csp_bypassing_schemes = {};

  bool allow_non_standard_schemes = false;
};

// See the LockSchemeRegistries declaration in the header.
bool scheme_registries_locked = false;

// Gets the scheme registry without locking the schemes. This should *only* be
// used for adding schemes to the registry.
SchemeRegistry* GetSchemeRegistryWithoutLocking() {
  static SchemeRegistry registry;
  return &registry;
}

const SchemeRegistry& GetSchemeRegistry() {
  return *GetSchemeRegistryWithoutLocking();
}

// Pass this enum through for methods which would like to know if whitespace
// removal is necessary.
enum WhitespaceRemovalPolicy {
  REMOVE_WHITESPACE,
  DO_NOT_REMOVE_WHITESPACE,
};

// This template converts a given character type to the corresponding
// StringPiece type.
template <typename CHAR>
struct CharToStringPiece {};
template <>
struct CharToStringPiece<char> {
  typedef StringPiece Piece;
};
template <>
struct CharToStringPiece<char16> {
  typedef StringPiece16 Piece;
};

// Given a string and a range inside the string, compares it to the given
// lower-case |compare_to| buffer.
template <typename CHAR>
inline bool DoCompareSchemeComponent(const CHAR* spec, const Component& component, const char* compare_to) {
  if (!component.is_nonempty())
    return compare_to[0] == 0;  // When component is empty, match empty scheme.
  return LowerCaseEqualsASCII(typename CharToStringPiece<CHAR>::Piece(&spec[component.begin], component.len),
                              compare_to);
}

// Returns true and sets |type| to the SchemeType of the given scheme
// identified by |scheme| within |spec| if in |schemes|.
template <typename CHAR>
bool DoIsInSchemes(const CHAR* spec,
                   const Component& scheme,
                   SchemeType* type,
                   const std::vector<SchemeWithType>& schemes) {
  if (!scheme.is_nonempty())
    return false;  // Empty or invalid schemes are non-standard.

  for (const SchemeWithType& scheme_with_type : schemes) {
    if (LowerCaseEqualsASCII(typename CharToStringPiece<CHAR>::Piece(&spec[scheme.begin], scheme.len),
                             scheme_with_type.scheme)) {
      *type = scheme_with_type.type;
      return true;
    }
  }
  return false;
}

template <typename CHAR>
bool DoIsStandard(const CHAR* spec, const Component& scheme, SchemeType* type) {
  return DoIsInSchemes(spec, scheme, type, GetSchemeRegistry().standard_schemes);
}

template <typename CHAR>
bool DoFindAndCompareScheme(const CHAR* str, int str_len, const char* compare, Component* found_scheme) {
  // Before extracting scheme, canonicalize the URL to remove any whitespace.
  // This matches the canonicalization done in DoCanonicalize function.
  STACK_UNINITIALIZED RawCanonOutputT<CHAR> whitespace_buffer;
  int spec_len;
  const CHAR* spec = RemoveURLWhitespace(str, str_len, &whitespace_buffer, &spec_len, nullptr);

  Component our_scheme;
  if (!ExtractScheme(spec, spec_len, &our_scheme)) {
    // No scheme.
    if (found_scheme)
      *found_scheme = Component();
    return false;
  }
  if (found_scheme)
    *found_scheme = our_scheme;
  return DoCompareSchemeComponent(spec, our_scheme, compare);
}

template <typename CHAR>
bool DoCanonicalize(const CHAR* spec,
                    int spec_len,
                    bool trim_path_end,
                    WhitespaceRemovalPolicy whitespace_policy,
                    CharsetConverter* charset_converter,
                    CanonOutput* output,
                    Parsed* output_parsed) {
  output->ReserveSizeIfNeeded(spec_len);

  // Remove any whitespace from the middle of the relative URL if necessary.
  // Possibly this will result in copying to the new buffer.
  STACK_UNINITIALIZED RawCanonOutputT<CHAR> whitespace_buffer;
  if (whitespace_policy == REMOVE_WHITESPACE) {
    spec =
        RemoveURLWhitespace(spec, spec_len, &whitespace_buffer, &spec_len, &output_parsed->potentially_dangling_markup);
  }

  Parsed parsed_input;
#if defined(WIN32)
  // For Windows, we allow things that look like absolute Windows paths to be
  // fixed up magically to file URLs. This is done for IE compatibility. For
  // example, this will change "c:/foo" into a file URL rather than treating
  // it as a URL with the protocol "c". It also works for UNC ("\\foo\bar.txt").
  // There is similar logic in url_canon_relative.cc for
  //
  // For Max & Unix, we don't do this (the equivalent would be "/foo/bar" which
  // has no meaning as an absolute path name. This is because browsers on Mac
  // & Unix don't generally do this, so there is no compatibility reason for
  // doing so.
  if (DoesBeginUNCPath(spec, 0, spec_len, false) || DoesBeginWindowsDriveSpec(spec, 0, spec_len)) {
    ParseFileURL(spec, spec_len, &parsed_input);
    return CanonicalizeFileURL(spec, spec_len, parsed_input, charset_converter, output, output_parsed);
  }
#endif

  Component scheme;
  if (!ExtractScheme(spec, spec_len, &scheme))
    return false;

  // This is the parsed version of the input URL, we have to canonicalize it
  // before storing it in our object.
  bool success;
  SchemeType scheme_type = SCHEME_WITH_HOST_PORT_AND_USER_INFORMATION;
  if (DoCompareSchemeComponent(spec, scheme, uri::kFileScheme)) {
    // File URLs are special.
    ParseFileURL(spec, spec_len, &parsed_input);
    success = CanonicalizeFileURL(spec, spec_len, parsed_input, charset_converter, output, output_parsed);
  } else if (DoCompareSchemeComponent(spec, scheme, uri::kDevScheme)) {
    // Dev URLs are special.
    ParseDevURL(spec, spec_len, &parsed_input);
    success = CanonicalizeDevURL(spec, spec_len, parsed_input, charset_converter, output, output_parsed);
  } else if (DoCompareSchemeComponent(spec, scheme, uri::kGsScheme)) {
    // Gs URLs are special.
    ParseGsURL(spec, spec_len, &parsed_input);
    success = CanonicalizeGsURL(spec, spec_len, parsed_input, charset_converter, output, output_parsed);
  } else if (DoCompareSchemeComponent(spec, scheme, uri::kS3Scheme)) {
    // S3 URLs are special.
    ParseS3URL(spec, spec_len, &parsed_input);
    success = CanonicalizeS3URL(spec, spec_len, parsed_input, charset_converter, output, output_parsed);
  } else if (DoCompareSchemeComponent(spec, scheme, uri::kUnknownScheme)) {
    // Unknown URLs are special.
    ParseUnknownURL(spec, spec_len, &parsed_input);
    success = CanonicalizeUnknownURL(spec, spec_len, parsed_input, charset_converter, output, output_parsed);
  } else if (DoCompareSchemeComponent(spec, scheme, uri::kUdpScheme)) {
    // Udp URLs are special.
    ParseUdpURL(spec, spec_len, &parsed_input);
    success = CanonicalizeUdpURL(spec, spec_len, parsed_input, charset_converter, output, output_parsed);
  } else if (DoCompareSchemeComponent(spec, scheme, uri::kRtpScheme)) {
    // Rtp URLs are special.
    ParseRtpURL(spec, spec_len, &parsed_input);
    success = CanonicalizeRtpURL(spec, spec_len, parsed_input, charset_converter, output, output_parsed);
  } else if (DoCompareSchemeComponent(spec, scheme, uri::kSrtScheme)) {
    // Srt URLs are special.
    ParseSrtURL(spec, spec_len, &parsed_input);
    success = CanonicalizeSrtURL(spec, spec_len, parsed_input, charset_converter, output, output_parsed);
  } else if (DoCompareSchemeComponent(spec, scheme, uri::kTcpScheme)) {
    // Tcp URLs are special.
    ParseTcpURL(spec, spec_len, &parsed_input);
    success = CanonicalizeTcpURL(spec, spec_len, parsed_input, charset_converter, output, output_parsed);
  } else if (DoCompareSchemeComponent(spec, scheme, uri::kRtmpScheme) ||
             DoCompareSchemeComponent(spec, scheme, uri::kRtmpsScheme) ||
             DoCompareSchemeComponent(spec, scheme, uri::kRtmpeScheme) ||
             DoCompareSchemeComponent(spec, scheme, uri::kRtmptScheme) ||
             DoCompareSchemeComponent(spec, scheme, uri::kRtmfpScheme)) {
    // Rtmp/rtmps URLs are special.
    ParseRtmpURL(spec, spec_len, &parsed_input);
    success = CanonicalizeRtmpURL(spec, spec_len, parsed_input, charset_converter, output, output_parsed);
  } else if (DoCompareSchemeComponent(spec, scheme, uri::kWebRTCScheme) ||
             DoCompareSchemeComponent(spec, scheme, uri::kWebRTCsScheme)) {
    // WebRTC/WebRTCs URLs are special.
    ParseWebRTCURL(spec, spec_len, &parsed_input);
    success = CanonicalizeWebRTCURL(spec, spec_len, parsed_input, charset_converter, output, output_parsed);
  } else if (DoCompareSchemeComponent(spec, scheme, uri::kRtspScheme)) {
    // Rtsp URLs are special.
    ParseRtspURL(spec, spec_len, &parsed_input);
    success = CanonicalizeRtspURL(spec, spec_len, parsed_input, charset_converter, output, output_parsed);
  } else if (DoIsStandard(spec, scheme, &scheme_type)) {
    // All "normal" URLs.
    ParseStandardURL(spec, spec_len, &parsed_input);
    success =
        CanonicalizeStandardURL(spec, spec_len, parsed_input, scheme_type, charset_converter, output, output_parsed);
  } else {
    // "Weird" URLs like data: and javascript:.
    ParsePathURL(spec, spec_len, trim_path_end, &parsed_input);
    success = CanonicalizePathURL(spec, spec_len, parsed_input, output, output_parsed);
  }
  return success;
}

template <typename CHAR>
bool DoResolveRelative(const char* base_spec,
                       int base_spec_len,
                       const Parsed& base_parsed,
                       const CHAR* in_relative,
                       int in_relative_length,
                       CharsetConverter* charset_converter,
                       CanonOutput* output,
                       Parsed* output_parsed) {
  // Remove any whitespace from the middle of the relative URL, possibly
  // copying to the new buffer.
  STACK_UNINITIALIZED RawCanonOutputT<CHAR> whitespace_buffer;
  int relative_length;
  const CHAR* relative = RemoveURLWhitespace(in_relative, in_relative_length, &whitespace_buffer, &relative_length,
                                             &output_parsed->potentially_dangling_markup);

  bool base_is_authority_based = false;
  bool base_is_hierarchical = false;
  if (base_spec && base_parsed.scheme.is_nonempty()) {
    int after_scheme = base_parsed.scheme.end() + 1;  // Skip past the colon.
    int num_slashes = CountConsecutiveSlashes(base_spec, after_scheme, base_spec_len);
    base_is_authority_based = num_slashes > 1;
    base_is_hierarchical = num_slashes > 0;
  }

  SchemeType unused_scheme_type = SCHEME_WITH_HOST_PORT_AND_USER_INFORMATION;
  bool standard_base_scheme =
      base_parsed.scheme.is_nonempty() && DoIsStandard(base_spec, base_parsed.scheme, &unused_scheme_type);

  bool is_relative;
  Component relative_component;
  if (!IsRelativeURL(base_spec, base_parsed, relative, relative_length, (base_is_hierarchical || standard_base_scheme),
                     &is_relative, &relative_component)) {
    // Error resolving.
    return false;
  }

  // Don't reserve buffer space here. Instead, reserve in DoCanonicalize and
  // ReserveRelativeURL, to enable more accurate buffer sizes.

  // Pretend for a moment that |base_spec| is a standard URL. Normally
  // non-standard URLs are treated as PathURLs, but if the base has an
  // authority we would like to preserve it.
  if (is_relative && base_is_authority_based && !standard_base_scheme) {
    Parsed base_parsed_authority;
    ParseStandardURL(base_spec, base_spec_len, &base_parsed_authority);
    if (base_parsed_authority.host.is_nonempty()) {
      STACK_UNINITIALIZED RawCanonOutputT<char> temporary_output;
      bool did_resolve_succeed =
          ResolveRelativeURL(base_spec, base_parsed_authority, false, relative, relative_component, charset_converter,
                             &temporary_output, output_parsed);
      // The output_parsed is incorrect at this point (because it was built
      // based on base_parsed_authority instead of base_parsed) and needs to be
      // re-created.
      DoCanonicalize(temporary_output.data(), temporary_output.length(), true, REMOVE_WHITESPACE, charset_converter,
                     output, output_parsed);
      return did_resolve_succeed;
    }
  } else if (is_relative) {
    // Relative, resolve and canonicalize.
    bool file_base_scheme =
        base_parsed.scheme.is_nonempty() && (DoCompareSchemeComponent(base_spec, base_parsed.scheme, kFileScheme) ||
                                             DoCompareSchemeComponent(base_spec, base_parsed.scheme, kDevScheme));
    return ResolveRelativeURL(base_spec, base_parsed, file_base_scheme, relative, relative_component, charset_converter,
                              output, output_parsed);
  }

  // Not relative, canonicalize the input.
  return DoCanonicalize(relative, relative_length, true, DO_NOT_REMOVE_WHITESPACE, charset_converter, output,
                        output_parsed);
}

template <typename CHAR>
bool DoReplaceComponents(const char* spec,
                         int spec_len,
                         const Parsed& parsed,
                         const Replacements<CHAR>& replacements,
                         CharsetConverter* charset_converter,
                         CanonOutput* output,
                         Parsed* out_parsed) {
  // If the scheme is overridden, just do a simple string substitution and
  // re-parse the whole thing. There are lots of edge cases that we really don't
  // want to deal with. Like what happens if I replace "http://e:8080/foo"
  // with a file. Does it become "file:///E:/8080/foo" where the port number
  // becomes part of the path? Parsing that string as a file URL says "yes"
  // but almost no sane rule for dealing with the components individually would
  // come up with that.
  //
  // Why allow these crazy cases at all? Programatically, there is almost no
  // case for replacing the scheme. The most common case for hitting this is
  // in JS when building up a URL using the location object. In this case, the
  // JS code expects the string substitution behavior:
  //   http://www.w3.org/TR/2008/WD-html5-20080610/structured.html#common3
  if (replacements.IsSchemeOverridden()) {
    // Canonicalize the new scheme so it is 8-bit and can be concatenated with
    // the existing spec.
    STACK_UNINITIALIZED RawCanonOutput<128> scheme_replaced;
    Component scheme_replaced_parsed;
    CanonicalizeScheme(replacements.sources().scheme, replacements.components().scheme, &scheme_replaced,
                       &scheme_replaced_parsed);

    // We can assume that the input is canonicalized, which means it always has
    // a colon after the scheme (or where the scheme would be).
    int spec_after_colon = parsed.scheme.is_valid() ? parsed.scheme.end() + 1 : 1;
    if (spec_len - spec_after_colon > 0) {
      scheme_replaced.Append(&spec[spec_after_colon], spec_len - spec_after_colon);
    }

    // We now need to completely re-parse the resulting string since its meaning
    // may have changed with the different scheme.
    STACK_UNINITIALIZED RawCanonOutput<128> recanonicalized;
    Parsed recanonicalized_parsed;
    DoCanonicalize(scheme_replaced.data(), scheme_replaced.length(), true, REMOVE_WHITESPACE, charset_converter,
                   &recanonicalized, &recanonicalized_parsed);

    // Recurse using the version with the scheme already replaced. This will now
    // use the replacement rules for the new scheme.
    //
    // Warning: this code assumes that ReplaceComponents will re-check all
    // components for validity. This is because we can't fail if DoCanonicalize
    // failed above since theoretically the thing making it fail could be
    // getting replaced here. If ReplaceComponents didn't re-check everything,
    // we wouldn't know if something *not* getting replaced is a problem.
    // If the scheme-specific replacers are made more intelligent so they don't
    // re-check everything, we should instead re-canonicalize the whole thing
    // after this call to check validity (this assumes replacing the scheme is
    // much much less common than other types of replacements, like clearing the
    // ref).
    Replacements<CHAR> replacements_no_scheme = replacements;
    replacements_no_scheme.SetScheme(NULL, Component());
    return DoReplaceComponents(recanonicalized.data(), recanonicalized.length(), recanonicalized_parsed,
                               replacements_no_scheme, charset_converter, output, out_parsed);
  }

  // TODO(csharrison): We could be smarter about size to reserve if this is done
  // in callers below, and the code checks to see which components are being
  // replaced, and with what length. If this ends up being a hot spot it should
  // be changed.
  output->ReserveSizeIfNeeded(spec_len);

  // If we get here, then we know the scheme doesn't need to be replaced, so can
  // just key off the scheme in the spec to know how to do the replacements.
  if (DoCompareSchemeComponent(spec, parsed.scheme, uri::kFileScheme) ||
      DoCompareSchemeComponent(spec, parsed.scheme, uri::kDevScheme)) {
    return ReplaceFileURL(spec, parsed, replacements, charset_converter, output, out_parsed);
  }
  SchemeType scheme_type = SCHEME_WITH_HOST_PORT_AND_USER_INFORMATION;
  if (DoIsStandard(spec, parsed.scheme, &scheme_type)) {
    return ReplaceStandardURL(spec, parsed, replacements, scheme_type, charset_converter, output, out_parsed);
  }

  // Default is a path URL.
  return ReplacePathURL(spec, parsed, replacements, output, out_parsed);
}

void DoSchemeModificationPreamble() {
  // If this assert triggers, it means you've called Add*Scheme after
  // LockSchemeRegistries has been called (see the header file for
  // LockSchemeRegistries for more).
  //
  // This normally means you're trying to set up a new scheme too late in your
  // application's init process. Locate where your app does this initialization
  // and calls LockSchemeRegistries, and add your new scheme there.
  DCHECK(!scheme_registries_locked) << "Trying to add a scheme after the lists have been locked.";
}

void DoAddScheme(const char* new_scheme, std::vector<std::string>* schemes) {
  DoSchemeModificationPreamble();
  DCHECK(schemes);
  DCHECK(strlen(new_scheme) > 0);
  DCHECK_EQ(ToLowerASCII(new_scheme), new_scheme);
  DCHECK(std::find(schemes->begin(), schemes->end(), new_scheme) == schemes->end());
  schemes->push_back(new_scheme);
}

void DoAddSchemeWithType(const char* new_scheme, SchemeType type, std::vector<SchemeWithType>* schemes) {
  DoSchemeModificationPreamble();
  DCHECK(schemes);
  DCHECK(strlen(new_scheme) > 0);
  DCHECK_EQ(ToLowerASCII(new_scheme), new_scheme);
  DCHECK(std::find_if(schemes->begin(), schemes->end(), [&new_scheme](const SchemeWithType& scheme) {
           return scheme.scheme == new_scheme;
         }) == schemes->end());
  schemes->push_back({new_scheme, type});
}

}  // namespace

void ClearSchemesForTests() {
  DCHECK(!scheme_registries_locked) << "Schemes already locked "
                                    << "(use ScopedSchemeRegistryForTests to relax for tests).";
  *GetSchemeRegistryWithoutLocking() = SchemeRegistry();
}

class ScopedSchemeRegistryInternal {
 public:
  ScopedSchemeRegistryInternal() : registry_(std::make_unique<SchemeRegistry>(*GetSchemeRegistryWithoutLocking())) {
    scheme_registries_locked = false;
  }
  ~ScopedSchemeRegistryInternal() {
    *GetSchemeRegistryWithoutLocking() = *registry_;
    scheme_registries_locked = true;
  }

 private:
  std::unique_ptr<SchemeRegistry> registry_;
};

ScopedSchemeRegistryForTests::ScopedSchemeRegistryForTests()
    : internal_(std::make_unique<ScopedSchemeRegistryInternal>()) {}

ScopedSchemeRegistryForTests::~ScopedSchemeRegistryForTests() = default;

void EnableNonStandardSchemesForAndroidWebView() {
  DoSchemeModificationPreamble();
  GetSchemeRegistryWithoutLocking()->allow_non_standard_schemes = true;
}

bool AllowNonStandardSchemesForAndroidWebView() {
  return GetSchemeRegistry().allow_non_standard_schemes;
}

void AddStandardScheme(const char* new_scheme, SchemeType type) {
  DoAddSchemeWithType(new_scheme, type, &GetSchemeRegistryWithoutLocking()->standard_schemes);
}

void AddReferrerScheme(const char* new_scheme, SchemeType type) {
  DoAddSchemeWithType(new_scheme, type, &GetSchemeRegistryWithoutLocking()->referrer_schemes);
}

void AddSecureScheme(const char* new_scheme) {
  DoAddScheme(new_scheme, &GetSchemeRegistryWithoutLocking()->secure_schemes);
}

const std::vector<std::string>& GetSecureSchemes() {
  return GetSchemeRegistry().secure_schemes;
}

void AddLocalScheme(const char* new_scheme) {
  DoAddScheme(new_scheme, &GetSchemeRegistryWithoutLocking()->local_schemes);
}

const std::vector<std::string>& GetLocalSchemes() {
  return GetSchemeRegistry().local_schemes;
}

void AddNoAccessScheme(const char* new_scheme) {
  DoAddScheme(new_scheme, &GetSchemeRegistryWithoutLocking()->no_access_schemes);
}

const std::vector<std::string>& GetNoAccessSchemes() {
  return GetSchemeRegistry().no_access_schemes;
}

void AddCorsEnabledScheme(const char* new_scheme) {
  DoAddScheme(new_scheme, &GetSchemeRegistryWithoutLocking()->cors_enabled_schemes);
}

const std::vector<std::string>& GetCorsEnabledSchemes() {
  return GetSchemeRegistry().cors_enabled_schemes;
}

void AddWebStorageScheme(const char* new_scheme) {
  DoAddScheme(new_scheme, &GetSchemeRegistryWithoutLocking()->web_storage_schemes);
}

const std::vector<std::string>& GetWebStorageSchemes() {
  return GetSchemeRegistry().web_storage_schemes;
}

void AddCSPBypassingScheme(const char* new_scheme) {
  DoAddScheme(new_scheme, &GetSchemeRegistryWithoutLocking()->csp_bypassing_schemes);
}

const std::vector<std::string>& GetCSPBypassingSchemes() {
  return GetSchemeRegistry().csp_bypassing_schemes;
}

void LockSchemeRegistries() {
  scheme_registries_locked = true;
}

bool IsStandard(const char* spec, const Component& scheme) {
  SchemeType unused_scheme_type;
  return DoIsStandard(spec, scheme, &unused_scheme_type);
}

bool GetStandardSchemeType(const char* spec, const Component& scheme, SchemeType* type) {
  return DoIsStandard(spec, scheme, type);
}

bool GetStandardSchemeType(const char16* spec, const Component& scheme, SchemeType* type) {
  return DoIsStandard(spec, scheme, type);
}

bool IsStandard(const char16* spec, const Component& scheme) {
  SchemeType unused_scheme_type;
  return DoIsStandard(spec, scheme, &unused_scheme_type);
}

bool IsReferrerScheme(const char* spec, const Component& scheme) {
  SchemeType unused_scheme_type;
  return DoIsInSchemes(spec, scheme, &unused_scheme_type, GetSchemeRegistry().referrer_schemes);
}

bool FindAndCompareScheme(const char* str, int str_len, const char* compare, Component* found_scheme) {
  return DoFindAndCompareScheme(str, str_len, compare, found_scheme);
}

bool FindAndCompareScheme(const char16* str, int str_len, const char* compare, Component* found_scheme) {
  return DoFindAndCompareScheme(str, str_len, compare, found_scheme);
}

bool DomainIs(StringPiece canonical_host, StringPiece canonical_domain) {
  if (canonical_host.empty() || canonical_domain.empty())
    return false;

  // If the host name ends with a dot but the input domain doesn't, then we
  // ignore the dot in the host name.
  size_t host_len = canonical_host.length();
  if (canonical_host.back() == '.' && canonical_domain.back() != '.')
    --host_len;

  if (host_len < canonical_domain.length())
    return false;

  // |host_first_pos| is the start of the compared part of the host name, not
  // start of the whole host name.
  const char* host_first_pos = canonical_host.data() + host_len - canonical_domain.length();

  if (StringPiece(host_first_pos, canonical_domain.length()) != canonical_domain) {
    return false;
  }

  // Make sure there aren't extra characters in host before the compared part;
  // if the host name is longer than the input domain name, then the character
  // immediately before the compared part should be a dot. For example,
  // www.google.com has domain "google.com", but www.iamnotgoogle.com does not.
  if (canonical_domain[0] != '.' && host_len > canonical_domain.length() && *(host_first_pos - 1) != '.') {
    return false;
  }

  return true;
}

bool HostIsIPAddress(StringPiece host) {
  STACK_UNINITIALIZED uri::RawCanonOutputT<char, 128> ignored_output;
  uri::CanonHostInfo host_info;
  uri::CanonicalizeIPAddress(host.data(), Component(0, host.length()), &ignored_output, &host_info);
  return host_info.IsIPAddress();
}

bool Canonicalize(const char* spec,
                  int spec_len,
                  bool trim_path_end,
                  CharsetConverter* charset_converter,
                  CanonOutput* output,
                  Parsed* output_parsed) {
  return DoCanonicalize(spec, spec_len, trim_path_end, REMOVE_WHITESPACE, charset_converter, output, output_parsed);
}

bool Canonicalize(const char16* spec,
                  int spec_len,
                  bool trim_path_end,
                  CharsetConverter* charset_converter,
                  CanonOutput* output,
                  Parsed* output_parsed) {
  return DoCanonicalize(spec, spec_len, trim_path_end, REMOVE_WHITESPACE, charset_converter, output, output_parsed);
}

bool ResolveRelative(const char* base_spec,
                     int base_spec_len,
                     const Parsed& base_parsed,
                     const char* relative,
                     int relative_length,
                     CharsetConverter* charset_converter,
                     CanonOutput* output,
                     Parsed* output_parsed) {
  return DoResolveRelative(base_spec, base_spec_len, base_parsed, relative, relative_length, charset_converter, output,
                           output_parsed);
}

bool ResolveRelative(const char* base_spec,
                     int base_spec_len,
                     const Parsed& base_parsed,
                     const char16* relative,
                     int relative_length,
                     CharsetConverter* charset_converter,
                     CanonOutput* output,
                     Parsed* output_parsed) {
  return DoResolveRelative(base_spec, base_spec_len, base_parsed, relative, relative_length, charset_converter, output,
                           output_parsed);
}

bool ReplaceComponents(const char* spec,
                       int spec_len,
                       const Parsed& parsed,
                       const Replacements<char>& replacements,
                       CharsetConverter* charset_converter,
                       CanonOutput* output,
                       Parsed* out_parsed) {
  return DoReplaceComponents(spec, spec_len, parsed, replacements, charset_converter, output, out_parsed);
}

bool ReplaceComponents(const char* spec,
                       int spec_len,
                       const Parsed& parsed,
                       const Replacements<char16>& replacements,
                       CharsetConverter* charset_converter,
                       CanonOutput* output,
                       Parsed* out_parsed) {
  return DoReplaceComponents(spec, spec_len, parsed, replacements, charset_converter, output, out_parsed);
}

void DecodeURLEscapeSequences(const char* input, int length, DecodeURLMode mode, CanonOutputW* output) {
  STACK_UNINITIALIZED RawCanonOutputT<char> unescaped_chars;
  for (int i = 0; i < length; i++) {
    if (input[i] == '%') {
      unsigned char ch;
      if (DecodeEscaped(input, &i, length, &ch)) {
        unescaped_chars.push_back(ch);
      } else {
        // Invalid escape sequence, copy the percent literal.
        unescaped_chars.push_back('%');
      }
    } else {
      // Regular non-escaped 8-bit character.
      unescaped_chars.push_back(input[i]);
    }
  }

  int output_initial_length = output->length();
  // Convert that 8-bit to UTF-16. It's not clear IE does this at all to
  // JavaScript URLs, but Firefox and Safari do.
  for (int i = 0; i < unescaped_chars.length(); i++) {
    unsigned char uch = static_cast<unsigned char>(unescaped_chars.at(i));
    if (uch < 0x80) {
      // Non-UTF-8, just append directly
      output->push_back(uch);
    } else {
      // next_ch will point to the last character of the decoded
      // character.
      int next_character = i;
      unsigned code_point;
      if (ReadUTFChar(unescaped_chars.data(), &next_character, unescaped_chars.length(), &code_point)) {
        // Valid UTF-8 character, convert to UTF-16.
        AppendUTF16Value(code_point, output);
        i = next_character;
      } else if (mode == DecodeURLMode::kUTF8) {
        DCHECK_EQ(code_point, 0xFFFDU);
        AppendUTF16Value(code_point, output);
        i = next_character;
      } else {
        // If there are any sequences that are not valid UTF-8, we
        // revert |output| changes, and promote any bytes to UTF-16. We
        // copy all characters from the beginning to the end of the
        // identified sequence.
        output->set_length(output_initial_length);
        for (int j = 0; j < unescaped_chars.length(); ++j)
          output->push_back(static_cast<unsigned char>(unescaped_chars.at(j)));
        break;
      }
    }
  }
}

void EncodeURIComponent(const char* input, int length, CanonOutput* output) {
  for (int i = 0; i < length; ++i) {
    unsigned char c = static_cast<unsigned char>(input[i]);
    if (IsComponentChar(c))
      output->push_back(c);
    else
      AppendEscapedChar(c, output);
  }
}

bool CompareSchemeComponent(const char* spec, const Component& component, const char* compare_to) {
  return DoCompareSchemeComponent(spec, component, compare_to);
}

bool CompareSchemeComponent(const char16* spec, const Component& component, const char* compare_to) {
  return DoCompareSchemeComponent(spec, component, compare_to);
}

}  // namespace uri
}  // namespace common
