/*  Copyright (C) 2014-2020 FastoGT. All right reserved.

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

#include <common/uri/gurl.h>

#include <stddef.h>

#include <algorithm>
#include <ostream>
#include <utility>

#include <common/string_util.h>
#include <common/uri/url_canon.h>
#include <common/uri/url_parse.h>
#include <common/uri/url_util.h>

namespace common {
namespace uri {

GURL::GURL() : is_valid_(false) {}

GURL::GURL(const GURL& other) : spec_(other.spec_), is_valid_(other.is_valid_), parsed_(other.parsed_) {
  if (other.inner_url_)
    inner_url_.reset(new GURL(*other.inner_url_));
  // Valid filesystem urls should always have an inner_url_.
  DCHECK(!is_valid_ || inner_url_);
}

GURL::GURL(GURL&& other) noexcept
    : spec_(std::move(other.spec_)),
      is_valid_(other.is_valid_),
      parsed_(other.parsed_),
      inner_url_(std::move(other.inner_url_)) {
  other.is_valid_ = false;
  other.parsed_ = Parsed();
}

GURL::GURL(StringPiece url_string) {
  InitCanonical(url_string, true);
}

GURL::GURL(StringPiece16 url_string) {
  InitCanonical(url_string, true);
}

GURL::GURL(const std::string& url_string, RetainWhiteSpaceSelector) {
  InitCanonical(StringPiece(url_string), false);
}

GURL::GURL(const char* canonical_spec, size_t canonical_spec_len, const Parsed& parsed, bool is_valid)
    : spec_(canonical_spec, canonical_spec_len), is_valid_(is_valid), parsed_(parsed) {
  InitializeFromCanonicalSpec();
}

GURL::GURL(std::string canonical_spec, const Parsed& parsed, bool is_valid)
    : spec_(std::move(canonical_spec)), is_valid_(is_valid), parsed_(parsed) {
  InitializeFromCanonicalSpec();
}

template <typename STR>
void GURL::InitCanonical(BasicStringPiece<STR> input_spec, bool trim_path_end) {
  StdStringCanonOutput output(&spec_);
  is_valid_ =
      Canonicalize(input_spec.data(), static_cast<int>(input_spec.length()), trim_path_end, NULL, &output, &parsed_);

  output.Complete();  // Must be done before using string.
  // Valid URLs always have non-empty specs.
  DCHECK(!is_valid_ || !spec_.empty());
}

void GURL::InitializeFromCanonicalSpec() {}

GURL::~GURL() = default;

GURL& GURL::operator=(const GURL& other) {
  spec_ = other.spec_;
  is_valid_ = other.is_valid_;
  parsed_ = other.parsed_;

  if (!other.inner_url_)
    inner_url_.reset();
  else if (inner_url_)
    *inner_url_ = *other.inner_url_;
  else
    inner_url_.reset(new GURL(*other.inner_url_));

  return *this;
}

GURL& GURL::operator=(GURL&& other) noexcept {
  spec_ = std::move(other.spec_);
  is_valid_ = other.is_valid_;
  parsed_ = other.parsed_;
  inner_url_ = std::move(other.inner_url_);

  other.is_valid_ = false;
  other.parsed_ = Parsed();
  return *this;
}

const std::string& GURL::spec() const {
  if (is_valid_ || spec_.empty())
    return spec_;

  DCHECK(false) << "Trying to get the spec of an invalid URL!";
  return EmptyString();
}

bool GURL::operator<(const GURL& other) const {
  return spec_ < other.spec_;
}

bool GURL::operator>(const GURL& other) const {
  return spec_ > other.spec_;
}

// Note: code duplicated below (it's inconvenient to use a template here).
GURL GURL::Resolve(StringPiece relative) const {
  // Not allowed for invalid URLs.
  if (!is_valid_)
    return GURL();

  GURL result;
  StdStringCanonOutput output(&result.spec_);
  if (!ResolveRelative(spec_.data(), static_cast<int>(spec_.length()), parsed_, relative.data(),
                       static_cast<int>(relative.length()), nullptr, &output, &result.parsed_)) {
    // Error resolving, return an empty URL.
    return GURL();
  }

  output.Complete();
  result.is_valid_ = true;
  return result;
}

// Note: code duplicated above (it's inconvenient to use a template here).
GURL GURL::Resolve(StringPiece16 relative) const {
  // Not allowed for invalid URLs.
  if (!is_valid_)
    return GURL();

  GURL result;
  StdStringCanonOutput output(&result.spec_);
  if (!ResolveRelative(spec_.data(), static_cast<int>(spec_.length()), parsed_, relative.data(),
                       static_cast<int>(relative.length()), nullptr, &output, &result.parsed_)) {
    // Error resolving, return an empty URL.
    return GURL();
  }

  output.Complete();
  result.is_valid_ = true;
  return result;
}

// Note: code duplicated below (it's inconvenient to use a template here).
GURL GURL::ReplaceComponents(const uri::Replacements<char>& replacements) const {
  GURL result;

  // Not allowed for invalid URLs.
  if (!is_valid_)
    return GURL();

  StdStringCanonOutput output(&result.spec_);
  result.is_valid_ = uri::ReplaceComponents(spec_.data(), static_cast<int>(spec_.length()), parsed_, replacements, NULL,
                                            &output, &result.parsed_);

  output.Complete();
  return result;
}

// Note: code duplicated above (it's inconvenient to use a template here).
GURL GURL::ReplaceComponents(const uri::Replacements<char16>& replacements) const {
  GURL result;

  // Not allowed for invalid URLs.
  if (!is_valid_)
    return GURL();

  StdStringCanonOutput output(&result.spec_);
  result.is_valid_ = uri::ReplaceComponents(spec_.data(), static_cast<int>(spec_.length()), parsed_, replacements, NULL,
                                            &output, &result.parsed_);

  output.Complete();
  return result;
}

GURL GURL::GetOrigin() const {
  // This doesn't make sense for invalid or nonstandard URLs, so return
  // the empty URL.
  if (!is_valid_ || !IsStandard())
    return GURL();

  uri::Replacements<char> replacements;
  replacements.ClearUsername();
  replacements.ClearPassword();
  replacements.ClearPath();
  replacements.ClearQuery();
  replacements.ClearRef();

  return ReplaceComponents(replacements);
}

GURL GURL::GetAsReferrer() const {
  if (!SchemeIsValidForReferrer())
    return GURL();

  if (!has_ref() && !has_username() && !has_password())
    return GURL(*this);

  uri::Replacements<char> replacements;
  replacements.ClearRef();
  replacements.ClearUsername();
  replacements.ClearPassword();
  return ReplaceComponents(replacements);
}

GURL GURL::GetWithEmptyPath() const {
  // This doesn't make sense for invalid or nonstandard URLs, so return
  // the empty URL.
  if (!is_valid_ || !IsStandard())
    return GURL();

  // We could optimize this since we know that the URL is canonical, and we are
  // appending a canonical path, so avoiding re-parsing.
  GURL other(*this);
  if (parsed_.path.len == 0)
    return other;

  // Clear everything after the path.
  other.parsed_.query.reset();
  other.parsed_.ref.reset();

  // Set the path, since the path is longer than one, we can just set the
  // first character and resize.
  other.spec_[other.parsed_.path.begin] = '/';
  other.parsed_.path.len = 1;
  other.spec_.resize(other.parsed_.path.begin + 1);
  return other;
}

GURL GURL::GetWithoutFilename() const {
  return Resolve(".");
}

bool GURL::IsStandard() const {
  return uri::IsStandard(spec_.data(), parsed_.scheme);
}

bool GURL::SchemeIs(StringPiece lower_ascii_scheme) const {
  DCHECK(IsStringASCII(lower_ascii_scheme));
  DCHECK(ToLowerASCII(lower_ascii_scheme) == lower_ascii_scheme);

  if (parsed_.scheme.len <= 0)
    return lower_ascii_scheme.empty();
  return scheme_piece() == lower_ascii_scheme;
}

bool GURL::SchemeIsHTTPOrHTTPS() const {
  return SchemeIs(kHttpScheme) || SchemeIs(kHttpsScheme);
}

bool GURL::SchemeIsValidForReferrer() const {
  return is_valid_ && IsReferrerScheme(spec_.data(), parsed_.scheme);
}

bool GURL::SchemeIsWSOrWSS() const {
  return SchemeIs(kWsScheme) || SchemeIs(kWssScheme);
}

bool GURL::SchemeIsCryptographic() const {
  if (parsed_.scheme.len <= 0)
    return false;
  return SchemeIsCryptographic(scheme_piece());
}

bool GURL::SchemeIsCryptographic(StringPiece lower_ascii_scheme) {
  DCHECK(IsStringASCII(lower_ascii_scheme));
  DCHECK(ToLowerASCII(lower_ascii_scheme) == lower_ascii_scheme);

  return lower_ascii_scheme == kHttpsScheme || lower_ascii_scheme == kWssScheme;
}

int GURL::IntPort() const {
  if (parsed_.port.is_nonempty())
    return ParsePort(spec_.data(), parsed_.port);
  return PORT_UNSPECIFIED;
}

int GURL::EffectiveIntPort() const {
  int int_port = IntPort();
  if (int_port == PORT_UNSPECIFIED && IsStandard())
    return DefaultPortForScheme(spec_.data() + parsed_.scheme.begin, parsed_.scheme.len);
  return int_port;
}

std::string GURL::ExtractFileName() const {
  Component file_component;
  uri::ExtractFileName(spec_.data(), parsed_.path, &file_component);
  return ComponentString(file_component);
}

StringPiece GURL::PathForRequestPiece() const {
  DCHECK(parsed_.path.len > 0) << "Canonical path for requests should be non-empty";
  if (parsed_.ref.len >= 0) {
    // Clip off the reference when it exists. The reference starts after the
    // #-sign, so we have to subtract one to also remove it.
    return StringPiece(&spec_[parsed_.path.begin], parsed_.ref.begin - parsed_.path.begin - 1);
  }
  // Compute the actual path length, rather than depending on the spec's
  // terminator. If we're an inner_url, our spec continues on into our outer
  // URL's path/query/ref.
  int path_len = parsed_.path.len;
  if (parsed_.query.is_valid())
    path_len = parsed_.query.end() - parsed_.path.begin;

  return StringPiece(&spec_[parsed_.path.begin], path_len);
}

std::string GURL::PathForRequest() const {
  return PathForRequestPiece().as_string();
}

std::string GURL::HostNoBrackets() const {
  return HostNoBracketsPiece().as_string();
}

StringPiece GURL::HostNoBracketsPiece() const {
  // If host looks like an IPv6 literal, strip the square brackets.
  Component h(parsed_.host);
  if (h.len >= 2 && spec_[h.begin] == '[' && spec_[h.end() - 1] == ']') {
    h.begin++;
    h.len -= 2;
  }
  return ComponentStringPiece(h);
}

std::string GURL::GetContent() const {
  if (!is_valid_)
    return std::string();
  std::string content = ComponentString(parsed_.GetContent());
  if (parsed_.ref.len >= 0)
    content.erase(content.size() - parsed_.ref.len - 1);
  return content;
}

bool GURL::HostIsIPAddress() const {
  return is_valid_ && uri::HostIsIPAddress(host_piece());
}

const GURL& GURL::EmptyGURL() {
  static GURL empty_gurl;
  return empty_gurl;
}

bool GURL::DomainIs(StringPiece canonical_domain) const {
  if (!is_valid_)
    return false;

  return uri::DomainIs(host_piece(), canonical_domain);
}

bool GURL::EqualsIgnoringRef(const GURL& other) const {
  int ref_position = parsed_.CountCharactersBefore(Parsed::REF, true);
  int ref_position_other = other.parsed_.CountCharactersBefore(Parsed::REF, true);
  return StringPiece(spec_).substr(0, ref_position) == StringPiece(other.spec_).substr(0, ref_position_other);
}

void GURL::Swap(GURL* other) {
  spec_.swap(other->spec_);
  std::swap(is_valid_, other->is_valid_);
  std::swap(parsed_, other->parsed_);
  inner_url_.swap(other->inner_url_);
}

std::ostream& operator<<(std::ostream& out, const GURL& url) {
  return out << url.possibly_invalid_spec();
}

bool operator==(const GURL& x, const GURL& y) {
  return x.possibly_invalid_spec() == y.possibly_invalid_spec();
}

bool operator!=(const GURL& x, const GURL& y) {
  return !(x == y);
}

bool operator==(const GURL& x, const StringPiece& spec) {
  DCHECK_EQ(GURL(spec).possibly_invalid_spec(), spec);
  return x.possibly_invalid_spec() == spec;
}

bool operator==(const StringPiece& spec, const GURL& x) {
  return x == spec;
}

bool operator!=(const GURL& x, const StringPiece& spec) {
  return !(x == spec);
}

bool operator!=(const StringPiece& spec, const GURL& x) {
  return !(x == spec);
}

}  // namespace uri
}  // namespace common
