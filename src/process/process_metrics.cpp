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

#include <common/process/process_metrics.h>

#include <common/time.h>

namespace common {
namespace process {

ProcessMetrics::~ProcessMetrics() {}

std::unique_ptr<ProcessMetrics> ProcessMetrics::CreateProcessMetrics(process_handle_t process) {
  return std::unique_ptr<ProcessMetrics>(new ProcessMetrics(process));
}

double ProcessMetrics::GetPlatformIndependentCPUUsage() {
  time64_t cumulative_cpu = GetCumulativeCPUUsage();
  time64_t time = time::current_utc_mstime();

  if (!last_cumulative_cpu_) {
    // First call, just set the last values.
    last_cumulative_cpu_ = cumulative_cpu;
    last_cpu_time_ = time;
    return 0;
  }

  time64_t system_time_delta = cumulative_cpu - *last_cumulative_cpu_;
  time64_t time_delta = time - last_cpu_time_;
  if (time_delta == 0) {
    return 0;
  }

  last_cumulative_cpu_ = cumulative_cpu;
  last_cpu_time_ = time;

  return 100.0 * static_cast<double>(system_time_delta) / static_cast<double>(time_delta);
}

ProcessMetrics::ProcessMetrics(process_handle_t process)
    : process_(process), last_cumulative_cpu_(), last_cpu_time_() {}

}  // namespace process
}  // namespace common
