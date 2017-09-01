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

#include <memory>
#include <string>  // for string

#include <common/system_info/types.h>  // for lcpu_count_t, core_count_t

namespace common {
namespace system_info {

class CpuInfo {
 public:
  static CpuInfo MakeCpuInfo();

  std::string BrandName() const;
  core_count_t CoreCount() const;
  lcpu_count_t LogicalCpusCount() const;
  lcpu_count_t ThreadsOnCore() const;

  bool IsValid() const;
  bool Equals(const CpuInfo& other) const;

 private:
  CpuInfo();

  struct CpuInfoImpl;
  std::shared_ptr<CpuInfoImpl> impl_;
};

inline bool operator==(const CpuInfo& lhs, const CpuInfo& rhs) {
  return lhs.Equals(rhs);
}

inline bool operator!=(const CpuInfo& lhs, const CpuInfo& rhs) {
  return !(lhs == rhs);
}

const CpuInfo& CurrentCpuInfo();

}  // namespace system_info
}  // namespace common
