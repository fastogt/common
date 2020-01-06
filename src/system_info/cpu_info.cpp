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

#include <common/system_info/cpu_info.h>

#include <common/patterns/singleton_pattern.h>

#include <common/sprintf.h>

#include <libcpuid/libcpuid.h>

namespace common {
namespace {

struct CurrentCpuInfo {
  CurrentCpuInfo() : info(system_info::CpuInfo::MakeCpuInfo()) {}

  const system_info::CpuInfo info;

  static CurrentCpuInfo* GetInstance() { return &patterns::LazySingleton<CurrentCpuInfo>::GetInstance(); }
};

}  // namespace
namespace system_info {

struct CpuInfo::CpuInfoImpl {
  CpuInfoImpl() : raw_cpuid_(), cpuid_(), is_valid_(false) {}

  cpu_raw_data_t raw_cpuid_;
  cpu_id_t cpuid_;  // contains recognized CPU features data
  bool is_valid_;
};

CpuInfo::CpuInfo() : impl_(new CpuInfoImpl) {}

std::string CpuInfo::GetBrandName() const {
  return impl_->cpuid_.brand_str;
}

core_count_t CpuInfo::GetCoreCount() const {
  if (impl_->cpuid_.num_cores <= 0) {
    impl_->cpuid_.num_cores = 1;
  }

  return impl_->cpuid_.num_cores;
}

lcpu_count_t CpuInfo::GetLogicalCpusCount() const {
  if (impl_->cpuid_.num_logical_cpus <= 0) {
    impl_->cpuid_.num_logical_cpus = 1;
  }

  return impl_->cpuid_.num_logical_cpus;
}

lcpu_count_t CpuInfo::GetThreadsOnCore() const {
  return GetLogicalCpusCount() / GetCoreCount();
}

std::string CpuInfo::GetNativeCpuID() const {
  return MemSPrintf("%08X %08X %08X %08X", impl_->raw_cpuid_.basic_cpuid[0][0], impl_->raw_cpuid_.basic_cpuid[0][1],
                    impl_->raw_cpuid_.basic_cpuid[0][2], impl_->raw_cpuid_.basic_cpuid[0][3]);
}

bool CpuInfo::IsValid() const {
  return impl_->is_valid_;
}

bool CpuInfo::Equals(const CpuInfo& other) const {
  return IsValid() == other.IsValid() && GetBrandName() == other.GetBrandName() &&
         GetCoreCount() == other.GetCoreCount() && GetLogicalCpusCount() == other.GetLogicalCpusCount() &&
         GetNativeCpuID() == other.GetNativeCpuID();
}

const CpuInfo& CurrentCpuInfo() {
  return CurrentCpuInfo::GetInstance()->info;
}

CpuInfo CpuInfo::MakeCpuInfo() {
  if (!cpuid_present()) {  // check for CPUID presence
    return CpuInfo();
  }

  struct cpu_raw_data_t raw;           // contains only raw data
  if (cpuid_get_raw_data(&raw) < 0) {  // obtain the raw CPUID data
    return CpuInfo();
  }

  cpu_id_t data;
  if (cpu_identify(&raw, &data) < 0) {  // identify the CPU, using the given raw data.
    return CpuInfo();
  }

  CpuInfo inf;
  inf.impl_->raw_cpuid_ = raw;
  inf.impl_->cpuid_ = data;
  inf.impl_->is_valid_ = true;
  return inf;
}

}  // namespace system_info
}  // namespace common
