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

#include <unistd.h>

#include <common/convert2string.h>
#include <common/file_system/file_system.h>
#include <common/file_system/path.h>
#include <common/string_util.h>
#include <common/time.h>

namespace {
const char kProcDir[] = "/proc";
const char kStatFile[] = "stat";
enum ProcStatsFields {
  VM_COMM = 1,         // Filename of executable, without parentheses.
  VM_STATE = 2,        // Letter indicating the state of the process.
  VM_PPID = 3,         // PID of the parent.
  VM_PGRP = 4,         // Process group id.
  VM_MINFLT = 9,       // Minor page fault count excluding children.
  VM_MAJFLT = 11,      // Major page fault count excluding children.
  VM_UTIME = 13,       // Time scheduled in user mode in clock ticks.
  VM_STIME = 14,       // Time scheduled in kernel mode in clock ticks.
  VM_NUMTHREADS = 19,  // Number of threads.
  VM_STARTTIME = 21,   // The time the process started in clock ticks.
  VM_VSIZE = 22,       // Virtual memory size in bytes.
  VM_RSS = 23,         // Resident Set Size in pages.
};
}  // namespace

namespace common {
namespace process {

namespace {

time64_t ClockTicksToTimeDelta(int clock_ticks) {
  // This queries the /proc-specific scaling factor which is
  // conceptually the system hertz.  To dump this value on another
  // system, try
  //   od -t dL /proc/self/auxv
  // and look for the number after 17 in the output; mine is
  //   0000040          17         100           3   134512692
  // which means the answer is 100.
  // It may be the case that this value is always 100.
  static const int kHertz = sysconf(_SC_CLK_TCK);
  return time::kMillisecondsPerSecond * clock_ticks / kHertz;
}

int64_t GetProcStatsFieldAsInt64(const std::vector<std::string>& proc_stats, ProcStatsFields field_num) {
  DCHECK_GE(field_num, VM_PPID);
  CHECK_LT(static_cast<size_t>(field_num), proc_stats.size());

  int64_t value;
  return ConvertFromString(proc_stats[field_num], &value) ? value : 0;
}

bool ReadProcFile(const file_system::ascii_file_string_path& file, std::string* buffer) {
  if (!buffer) {
    return false;
  }

  std::string lbuffer;
  if (!file_system::read_file_to_string(file.GetPath(), &lbuffer)) {
    return false;
  }

  if (lbuffer.empty()) {
    return false;
  }

  *buffer = lbuffer;
  return true;
}

bool ReadProcStats(pid_t pid, std::string* buffer) {
  file_system::ascii_directory_string_path proc_dir(kProcDir);
  const auto proc_pid_dir = proc_dir.MakeDirectoryStringPath(ConvertToString(pid));
  if (!proc_pid_dir) {
    return false;
  }

  const auto stat_file = proc_pid_dir->MakeFileStringPath(kStatFile);
  if (!stat_file) {
    return false;
  }
  return ReadProcFile(*stat_file, buffer);
}

bool ParseProcStats(const std::string& stats_data, std::vector<std::string>* proc_stats) {
  // |stats_data| may be empty if the process disappeared somehow.
  // e.g. http://crbug.com/145811
  if (stats_data.empty()) {
    return false;
  }

  // The stat file is formatted as:
  // pid (process name) data1 data2 .... dataN
  // Look for the closing paren by scanning backwards, to avoid being fooled by
  // processes with ')' in the name.
  size_t open_parens_idx = stats_data.find(" (");
  size_t close_parens_idx = stats_data.rfind(") ");
  if (open_parens_idx == std::string::npos || close_parens_idx == std::string::npos ||
      open_parens_idx > close_parens_idx) {
    NOTREACHED();
    return false;
  }
  open_parens_idx++;

  proc_stats->clear();
  // PID.
  proc_stats->push_back(stats_data.substr(0, open_parens_idx));
  // Process name without parentheses.
  proc_stats->push_back(stats_data.substr(open_parens_idx + 1, close_parens_idx - (open_parens_idx + 1)));

  // Split the rest.
  std::vector<std::string> other_stats;
  Tokenize(stats_data.substr(close_parens_idx + 2), " ", &other_stats);
  for (const auto& i : other_stats) {
    proc_stats->push_back(i);
  }
  return true;
}

int64_t GetProcessCPU(pid_t pid) {
  std::string buffer;
  std::vector<std::string> proc_stats;
  if (!ReadProcStats(pid, &buffer) || !ParseProcStats(buffer, &proc_stats)) {
    return -1;
  }

  return GetProcStatsFieldAsInt64(proc_stats, VM_UTIME) + GetProcStatsFieldAsInt64(proc_stats, VM_STIME);
}

size_t GetProcStatsFieldAsSizeT(const std::vector<std::string>& proc_stats, ProcStatsFields field_num) {
  DCHECK_GE(field_num, VM_PPID);
  CHECK_LT(static_cast<size_t>(field_num), proc_stats.size());

  size_t value;
  return ConvertFromString(proc_stats[field_num], &value) ? value : 0;
}

size_t ReadProcStatsAndGetFieldAsSizeT(pid_t pid, ProcStatsFields field_num) {
  std::string stats_data;
  if (!ReadProcStats(pid, &stats_data)) {
    return 0;
  }
  std::vector<std::string> proc_stats;
  if (!ParseProcStats(stats_data, &proc_stats)) {
    return 0;
  }
  return GetProcStatsFieldAsSizeT(proc_stats, field_num);
}
}  // namespace

time64_t ProcessMetrics::GetCumulativeCPUUsage() {
  return ClockTicksToTimeDelta(GetProcessCPU(process_));
}

size_t ProcessMetrics::GetResidentSetSize() const {
  return ReadProcStatsAndGetFieldAsSizeT(process_, VM_RSS) * getpagesize();
}

}  // namespace process
}  // namespace common
