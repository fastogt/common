/*  Copyright (C) 2014-2022 FastoGT. All right reserved.

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

#include <common/macros.h>
#include <common/system_info/types.h>  // for lcpu_count_t, core_count_t

#include <string>  // for string

namespace common {
namespace system_info {

#if defined(ARCH_CPU_X86_FAMILY)
namespace internal {

// Compute the CPU family and model based on the vendor and CPUID signature.
// Returns in order: family, model, extended family, extended model.
std::tuple<int, int, int, int> ComputeX86FamilyAndModel(const std::string& vendor, int signature);

}  // namespace internal
#endif  // defined(ARCH_CPU_X86_FAMILY)

// Query information about the processor.
class CPU final {
 public:
  explicit CPU();

  // Accessors for CPU information.
  const std::string& vendor_name() const { return cpu_vendor_; }
  bool is_running_in_vm() const { return is_running_in_vm_; }
  const std::string& cpu_brand() const { return cpu_brand_; }

 private:
  // Query the processor for CPUID information.
  void Initialize();

  bool is_running_in_vm_ = false;
  std::string cpu_vendor_ = "Unknown";
  std::string cpu_brand_;
};

}  // namespace system_info
}  // namespace common
