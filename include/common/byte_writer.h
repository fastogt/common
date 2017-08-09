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

#include <sstream>

#include <common/types.h>

namespace common {

template <typename CharT, typename TraitsT = std::char_traits<CharT> >
class ByteWriter {
 public:
  typedef typename std::basic_ostringstream<CharT, TraitsT> ostringstream_t;
  typedef std::basic_string<CharT, TraitsT> string_t;

  ByteWriter() : buffer_() {}

  inline string_t GetBuffer() const { return buffer_.str(); }

  template <typename T>
  inline void AppendObject(T obj) {
    buffer_ << obj;
  }

 private:
  ostringstream_t buffer_;
};

typedef ByteWriter<char> string_byte_writer;
typedef ByteWriter<byte_t> buffer_byte_writer;

std::basic_ostream<byte_t>& operator<<(std::basic_ostream<byte_t>& out, const buffer_t& buff);
std::basic_ostream<byte_t>& operator<<(std::basic_ostream<byte_t>& out, const std::string& buff);

template <typename CharT, typename TraitsT, typename T>
inline ByteWriter<CharT, TraitsT>& operator<<(ByteWriter<CharT, TraitsT>& os, T obj) {
  os.AppendObject(obj);
  return os;
}

}  // namespace common
