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

#include <common/system_info/cpu_info.h>
#include <common/types.h>
#include <string.h>

#if defined(ARCH_CPU_ARM_FAMILY)
#if defined(OS_ANDROID) || defined(OS_LINUX) || defined(OS_CHROMEOS)
#include <asm/hwcap.h>
#include <common/file_system/file_system.h>
#include <sys/auxv.h>
// Temporary definitions until a new hwcap.h is pulled in.
#define HWCAP2_MTE (1 << 18)
#define HWCAP2_BTI (1 << 17)
#else
#include <sys/sysctl.h>
#endif
#endif

#if defined(ARCH_CPU_X86_FAMILY)
#if defined(COMPILER_MSVC)
#include <immintrin.h>  // For _xgetbv()
#include <intrin.h>
#endif
#endif

namespace common {
namespace system_info {

#if defined(ARCH_CPU_X86_FAMILY)
namespace internal {

std::tuple<int, int, int, int> ComputeX86FamilyAndModel(const std::string& vendor, int signature) {
  int family = (signature >> 8) & 0xf;
  int model = (signature >> 4) & 0xf;
  int ext_family = 0;
  int ext_model = 0;

  // The "Intel 64 and IA-32 Architectures Developer's Manual: Vol. 2A"
  // specifies the Extended Model is defined only when the Base Family is
  // 06h or 0Fh.
  // The "AMD CPUID Specification" specifies that the Extended Model is
  // defined only when Base Family is 0Fh.
  // Both manuals define the display model as
  // {ExtendedModel[3:0],BaseModel[3:0]} in that case.
  if (family == 0xf || (family == 0x6 && vendor == "GenuineIntel")) {
    ext_model = (signature >> 16) & 0xf;
    model += ext_model << 4;
  }
  // Both the "Intel 64 and IA-32 Architectures Developer's Manual: Vol. 2A"
  // and the "AMD CPUID Specification" specify that the Extended Family is
  // defined only when the Base Family is 0Fh.
  // Both manuals define the display family as {0000b,BaseFamily[3:0]} +
  // ExtendedFamily[7:0] in that case.
  if (family == 0xf) {
    ext_family = (signature >> 20) & 0xff;
    family += ext_family;
  }

  return {family, model, ext_family, ext_model};
}

}  // namespace internal
#endif  // defined(ARCH_CPU_X86_FAMILY)

CPU::CPU() {
  Initialize();
}

namespace {

#if defined(ARCH_CPU_X86_FAMILY)
#if !defined(COMPILER_MSVC)

#if defined(__pic__) && defined(__i386__)

void __cpuid(int cpu_info[4], int info_type) {
  __asm__ volatile(
      "mov %%ebx, %%edi\n"
      "cpuid\n"
      "xchg %%edi, %%ebx\n"
      : "=a"(cpu_info[0]), "=D"(cpu_info[1]), "=c"(cpu_info[2]), "=d"(cpu_info[3])
      : "a"(info_type), "c"(0));
}

#else

void __cpuid(int cpu_info[4], int info_type) {
  __asm__ volatile("cpuid\n"
                   : "=a"(cpu_info[0]), "=b"(cpu_info[1]), "=c"(cpu_info[2]), "=d"(cpu_info[3])
                   : "a"(info_type), "c"(0));
}

#endif
#endif  // !defined(COMPILER_MSVC)

#endif  // ARCH_CPU_X86_FAMILY

#if defined(ARCH_CPU_ARM_FAMILY)
#if defined(OS_ANDROID) || defined(OS_LINUX) || defined(OS_CHROMEOS)
std::string* CpuInfoBrand() {
  static std::string* brand = []() {
    // This function finds the value from /proc/cpuinfo under the key "model
    // name" or "Processor". "model name" is used in Linux 3.8 and later (3.7
    // and later for arm64) and is shown once per CPU. "Processor" is used in
    // earler versions and is shown only once at the top of /proc/cpuinfo
    // regardless of the number CPUs.
    const char kModelNamePrefix[] = "model name\t: ";
    const char kProcessorPrefix[] = "Processor\t: ";

    std::string contents;
    file_system::read_file_to_string("/proc/cpuinfo", &contents);
    DCHECK(!contents.empty());

    std::istringstream iss(contents);
    std::string line;
    while (std::getline(iss, line)) {
      if (line.compare(0, strlen(kModelNamePrefix), kModelNamePrefix) == 0)
        return new std::string(line.substr(strlen(kModelNamePrefix)));
      if (line.compare(0, strlen(kProcessorPrefix), kProcessorPrefix) == 0)
        return new std::string(line.substr(strlen(kProcessorPrefix)));
    }

    return new std::string();
  }();

  return brand;
}
#else
std::string* CpuInfoBrand() {
  static std::string* brand = []() {
    size_t size;
    if (sysctlbyname("machdep.cpu.brand_string", NULL, &size, NULL, 0) != 0) {
      return new std::string();
    }

    char* machine_name = new char[size];
    if (sysctlbyname("machdep.cpu.brand_string", machine_name, &size, NULL, 0) != 0) {
      delete[] machine_name;
      return new std::string();
    }

    auto copy = new std::string(machine_name);
    delete[] machine_name;
    return copy;
  }();

  return brand;
}
#endif
#endif
}  // namespace

void CPU::Initialize() {
#if defined(ARCH_CPU_X86_FAMILY)
  int cpu_info[4] = {-1};
  // This array is used to temporarily hold the vendor name and then the brand
  // name. Thus it has to be big enough for both use cases. There are
  // static_asserts below for each of the use cases to make sure this array is
  // big enough.
  char cpu_string[sizeof(cpu_info) * 3 + 1];

  // __cpuid with an InfoType argument of 0 returns the number of
  // valid Ids in CPUInfo[0] and the CPU identification string in
  // the other three array elements. The CPU identification string is
  // not in linear order. The code below arranges the information
  // in a human readable form. The human readable order is CPUInfo[1] |
  // CPUInfo[3] | CPUInfo[2]. CPUInfo[2] and CPUInfo[3] are swapped
  // before using memcpy() to copy these three array elements to |cpu_string|.
  __cpuid(cpu_info, 0);
  int num_ids = cpu_info[0];
  std::swap(cpu_info[2], cpu_info[3]);
  static constexpr size_t kVendorNameSize = 3 * sizeof(cpu_info[1]);
  static_assert(kVendorNameSize < common::size(cpu_string), "cpu_string too small");
  memcpy(cpu_string, &cpu_info[1], kVendorNameSize);
  cpu_string[kVendorNameSize] = '\0';
  cpu_vendor_ = cpu_string;

  // Interpret CPU feature information.
  if (num_ids > 0) {
    int cpu_info7[4] = {0};
    __cpuid(cpu_info, 1);
    if (num_ids >= 7) {
      __cpuid(cpu_info7, 7);
    }

    // "Hypervisor Present Bit: Bit 31 of ECX of CPUID leaf 0x1."
    // See https://lwn.net/Articles/301888/
    // This is checking for any hypervisor. Hypervisors may choose not to
    // announce themselves. Hypervisors trap CPUID and sometimes return
    // different results to underlying hardware.
    is_running_in_vm_ = (cpu_info[2] & 0x80000000) != 0;
  }

  // Get the brand string of the cpu.
  __cpuid(cpu_info, 0x80000000);
  const int max_parameter = cpu_info[0];

  static constexpr int kParameterStart = 0x80000002;
  static constexpr int kParameterEnd = 0x80000004;
  static constexpr int kParameterSize = kParameterEnd - kParameterStart + 1;
  static_assert(kParameterSize * sizeof(cpu_info) + 1 == common::size(cpu_string), "cpu_string has wrong size");

  if (max_parameter >= kParameterEnd) {
    size_t i = 0;
    for (int parameter = kParameterStart; parameter <= kParameterEnd; ++parameter) {
      __cpuid(cpu_info, parameter);
      memcpy(&cpu_string[i], cpu_info, sizeof(cpu_info));
      i += sizeof(cpu_info);
    }
    cpu_string[i] = '\0';
    cpu_brand_ = cpu_string;
  }
#elif defined(ARCH_CPU_ARM_FAMILY)
#if defined(OS_ANDROID) || defined(OS_LINUX) || defined(OS_CHROMEOS)
  cpu_brand_ = *CpuInfoBrand();
#elif defined(OS_MACOSX)
  cpu_brand_ = *CpuInfoBrand();
#if defined(ARCH_CPU_ARM64)
  cpu_vendor_ = "Apple";
#else
#endif
#elif defined(OS_WIN)
  // Windows makes high-resolution thread timing information available in
  // user-space.
  has_non_stop_time_stamp_counter_ = true;
#endif
#endif
}

}  // namespace system_info
}  // namespace common
