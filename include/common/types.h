/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

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

#include <stdint.h>  // for int64_t, uint8_t

#include <atomic>
#include <iosfwd>  // for basic_ifstream, etc
#include <string>  // for string
#include <vector>  // for vector

#include <common/string16.h>  // for char16, etc

namespace common {

typedef std::atomic<bool> atomic_bool;
typedef std::atomic_uchar atomic_uchar;
typedef std::atomic_uint atomic_uint;
typedef std::atomic_ulong atomic_ulong;
typedef std::atomic_ullong atomic_ullong;
typedef std::atomic_size_t atomic_size;
template <typename Tp>
using atomic = std::atomic<Tp>;

enum tribool { FAIL = 0, SUCCESS = 1, INDETERMINATE = -1 };

typedef uint8_t byte_t;
typedef std::vector<byte_t> byte_array_t;
typedef byte_array_t buffer_t;

typedef std::basic_ostream<char16, string16_char_traits> string16_ostream;
typedef std::basic_istream<char16, string16_char_traits> string16_istream;
typedef std::basic_ofstream<char16, string16_char_traits> string16_ofstream;
typedef std::basic_ifstream<char16, string16_char_traits> string16_ifstream;

const char* common_strerror(int err);

std::string EscapedText(const std::string& str);

typedef int64_t time64_t;  // millisecond

class IMetaClassInfo {
 public:
  virtual const char* ClassName() const = 0;
  virtual ~IMetaClassInfo();
};

template <typename T>
class ClonableBase {
 public:
  virtual T* Clone() const = 0;
  virtual ~ClonableBase() {}
};

}  // namespace common

#define MAKE_BUFFER_SIZE(STR, SIZE) common::buffer_t(STR, STR + SIZE)
#define MAKE_BUFFER(STR) MAKE_BUFFER_SIZE(STR, sizeof(STR) - 1)