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

#include <common/uri/url_canon.h>

namespace common {
namespace uri {

namespace {

template <typename CHAR, typename UCHAR>
bool DoCanonicalizeTcpURL(const URLComponentSource<CHAR>& source,
                          const Parsed& parsed,
                          CharsetConverter* query_converter,
                          CanonOutput* output,
                          Parsed* new_parsed) {
  UNUSED(query_converter);

  // Things we don't set in udp: URLs.
  new_parsed->username = Component();
  new_parsed->password = Component();
  new_parsed->query = Component();

  // Scheme (known, so we don't bother running it through the more
  // complicated scheme canonicalizer).
  new_parsed->scheme.begin = output->length();
  output->Append("tcp://", 6);
  new_parsed->scheme.len = 3;

  bool success = CanonicalizeHost(source.host, parsed.host, output, &new_parsed->host);
  int default_port = DefaultPortForScheme(&output->data()[new_parsed->scheme.begin], new_parsed->scheme.len);
  success &= CanonicalizePort(source.port, parsed.port, default_port, output, &new_parsed->port);
  return success;
}

}  // namespace

bool CanonicalizeTcpURL(const char* spec,
                        int spec_len,
                        const Parsed& parsed,
                        CharsetConverter* query_converter,
                        CanonOutput* output,
                        Parsed* new_parsed) {
  UNUSED(spec_len);
  return DoCanonicalizeTcpURL<char, unsigned char>(URLComponentSource<char>(spec), parsed, query_converter, output,
                                                   new_parsed);
}

bool CanonicalizeTcpURL(const char16* spec,
                        int spec_len,
                        const Parsed& parsed,
                        CharsetConverter* query_converter,
                        CanonOutput* output,
                        Parsed* new_parsed) {
  UNUSED(spec_len);
  return DoCanonicalizeTcpURL<char16, char16>(URLComponentSource<char16>(spec), parsed, query_converter, output,
                                              new_parsed);
}

}  // namespace uri
}  // namespace common
