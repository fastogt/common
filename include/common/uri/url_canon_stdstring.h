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

#pragma once

#include <string>

#include <common/string_piece.h>
#include <common/uri/url_canon.h>

namespace common {
namespace uri {

// Write into a std::string given in the constructor. This object does not own
// the string itself, and the user must ensure that the string stays alive
// throughout the lifetime of this object.
//
// The given string will be appended to; any existing data in the string will
// be preserved.
//
// Note that when canonicalization is complete, the string will likely have
// unused space at the end because we make the string very big to start out
// with (by |initial_size|). This ends up being important because resize
// operations are slow, and because the base class needs to write directly
// into the buffer.
//
// Therefore, the user should call Complete() before using the string that
// this class wrote into.
class StdStringCanonOutput : public CanonOutput {
 public:
  StdStringCanonOutput(std::string* str);
  ~StdStringCanonOutput() override;

  // Must be called after writing has completed but before the string is used.
  void Complete();

  void Resize(int sz) override;

 protected:
  std::string* str_;
  DISALLOW_COPY_AND_ASSIGN(StdStringCanonOutput);
};

// An extension of the Replacements class that allows the setters to use
// StringPieces (implicitly allowing strings or char*s).
//
// The contents of the StringPieces are not copied and must remain valid until
// the StringPieceReplacements object goes out of scope.
template <typename STR>
class StringPieceReplacements : public Replacements<typename STR::value_type> {
 public:
  void SetSchemeStr(const BasicStringPiece<STR>& s) {
    this->SetScheme(s.data(), Component(0, static_cast<int>(s.length())));
  }
  void SetUsernameStr(const BasicStringPiece<STR>& s) {
    this->SetUsername(s.data(), Component(0, static_cast<int>(s.length())));
  }
  void SetPasswordStr(const BasicStringPiece<STR>& s) {
    this->SetPassword(s.data(), Component(0, static_cast<int>(s.length())));
  }
  void SetHostStr(const BasicStringPiece<STR>& s) {
    this->SetHost(s.data(), Component(0, static_cast<int>(s.length())));
  }
  void SetPortStr(const BasicStringPiece<STR>& s) {
    this->SetPort(s.data(), Component(0, static_cast<int>(s.length())));
  }
  void SetPathStr(const BasicStringPiece<STR>& s) {
    this->SetPath(s.data(), Component(0, static_cast<int>(s.length())));
  }
  void SetQueryStr(const BasicStringPiece<STR>& s) {
    this->SetQuery(s.data(), Component(0, static_cast<int>(s.length())));
  }
  void SetRefStr(const BasicStringPiece<STR>& s) { this->SetRef(s.data(), Component(0, static_cast<int>(s.length()))); }
};

}  // namespace uri
}  // namespace common
