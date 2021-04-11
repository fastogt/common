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

#include <common/http/http.h>

#include <common/convert2string.h>  // for ConvertFromString
#include <common/sprintf.h>
#include <common/string_split.h>
#include <common/uri/gurl.h>
#include <common/uri/url_util.h>
#include <common/utils.h>

namespace common {

std::string ConvertToString(http::http_protocol protocol) {
  if (protocol == http::HP_1_0) {
    return HTTP_1_0_PROTOCOL_NAME;
  } else if (protocol == http::HP_1_1) {
    return HTTP_1_1_PROTOCOL_NAME;
  } else if (protocol == http::HP_2_0) {
    return HTTP_2_0_PROTOCOL_NAME;
  }

  NOTREACHED();
  return "UNKNOWN";
}

bool ConvertFromString(const std::string& from, http::http_protocol* out) {
  if (!out) {
    return false;
  }

  if (from == HTTP_1_0_PROTOCOL_NAME) {
    *out = http::HP_1_0;
    return true;
  } else if (from == HTTP_1_1_PROTOCOL_NAME) {
    *out = http::HP_1_1;
    return true;
  } else if (from == HTTP_2_0_PROTOCOL_NAME) {
    *out = http::HP_2_0;
    return true;
  }

  DNOTREACHED() << "Unknown protocol: " << from;
  return false;
}

namespace http {

namespace {

common::Error ParseHttpHeader(const std::string& line, HttpHeader* out) {
  if (line.empty() || !out) {
    return common::make_error_inval();
  }

  size_t delem = line.find_first_of(':');
  if (delem == std::string::npos) {
    return common::make_error_inval();
  }

  std::string key = line.substr(0, delem);
  std::string value = line.substr(delem + 1);
  std::string trimkey;
  TrimWhitespaceASCII(key, TRIM_ALL, &trimkey);

  std::string trimvalue;
  TrimWhitespaceASCII(value, TRIM_ALL, &trimvalue);
  *out = HttpHeader(trimkey, trimvalue);
  return common::Error();
}

common::Error ParseHttpHeaders(const std::string& headers_data, HttpResponse* out) {
  if (headers_data.empty() || !out) {
    return common::make_error_inval();
  }

  http_protocol lprotocol = HP_1_0;
  headers_t lheaders;
  uint16_t lstatus = 0;

  std::vector<std::string> result = common::SplitString(headers_data, "\r\n", TRIM_WHITESPACE, SPLIT_WANT_ALL);
  if (result.empty()) {
    return common::make_error_inval();
  }

  std::string first_line = result[0];
  size_t spaceP = first_line.find_first_of(" ");
  if (spaceP != std::string::npos) {
    std::string status_str = first_line.substr(spaceP + 1);
    std::string protocol_str = first_line.substr(0, spaceP);
    size_t http_sep = protocol_str.find_first_of("/");
    if (http_sep == std::string::npos) {
      return make_error_inval();
    }

    if (!ConvertFromString(protocol_str, &lprotocol)) {
      DNOTREACHED() << "Unknown protocol: " << protocol_str;
    }

    size_t status_sep = status_str.find_first_of(" ");
    if (status_sep == std::string::npos) {
      return make_error_inval();
    }

    std::string status_num = status_str.substr(0, status_sep);
    if (!ConvertFromString(status_num, &lstatus)) {
      return make_error_inval();
    }
  } else {
    return make_error_inval();
  }

  for (size_t i = 1; i < result.size(); ++i) {
    HttpHeader lhead;
    common::Error perr = ParseHttpHeader(result[i], &lhead);
    if (!perr) {
      lheaders.push_back(lhead);
    }
  }

  *out = HttpResponse(lprotocol, static_cast<http_status>(lstatus), lheaders, char_buffer_t());
  return common::Error();
}

}  // namespace

const char* MimeTypes::GetType(const char* extension) {
  const char* dot = strrchr(extension, '.');
  if (dot) {
    if (dot != extension) {
      extension = dot;
    }

    extension++;
  }

  int min = 0;
  int max = (sizeof(types) / sizeof(*types) - 1);

  while (min <= max) {
    int i = (int)(((max + min) / 2) + 0.5);
    int order = strcmpi(extension, types[i].fileExtension);

    if (order == 0) {
      return types[i].mimeType;
    } else if (order > 0) {
      min = i + 1;
    } else {
      max = i - 1;
    }
  }

  return NULL;
}

const char* MimeTypes::GetExtension(const char* type) {
  int length = (sizeof(types) / sizeof(*types));

  for (int i = 0; i < length; i++) {
    if (strcmpi(type, types[i].mimeType) == 0) {
      return types[i].fileExtension;
    }
  }

  return NULL;
}

// https://www.geeksforgeeks.org/write-your-own-strcmp-which-ignores-cases/
int MimeTypes::strcmpi(const char* s1, const char* s2) {
  int i;

  for (i = 0; s1[i] && s2[i]; ++i) {
    if (s1[i] == s2[i] || (s1[i] ^ 32) == s2[i]) {
      continue;
    } else {
      break;
    }
  }

  if (s1[i] == s2[i]) {
    return 0;
  }

  if ((s1[i] | 32) < (s2[i] | 32)) {
    return -1;
  }

  return 1;
}

MimeTypes::entry MimeTypes::types[347] = {
    {"*3gpp", "audio/3gpp"},
    {"*jpm", "video/jpm"},
    {"*mp3", "audio/mp3"},
    {"*rtf", "text/rtf"},
    {"*wav", "audio/wave"},
    {"*xml", "text/xml"},
    {"3g2", "video/3gpp2"},
    {"3gp", "video/3gpp"},
    {"3gpp", "video/3gpp"},
    {"ac", "application/pkix-attr-cert"},
    {"adp", "audio/adpcm"},
    {"ai", "application/postscript"},
    {"apng", "image/apng"},
    {"appcache", "text/cache-manifest"},
    {"asc", "application/pgp-signature"},
    {"atom", "application/atom+xml"},
    {"atomcat", "application/atomcat+xml"},
    {"atomsvc", "application/atomsvc+xml"},
    {"au", "audio/basic"},
    {"aw", "application/applixware"},
    {"bdoc", "application/bdoc"},
    {"bin", "application/octet-stream"},
    {"bmp", "image/bmp"},
    {"bpk", "application/octet-stream"},
    {"buffer", "application/octet-stream"},
    {"ccxml", "application/ccxml+xml"},
    {"cdmia", "application/cdmi-capability"},
    {"cdmic", "application/cdmi-container"},
    {"cdmid", "application/cdmi-domain"},
    {"cdmio", "application/cdmi-object"},
    {"cdmiq", "application/cdmi-queue"},
    {"cer", "application/pkix-cert"},
    {"cgm", "image/cgm"},
    {"class", "application/java-vm"},
    {"coffee", "text/coffeescript"},
    {"conf", "text/plain"},
    {"cpt", "application/mac-compactpro"},
    {"crl", "application/pkix-crl"},
    {"css", "text/css"},
    {"csv", "text/csv"},
    {"cu", "application/cu-seeme"},
    {"davmount", "application/davmount+xml"},
    {"dbk", "application/docbook+xml"},
    {"deb", "application/octet-stream"},
    {"def", "text/plain"},
    {"deploy", "application/octet-stream"},
    {"disposition-notification", "message/disposition-notification"},
    {"dist", "application/octet-stream"},
    {"distz", "application/octet-stream"},
    {"dll", "application/octet-stream"},
    {"dmg", "application/octet-stream"},
    {"dms", "application/octet-stream"},
    {"doc", "application/msword"},
    {"dot", "application/msword"},
    {"drle", "image/dicom-rle"},
    {"dssc", "application/dssc+der"},
    {"dtd", "application/xml-dtd"},
    {"dump", "application/octet-stream"},
    {"ear", "application/java-archive"},
    {"ecma", "application/ecmascript"},
    {"elc", "application/octet-stream"},
    {"emf", "image/emf"},
    {"eml", "message/rfc822"},
    {"emma", "application/emma+xml"},
    {"eps", "application/postscript"},
    {"epub", "application/epub+zip"},
    {"es", "application/ecmascript"},
    {"exe", "application/octet-stream"},
    {"exi", "application/exi"},
    {"exr", "image/aces"},
    {"ez", "application/andrew-inset"},
    {"fits", "image/fits"},
    {"g3", "image/g3fax"},
    {"gbr", "application/rpki-ghostbusters"},
    {"geojson", "application/geo+json"},
    {"gif", "image/gif"},
    {"glb", "model/gltf-binary"},
    {"gltf", "model/gltf+json"},
    {"gml", "application/gml+xml"},
    {"gpx", "application/gpx+xml"},
    {"gram", "application/srgs"},
    {"grxml", "application/srgs+xml"},
    {"gxf", "application/gxf"},
    {"gz", "application/gzip"},
    {"gz", "application/x-gzip"},
    {"h261", "video/h261"},
    {"h263", "video/h263"},
    {"h264", "video/h264"},
    {"heic", "image/heic"},
    {"heics", "image/heic-sequence"},
    {"heif", "image/heif"},
    {"heifs", "image/heif-sequence"},
    {"hjson", "application/hjson"},
    {"hlp", "application/winhlp"},
    {"hqx", "application/mac-binhex40"},
    {"htm", "text/html"},
    {"html", "text/html"},
    {"ics", "text/calendar"},
    {"ief", "image/ief"},
    {"ifb", "text/calendar"},
    {"iges", "model/iges"},
    {"igs", "model/iges"},
    {"img", "application/octet-stream"},
    {"in", "text/plain"},
    {"ini", "text/plain"},
    {"ink", "application/inkml+xml"},
    {"inkml", "application/inkml+xml"},
    {"ipfix", "application/ipfix"},
    {"iso", "application/octet-stream"},
    {"jade", "text/jade"},
    {"jar", "application/java-archive"},
    {"jls", "image/jls"},
    {"jp2", "image/jp2"},
    {"jpe", "image/jpeg"},
    {"jpeg", "image/jpeg"},
    {"jpf", "image/jpx"},
    {"jpg", "image/jpeg"},
    {"jpg2", "image/jp2"},
    {"jpgm", "video/jpm"},
    {"jpgv", "video/jpeg"},
    {"jpm", "image/jpm"},
    {"jpx", "image/jpx"},
    {"js", "application/javascript"},
    {"json", "application/json"},
    {"json5", "application/json5"},
    {"jsonld", "application/ld+json"},
    {"jsonml", "application/jsonml+json"},
    {"jsx", "text/jsx"},
    {"kar", "audio/midi"},
    {"ktx", "image/ktx"},
    {"less", "text/less"},
    {"list", "text/plain"},
    {"litcoffee", "text/coffeescript"},
    {"log", "text/plain"},
    {"lostxml", "application/lost+xml"},
    {"lrf", "application/octet-stream"},
    {"m1v", "video/mpeg"},
    {"m21", "application/mp21"},
    {"m2a", "audio/mpeg"},
    {"m2v", "video/mpeg"},
    {"m3a", "audio/mpeg"},
    {"m4a", "audio/mp4"},
    {"m4p", "application/mp4"},
    {"ma", "application/mathematica"},
    {"mads", "application/mads+xml"},
    {"man", "text/troff"},
    {"manifest", "text/cache-manifest"},
    {"map", "application/json"},
    {"mar", "application/octet-stream"},
    {"markdown", "text/markdown"},
    {"mathml", "application/mathml+xml"},
    {"mb", "application/mathematica"},
    {"mbox", "application/mbox"},
    {"md", "text/markdown"},
    {"me", "text/troff"},
    {"mesh", "model/mesh"},
    {"meta4", "application/metalink4+xml"},
    {"metalink", "application/metalink+xml"},
    {"mets", "application/mets+xml"},
    {"mft", "application/rpki-manifest"},
    {"mid", "audio/midi"},
    {"midi", "audio/midi"},
    {"mime", "message/rfc822"},
    {"mj2", "video/mj2"},
    {"mjp2", "video/mj2"},
    {"mjs", "application/javascript"},
    {"mml", "text/mathml"},
    {"mods", "application/mods+xml"},
    {"mov", "video/quicktime"},
    {"mp2", "audio/mpeg"},
    {"mp21", "application/mp21"},
    {"mp2a", "audio/mpeg"},
    {"mp3", "audio/mpeg"},
    {"mp4", "video/mp4"},
    {"mp4a", "audio/mp4"},
    {"mp4s", "application/mp4"},
    {"mp4v", "video/mp4"},
    {"mpd", "application/dash+xml"},
    {"mpe", "video/mpeg"},
    {"mpeg", "video/mpeg"},
    {"mpg", "video/mpeg"},
    {"mpg4", "video/mp4"},
    {"mpga", "audio/mpeg"},
    {"mrc", "application/marc"},
    {"mrcx", "application/marcxml+xml"},
    {"ms", "text/troff"},
    {"mscml", "application/mediaservercontrol+xml"},
    {"msh", "model/mesh"},
    {"msi", "application/octet-stream"},
    {"msm", "application/octet-stream"},
    {"msp", "application/octet-stream"},
    {"mxf", "application/mxf"},
    {"mxml", "application/xv+xml"},
    {"n3", "text/n3"},
    {"nb", "application/mathematica"},
    {"oda", "application/oda"},
    {"oga", "audio/ogg"},
    {"ogg", "audio/ogg"},
    {"ogv", "video/ogg"},
    {"ogx", "application/ogg"},
    {"omdoc", "application/omdoc+xml"},
    {"onepkg", "application/onenote"},
    {"onetmp", "application/onenote"},
    {"onetoc", "application/onenote"},
    {"onetoc2", "application/onenote"},
    {"opf", "application/oebps-package+xml"},
    {"otf", "font/otf"},
    {"owl", "application/rdf+xml"},
    {"oxps", "application/oxps"},
    {"p10", "application/pkcs10"},
    {"p7c", "application/pkcs7-mime"},
    {"p7m", "application/pkcs7-mime"},
    {"p7s", "application/pkcs7-signature"},
    {"p8", "application/pkcs8"},
    {"pdf", "application/pdf"},
    {"pfr", "application/font-tdpfr"},
    {"pgp", "application/pgp-encrypted"},
    {"pkg", "application/octet-stream"},
    {"pki", "application/pkixcmp"},
    {"pkipath", "application/pkix-pkipath"},
    {"pls", "application/pls+xml"},
    {"png", "image/png"},
    {"prf", "application/pics-rules"},
    {"ps", "application/postscript"},
    {"pskcxml", "application/pskc+xml"},
    {"qt", "video/quicktime"},
    {"raml", "application/raml+yaml"},
    {"rdf", "application/rdf+xml"},
    {"rif", "application/reginfo+xml"},
    {"rl", "application/resource-lists+xml"},
    {"rld", "application/resource-lists-diff+xml"},
    {"rmi", "audio/midi"},
    {"rnc", "application/relax-ng-compact-syntax"},
    // {"rng", "application/xml"},
    {"roa", "application/rpki-roa"},
    {"roff", "text/troff"},
    {"rq", "application/sparql-query"},
    {"rs", "application/rls-services+xml"},
    {"rsd", "application/rsd+xml"},
    {"rss", "application/rss+xml"},
    {"rtf", "application/rtf"},
    {"rtx", "text/richtext"},
    {"s3m", "audio/s3m"},
    {"sbml", "application/sbml+xml"},
    {"scq", "application/scvp-cv-request"},
    {"scs", "application/scvp-cv-response"},
    {"sdp", "application/sdp"},
    {"ser", "application/java-serialized-object"},
    {"setpay", "application/set-payment-initiation"},
    {"setreg", "application/set-registration-initiation"},
    {"sgi", "image/sgi"},
    {"sgm", "text/sgml"},
    {"sgml", "text/sgml"},
    {"shex", "text/shex"},
    {"shf", "application/shf+xml"},
    {"shtml", "text/html"},
    {"sig", "application/pgp-signature"},
    {"sil", "audio/silk"},
    {"silo", "model/mesh"},
    {"slim", "text/slim"},
    {"slm", "text/slim"},
    {"smi", "application/smil+xml"},
    {"smil", "application/smil+xml"},
    {"snd", "audio/basic"},
    {"so", "application/octet-stream"},
    {"spp", "application/scvp-vp-response"},
    {"spq", "application/scvp-vp-request"},
    {"spx", "audio/ogg"},
    {"sru", "application/sru+xml"},
    {"srx", "application/sparql-results+xml"},
    {"ssdl", "application/ssdl+xml"},
    {"ssml", "application/ssml+xml"},
    {"stk", "application/hyperstudio"},
    {"styl", "text/stylus"},
    {"stylus", "text/stylus"},
    {"svg", "image/svg+xml"},
    {"svgz", "image/svg+xml"},
    {"t", "text/troff"},
    {"t38", "image/t38"},
    {"tei", "application/tei+xml"},
    {"teicorpus", "application/tei+xml"},
    {"text", "text/plain"},
    {"tfi", "application/thraud+xml"},
    {"tfx", "image/tiff-fx"},
    {"tif", "image/tiff"},
    {"tiff", "image/tiff"},
    {"tr", "text/troff"},
    {"ts", "video/mp2t"},
    {"tsd", "application/timestamped-data"},
    {"tsv", "text/tab-separated-values"},
    {"ttc", "font/collection"},
    {"ttf", "font/ttf"},
    {"ttl", "text/turtle"},
    {"txt", "text/plain"},
    {"u8dsn", "message/global-delivery-status"},
    {"u8hdr", "message/global-headers"},
    {"u8mdn", "message/global-disposition-notification"},
    {"u8msg", "message/global"},
    {"uri", "text/uri-list"},
    {"uris", "text/uri-list"},
    {"urls", "text/uri-list"},
    {"vcard", "text/vcard"},
    {"vrml", "model/vrml"},
    {"vtt", "text/vtt"},
    {"vxml", "application/voicexml+xml"},
    {"war", "application/java-archive"},
    {"wasm", "application/wasm"},
    {"wav", "audio/wav"},
    {"weba", "audio/webm"},
    {"webm", "video/webm"},
    {"webmanifest", "application/manifest+json"},
    {"webp", "image/webp"},
    {"wgt", "application/widget"},
    {"wmf", "image/wmf"},
    {"woff", "font/woff"},
    {"woff2", "font/woff2"},
    {"wrl", "model/vrml"},
    {"wsdl", "application/wsdl+xml"},
    {"wspolicy", "application/wspolicy+xml"},
    {"x3d", "model/x3d+xml"},
    {"x3db", "model/x3d+binary"},
    {"x3dbz", "model/x3d+binary"},
    {"x3dv", "model/x3d+vrml"},
    {"x3dvz", "model/x3d+vrml"},
    {"x3dz", "model/x3d+xml"},
    {"xaml", "application/xaml+xml"},
    {"xdf", "application/xcap-diff+xml"},
    {"xdssc", "application/dssc+xml"},
    {"xenc", "application/xenc+xml"},
    {"xer", "application/patch-ops-error+xml"},
    {"xht", "application/xhtml+xml"},
    {"xhtml", "application/xhtml+xml"},
    {"xhvml", "application/xv+xml"},
    {"xm", "audio/xm"},
    {"xml", "application/xml"},
    {"xop", "application/xop+xml"},
    {"xpl", "application/xproc+xml"},
    {"xsd", "application/xml"},
    {"xsl", "application/xml"},
    {"xslt", "application/xslt+xml"},
    {"xspf", "application/xspf+xml"},
    {"xvm", "application/xv+xml"},
    {"xvml", "application/xv+xml"},
    {"yaml", "text/yaml"},
    {"yang", "application/yang"},
    {"yin", "application/yin+xml"},
    {"yml", "text/yaml"},
    {"zip", "application/zip"},
};

HttpHeader::HttpHeader() : key(), value() {}

HttpHeader::HttpHeader(const std::string& key, const std::string& value) : key(key), value(value) {}

bool HttpHeader::IsValid() const {
  return !key.empty();
}

std::string HttpHeader::as_string() const {
  return MemSPrintf("%s: %s", key, value);
}

HttpRequest::HttpRequest() : method_(), relative_url_(), base_url_(), protocol_(), headers_(), body_() {}

HttpRequest::HttpRequest(http_method method,
                         const path_t& relative_url,
                         http_protocol protocol,
                         const headers_t& headers,
                         const body_t& body)
    : method_(method), relative_url_(relative_url), base_url_(), protocol_(protocol), headers_(headers), body_(body) {}

http::http_protocol HttpRequest::GetProtocol() const {
  return protocol_;
}

headers_t HttpRequest::GetHeaders() const {
  return headers_;
}

bool HttpRequest::IsValid() const {
  return !relative_url_.empty();
}

HttpRequest::path_t HttpRequest::GetRelativeUrl() const {
  return relative_url_;
}

void HttpRequest::SetRelativeUrl(const path_t& path) {
  relative_url_ = path;
}

uri::GURL HttpRequest::GetURL() const {
  if (base_url_.is_valid()) {
    return base_url_.Resolve(relative_url_);
  }

  std::string host = "localhost";
  header_t server;
  if (FindHeaderByKey("Host", false, &server)) {
    host = server.value;
  }
  return uri::GURL(MemSPrintf("http://%s%s", host, relative_url_));
}

http::http_method HttpRequest::GetMethod() const {
  return method_;
}

HttpRequest::body_t HttpRequest::GetBody() const {
  return body_;
}

bool HttpRequest::FindHeaderByKeyAndChange(const std::string& key, bool case_sensitive, header_t new_value) {
  for (size_t i = 0; i < headers_.size(); ++i) {
    std::string cur_key = headers_[i].key;
    if (EqualsASCII(cur_key, key, case_sensitive)) {
      headers_[i] = new_value;
      return true;
    }
  }

  return false;
}

void HttpRequest::RemoveHeaderByKey(const std::string& key, bool case_sensitive) {
  for (size_t i = 0; i < headers_.size(); ++i) {
    std::string curKey = headers_[i].key;
    if (EqualsASCII(curKey, key, case_sensitive)) {
      headers_.erase(headers_.begin(), headers_.begin() + i);
      return;
    }
  }
}

bool HttpRequest::FindHeaderByKey(const std::string& key, bool case_sensitive, header_t* hdr) const {
  if (!hdr) {
    return false;
  }

  for (size_t i = 0; i < headers_.size(); ++i) {
    std::string curKey = headers_[i].key;
    if (EqualsASCII(curKey, key, case_sensitive)) {
      *hdr = headers_[i];
      return true;
    }
  }

  return false;
}

bool HttpRequest::FindHeaderByValue(const std::string& value, bool case_sensitive, header_t* hdr) const {
  if (!hdr) {
    return false;
  }

  for (size_t i = 0; i < headers_.size(); ++i) {
    std::string curKey = headers_[i].value;
    if (EqualsASCII(curKey, value, case_sensitive)) {
      *hdr = headers_[i];
      return true;
    }
  }

  return false;
}

Optional<HttpRequest> MakeHeadRequest(const std::string& path, http_protocol protocol, const headers_t& headers) {
  http::HttpRequest req(http::HM_HEAD, path, protocol, headers, http::HttpRequest::body_t());
  if (!req.IsValid()) {
    return Optional<HttpRequest>();
  }
  return req;
}

Optional<HttpRequest> MakeGetRequest(const std::string& path,
                                     http_protocol protocol,
                                     const headers_t& headers,
                                     const HttpRequest::body_t& body) {
  http::HttpRequest req(http::HM_GET, path, protocol, headers, body);
  if (!req.IsValid()) {
    return Optional<HttpRequest>();
  }
  return req;
}

Optional<HttpRequest> MakePostRequest(const std::string& path,
                                      http_protocol protocol,
                                      const headers_t& headers,
                                      const http::HttpRequest::body_t& body) {
  http::HttpRequest req(http::HM_POST, path, protocol, headers, body);
  if (!req.IsValid()) {
    return Optional<HttpRequest>();
  }
  return req;
}

std::pair<http_status, Error> parse_http_request(const std::string& request, HttpRequest* req_out) {
  if (request.empty() || !req_out) {
    return std::make_pair(HS_FORBIDDEN, make_error_inval());
  }

  typedef std::string::size_type string_size_t;

  const string_size_t len = request.size();

  http_method lmethod = HM_GET;
  std::string lpath;
  http_protocol lprotocol = HP_1_0;
  headers_t lheaders;

  string_size_t pos = 0;
  string_size_t start = 0;
  uint8_t line_count = 0;
  while ((pos = request.find("\r\n", start)) != std::string::npos) {
    std::string line = request.substr(start, pos - start);
    if (line_count == 0) {  // GET //POST //HEAD
      string_size_t space = line.find_first_of(' ');
      if (space != std::string::npos) {
        std::string method = line.substr(0, space);
        if (method == "GET" || method == "HEAD" || method == "POST") {
          if (method == "GET") {
            lmethod = HM_GET;
          } else if (method == "HEAD") {
            lmethod = HM_HEAD;
          } else {
            lmethod = HM_POST;
          }

          std::string protcolAndPath = line.substr(space + 1);
          size_t space_protocol = protcolAndPath.find_first_of("HTTP");
          if (space_protocol != std::string::npos) {
            std::string path = protcolAndPath.substr(0, space_protocol - 1);
            if (protcolAndPath[0] != '/') {  // must start with /
              return std::make_pair(HS_BAD_REQUEST, make_error("Bad filename."));
            }

            std::string protocol_str = protcolAndPath.substr(space_protocol);
            if (!ConvertFromString(protocol_str, &lprotocol)) {
              DNOTREACHED() << "Unknown protocol: " << protocol_str;
            }
            common::uri::GURL url(path);
            if (url.is_valid()) {
              lpath = url.PathForRequest();
            } else if (path[0] == '/') {
              lpath = path;
            } else {
              lpath = "/" + path;
            }
          } else {
            return std::make_pair(HS_FORBIDDEN, make_error("Not allowed."));
          }
        } else {
          return std::make_pair(HS_NOT_IMPLEMENTED, make_error("That method is not implemented."));
        }
      } else {
        return std::make_pair(HS_NOT_IMPLEMENTED, make_error("That method is not implemented."));
      }
    } else {
      HttpHeader lhead;
      common::Error perr = ParseHttpHeader(line, &lhead);
      if (!perr) {
        lheaders.push_back(lhead);
      }
    }
    line_count++;
    start = pos + 2;
  }

  uri::RawCanonOutputT<char16> unescaped;
  if (len != start && line_count != 0) {
    const char* request_str = request.c_str() + start;
    uri::DecodeURLEscapeSequences(request_str, strlen(request_str), uri::DecodeURLMode::kUTF8OrIsomorphic, &unescaped);
  }

  if (line_count == 0) {
    return std::make_pair(HS_BAD_REQUEST, make_error("Not found CRLF"));
  }

  *req_out =
      HttpRequest(lmethod, lpath, lprotocol, lheaders, MAKE_CHAR_BUFFER_SIZE(unescaped.data(), unescaped.length()));
  return std::make_pair(HS_OK, Error());
}

HttpResponse::HttpResponse() : protocol_(), status_(), headers_(), body_() {}

HttpResponse::HttpResponse(http_protocol protocol,
                           http_status status,
                           const headers_t& headers,
                           const char_buffer_t& body)
    : protocol_(protocol), status_(status), headers_(headers), body_(body) {}

bool HttpResponse::FindHeaderByKey(const std::string& key, bool case_sensitive, header_t* hdr) const {
  if (!hdr) {
    return false;
  }

  for (size_t i = 0; i < headers_.size(); ++i) {
    std::string curKey = headers_[i].key;
    if (EqualsASCII(curKey, key, case_sensitive)) {
      *hdr = headers_[i];
      return true;
    }
  }

  return false;
}

void HttpResponse::SetBody(const body_t& body) {
  body_ = body;
}

bool HttpResponse::IsEmptyBody() const {
  return body_.empty();
}

HttpResponse::body_t HttpResponse::GetBody() const {
  return body_;
}

http_status HttpResponse::GetStatus() const {
  return status_;
}

http_protocol HttpResponse::GetProtocol() const {
  return protocol_;
}

headers_t HttpResponse::GetHeaders() const {
  return headers_;
}

Error parse_http_response(const std::string& response, HttpResponse* res_out, size_t* not_parsed) {
  if (response.empty() || !res_out || !not_parsed) {
    return make_error_inval();
  }

  typedef std::string::size_type string_size_t;
  const string_size_t headers_line_pos = response.find("\r\n\r\n");
  if (headers_line_pos == std::string::npos) {
    return make_error("Not found CRLFCRLF");
  }

  const std::string header_data = response.substr(0, headers_line_pos);
  const std::string body_data = response.substr(headers_line_pos + 4);

  HttpResponse lres;
  common::Error err = ParseHttpHeaders(header_data, &lres);
  if (err) {
    return err;
  }

  size_t lnot_parsed = body_data.size();
  http::header_t cont;
  if (lres.FindHeaderByKey("Content-Length", false, &cont)) {
    size_t body_len = 0;
    if (ConvertFromString(cont.value, &body_len)) {
      if (lnot_parsed == body_len) {  // full
        lres.SetBody(MAKE_CHAR_BUFFER_SIZE(body_data.data(), body_len));
        lnot_parsed = 0;
      } else {
        *not_parsed = lnot_parsed;
      }
    }
  }

  *not_parsed = lnot_parsed;
  *res_out = lres;
  return Error();
}

}  // namespace http

std::string ConvertToString(http::http_method method) {
  if (method == http::HM_GET) {
    return "GET";
  } else if (method == http::HM_HEAD) {
    return "HEAD";
  } else {
    return "POST";
  }
}

bool ConvertFromString(const std::string& from, http::http_method* out) {
  if (!out) {
    return false;
  }

  if (from == "GET") {
    *out = http::HM_GET;
    return true;
  } else if (from == "HEAD") {
    *out = http::HM_HEAD;
    return true;
  } else if (from == "POST") {
    *out = http::HM_POST;
    return true;
  }

  return false;
}

std::string ConvertToString(http::http_status status) {
  switch (status) {
    case http::HS_CONTINUE:
      return "Continue";
    case http::HS_SWITCH_PROTOCOL:
      return "Switching Protocols";
    case http::HS_OK:
      return "OK";
    case http::HS_CREATED:
      return "Created";
    case http::HS_ACCEPTED:
      return "Accepted";
    case http::HS_NON_AUTH_INFO:
      return "Non-Authoritative Information";
    case http::HS_NO_CONTENT:
      return "No Content";
    case http::HS_RESET_CONTENT:
      return "Reset Content";
    case http::HS_PARTIAL_CONTENT:
      return "Partial Content";
    case http::HS_MULTIPLE_CHOICES:
      return "Multiple Choices";
    case http::HS_MOVED_PERMANENTLY:
      return "Moved Permanently";
    case http::HS_FOUND:
      return "Found";
    case http::HS_SEE_OTHER:
      return "See Other";
    case http::HS_NOT_MODIFIED:
      return "Not Modified";
    case http::HS_USE_PROXY:
      return "Use Proxy";
    case http::HS_TEMPORARY_REDIRECT:
      return "Temporary Redirect";
    case http::HS_PERMANENT_REDIRECT:
      return "Permanent Redirect";
    case http::HS_BAD_REQUEST:
      return "Bad Request";
    case http::HS_UNAUTHORIZED:
      return "Unauthorized";
    case http::HS_PYMENT_REQUIRED:
      return "Payment Required";
    case http::HS_FORBIDDEN:
      return "Forbidden";
    case http::HS_NOT_FOUND:
      return "Not Found";
    case http::HS_NOT_ALLOWED:
      return "Method Not Allowed";
    case http::HS_NOT_ACCEPTABLE:
      return "Not Acceptable";
    case http::HS_PROXY_AUTH_REQUIRED:
      return "Proxy Authentication Required";
    case http::HS_REQUEST_TIMEOUT:
      return "Request Timeout";
    case http::HS_CONFLICT:
      return "Conflict";
    case http::HS_GONE:
      return "Gone";
    case http::HS_LENGTH_REQUIRED:
      return "Length Required";
    case http::HS_PRECONDITION_FAILED:
      return "Precondition Failed";
    case http::HS_PAYLOAD_TOO_LARGE:
      return "Payload Too Large";
    case http::HS_URI_TOO_LONG:
      return "URI Too Long";
    case http::HS_UNSUPPORTED_MEDIA_TYPE:
      return "Unsupported Media Type";
    case http::HS_REQUESTED_RANGE_NOT_SATISFIABLE:
      return "Requested Range Not Satisfiable";
    case http::HS_EXPECTATION_FAILED:
      return "Expectation Failed";
    case http::HS_MISDIRECTED_REQUEST:
      return "Misdirected Request";
    case http::HS_UPGRADE_REQUIRED:
      return "Upgrade Required";
    case http::HS_PRECONDITION_REQUIRED:
      return "Precondition Required";
    case http::HS_TOO_MANY_REQUESTS:
      return "Too Many Requests";
    case http::HS_REQUEST_HEADER_FIELDS_TOO_LARGE:
      return "Request Header Fields Too Large";
    case http::HS_INTERNAL_ERROR:
      return "Internal Server Error";
    case http::HS_NOT_IMPLEMENTED:
      return "Not Implemented";
    case http::HS_BAD_GATEWAY:
      return "Bad Gateway";
    case http::HS_SERVICE_UNAVAILIBLE:
      return "Service Unavailable";
    case http::HS_GATEWAY_TIMEOUT:
      return "Gateway Timeout";
    case http::HS_HTTP_VERSION_NOT_SUPPORTED:
      return "HTTP Version Not Supported";
    case http::HS_NETWORK_AUTH_REQUIRED:
      return "Network Authentication Required";
    default:
      NOTREACHED();
      return "Unknown";
  }
}

std::string ConvertToString(http::HttpHeader header) {
  if (!header.IsValid()) {
    return std::string();
  }

  return header.key + ":" + header.value;
}

std::string ConvertToString(http::HttpRequest request) {
  auto upath = request.GetRelativeUrl();
  http::http_method method = request.GetMethod();

  std::string headerout = MemSPrintf("%s %s %s\r\n", ConvertToString(method), upath,
                                     ConvertToString(request.GetProtocol()));  // "GET /hello.htm HTTP/1.1\r\n"
  http::headers_t headers = request.GetHeaders();
  for (size_t i = 0; i < headers.size(); ++i) {
    http::header_t header = headers[i];
    std::string headerStr = ConvertToString(header);
    if (!headerStr.empty()) {
      headerout += headerStr + "\r\n";
    }
  }
  headerout += "\r\n";

  return headerout;
}

buffer_t ConvertToBytes(http::HttpRequest request) {
  std::string request_str = ConvertToString(request);
  buffer_t res;
  if (!ConvertFromString(request_str, &res)) {
    return buffer_t();
  }
  return res;
}

}  // namespace common
