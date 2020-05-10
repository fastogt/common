#include <common/uri/url_util.h>

#include <common/string_util.h>
#include <common/uri/url_canon.h>
#include <common/uri/url_constants.h>

#include <vector>

namespace common {
namespace uri {

namespace {
struct SchemeWithType {
  std::string scheme;
  SchemeType type;
};

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
      {kFtpScheme, SCHEME_WITH_HOST_PORT_AND_USER_INFORMATION},
      {kWssScheme, SCHEME_WITH_HOST_PORT_AND_USER_INFORMATION},  // WebSocket secure.
      {kWsScheme, SCHEME_WITH_HOST_PORT_AND_USER_INFORMATION},   // WebSocket.
      {kFileSystemScheme, SCHEME_WITHOUT_AUTHORITY},
      {kQuicTransportScheme, SCHEME_WITH_HOST_AND_PORT},
  };

  // Schemes that are allowed for referrers.
  std::vector<SchemeWithType> referrer_schemes = {
      {kHttpsScheme, SCHEME_WITH_HOST_PORT_AND_USER_INFORMATION},
      {kHttpScheme, SCHEME_WITH_HOST_PORT_AND_USER_INFORMATION},
  };

  // Schemes that do not trigger mixed content warning.
  std::vector<std::string> secure_schemes = {
      kHttpsScheme, kAboutScheme, kDataScheme, kQuicTransportScheme, kWssScheme,
  };

  // Schemes that normal pages cannot link to or access (i.e., with the same
  // security rules as those applied to "file" URLs).
  std::vector<std::string> local_schemes = {
      kFileScheme,
  };

  // Schemes that cause pages loaded with them to not have access to pages
  // loaded with any other URL scheme.
  std::vector<std::string> no_access_schemes = {
      kAboutScheme,
      kJavaScriptScheme,
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

  // Schemes that are strictly empty documents, allowing them to commit
  // synchronously.
  std::vector<std::string> empty_document_schemes = {
      kAboutScheme,
  };

  bool allow_non_standard_schemes = false;
};

const SchemeRegistry& GetSchemeRegistry() {
  static SchemeRegistry registry;
  return registry;
}

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
    auto str = typename CharToStringPiece<CHAR>::Piece(&spec[scheme.begin], scheme.len);
    if (LowerCaseEqualsASCII(str, scheme_with_type.scheme)) {
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

}  // namespace

bool IsStandard(const char* spec, const Component& scheme) {
  SchemeType unused_scheme_type;
  return DoIsStandard(spec, scheme, &unused_scheme_type);
}

bool IsStandard(const char16* spec, const Component& scheme) {
  SchemeType unused_scheme_type;
  return DoIsStandard(spec, scheme, &unused_scheme_type);
}

}  // namespace uri
}  // namespace common
