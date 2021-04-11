/*  Copyright (C) 2014-2021 FastoGT. All right reserved.
    This file is part of iptv_cloud.
    iptv_cloud is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    iptv_cloud is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with iptv_cloud.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <common/license/utils.h>
#include <common/macros.h>

namespace {
void native_cpuid(unsigned int* eax, unsigned int* ebx, unsigned int* ecx, unsigned int* edx) {
#if defined(ARCH_CPU_X86_FAMILY)
  /* ecx is often an input as well as an output. */
  asm volatile("cpuid" : "=a"(*eax), "=b"(*ebx), "=c"(*ecx), "=d"(*edx) : "0"(*eax), "2"(*ecx) : "memory");
#else

#endif
}

}  // namespace

namespace common {
namespace license {

std::string GetNativeCpuID() {
  unsigned int eax = 0, ebx = 0, ecx = 0, edx = 0;
  native_cpuid(&eax, &ebx, &ecx, &edx);
  char result[36] = {0};
  int res = snprintf(result, sizeof(result), "%08X %08X %08X %08X", eax, ebx, ecx, edx);
  if (res < 0 || sizeof(result) < static_cast<size_t>(res)) {
    return std::string();
  }
  return std::string(result, res);
}

}  // namespace license
}  // namespace common
