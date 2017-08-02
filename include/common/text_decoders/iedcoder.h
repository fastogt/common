/*  Copyright (C) 2014-2017 FastoGT. All right reserved.

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

#include <string>  // for string

#include <common/error.h>  // for Error

namespace common {

enum EDTypes { Base64, CompressZlib, CompressSnappy, Hex, MsgPack, HtmlEsc };

const std::string EDecoderTypes[] = {"Base64", "GZip", "Snappy", "Hex", "MsgPack", "HtmlEscape"};

class IEDcoder {
 public:
  Error Encode(const std::string& data, std::string* out) WARN_UNUSED_RESULT;
  Error Decode(const std::string& data, std::string* out) WARN_UNUSED_RESULT;
  EDTypes GetType() const;

  static IEDcoder* CreateEDCoder(EDTypes type);
  static IEDcoder* CreateEDCoder(const std::string& name);

  virtual ~IEDcoder();

 protected:
  explicit IEDcoder(EDTypes type);

 private:
  virtual Error EncodeImpl(const std::string& data, std::string* out) = 0;
  virtual Error DecodeImpl(const std::string& data, std::string* out) = 0;
  const EDTypes type_;
};

std::string ConvertToString(EDTypes t);
bool ConvertFromString(const std::string& from, EDTypes* out) WARN_UNUSED_RESULT;

}  // namespace common
