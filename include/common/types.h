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

#include <stdint.h>
#include <time.h>

#include <iosfwd>
#include <vector>  // for vector

namespace common {

enum tribool { FAIL = 0, SUCCESS = 1, INDETERMINATE = -1 };

typedef uint8_t byte_t;
typedef std::vector<byte_t> byte_array_t;
typedef byte_array_t buffer_t;

typedef int64_t time64_t;  // millisecond
typedef time_t utctime_t;  // seconds

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

std::ostream& operator<<(std::ostream& out, const buffer_t& buff);

}  // namespace common

#define MAKE_BUFFER_SIZE(STR, SIZE) common::buffer_t(STR, STR + SIZE)
#define MAKE_BUFFER(STR) MAKE_BUFFER_SIZE(STR, sizeof(STR) - 1)

#define MAKE_STRING_SIZE(STR, SIZE) std::string(STR, STR + SIZE)
#define MAKE_STRING(STR) MAKE_STRING_SIZE(STR, sizeof(STR) - 1)
