/*  Copyright (C) 2014-2018 FastoGT. All right reserved.

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

#include "common/system_info/system_info.h"

#include <sys/sysctl.h>
#include <sys/types.h>

#include <common/macros.h>

namespace common {
namespace system_info {

Optional<size_t> AmountOfPhysicalMemory() {
  int pages, page_size;
  size_t size = sizeof(pages);
  sysctlbyname("vm.stats.vm.v_page_count", &pages, &size, NULL, 0);
  sysctlbyname("vm.stats.vm.v_page_size", &page_size, &size, NULL, 0);
  if (pages == -1 || page_size == -1) {
    DNOTREACHED();
    return Optional<size_t>();
  }
  return static_cast<int64_t>(pages) * page_size;
}

Optional<size_t> AmountOfAvailablePhysicalMemory() {
  int pages, page_size;
  size_t size = sizeof(pages);
  sysctlbyname("vm.stats.vm.v_free_count", &pages, &size, NULL, 0);
  sysctlbyname("vm.stats.vm.v_page_size", &page_size, &size, NULL, 0);
  if (pages == -1 || page_size == -1) {
    DNOTREACHED();
    return Optional<size_t>();
  }
  return pages * page_size;
}

Optional<size_t> AmountOfTotalRAM() {
  return AmountOfPhysicalMemory();
}

Optional<size_t> AmountOfAvailableRAM() {
  return AmountOfAvailablePhysicalMemory();
}

}  // namespace system_info
}  // namespace common
