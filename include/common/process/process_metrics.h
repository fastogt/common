/*  Copyright (C) 2014-2021 FastoGT. All right reserved.

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

#include <common/optional.h>
#include <common/time.h>

namespace common {
namespace process {

class ProcessMetrics {
 public:
#if defined(OS_WIN)
  typedef void* process_handle_t;
#else
  typedef pid_t process_handle_t;
#endif

  ~ProcessMetrics();

  static std::unique_ptr<ProcessMetrics> CreateProcessMetrics(process_handle_t process);

  double GetPlatformIndependentCPUUsage();
  time64_t GetCumulativeCPUUsage();

#if defined(OS_LINUX) || defined(OS_ANDROID)
  // Resident Set Size is a Linux/Android specific memory concept. Do not attempt to extend this to other platforms.
  size_t GetResidentSetSize() const;
#endif

 private:
  explicit ProcessMetrics(process_handle_t process);

  process_handle_t process_;
  Optional<time64_t> last_cumulative_cpu_;
  time64_t last_cpu_time_;
};

}  // namespace process
}  // namespace common
