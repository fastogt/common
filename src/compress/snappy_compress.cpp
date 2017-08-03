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

#include <common/compress/snappy_compress.h>

#ifdef HAVE_SNAPPY
#include <snappy.h>

namespace common {
namespace compress {

Error EncodeSnappy(const std::string& data, std::string* out) {
  if (data.empty() || !out) {
    return make_inval_error_value(ErrorValue::E_ERROR);
  }

  std::string lout;
  size_t writed_bytes = snappy::Compress(data.c_str(), data.length(), &lout);
  if (writed_bytes == 0) {
    return make_inval_error_value(ErrorValue::E_ERROR);
  }

  *out = lout;
  return Error();
}

Error DecodeSnappy(const std::string& data, std::string* out) {
  if (data.empty() || !out) {
    return make_inval_error_value(ErrorValue::E_ERROR);
  }

  std::string lout;
  size_t writed_bytes = snappy::Uncompress(data.c_str(), data.length(), &lout);
  if (writed_bytes == 0) {
    return make_inval_error_value(ErrorValue::E_ERROR);
  }

  *out = lout;
  return Error();
}

}  // namespace compress
}  // namespace common
#endif
