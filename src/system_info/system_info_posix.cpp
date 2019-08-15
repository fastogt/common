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

#include <common/system_info/system_info.h>

#include <sys/utsname.h>

#include <limits>

#if defined(OS_ANDROID)
#include <sys/vfs.h>
#define statvfs statfs  // Android uses a statvfs-like statfs struct and call.
#else
#include <sys/statvfs.h>
#endif

#if defined(OS_LINUX)
#include <linux/magic.h>
#include <sys/vfs.h>
#endif

#include <common/eintr_wrapper.h>
#include <common/macros.h>

namespace {
#if defined(OS_LINUX)
bool IsStatsZeroIfUnlimited(const std::string& path) {
  struct statfs stats;

  if (HANDLE_EINTR(statfs(path.c_str(), &stats)) != 0) {
    return false;
  }

  switch (stats.f_type) {
    case TMPFS_MAGIC:
    case HUGETLBFS_MAGIC:
    case RAMFS_MAGIC:
      return true;
  }
  return false;
}
#endif  // defined(OS_LINUX)

bool GetDiskSpaceInfo(const std::string& path, size_t* available_bytes, size_t* total_bytes) {
  struct statvfs stats;
  if (HANDLE_EINTR(statvfs(path.c_str(), &stats)) != 0) {
    return false;
  }

#if defined(OS_LINUX)
  const bool zero_size_means_unlimited = stats.f_blocks == 0 && IsStatsZeroIfUnlimited(path);
#else
  const bool zero_size_means_unlimited = false;
#endif

  if (available_bytes) {
    *available_bytes = zero_size_means_unlimited ? std::numeric_limits<size_t>::max() : stats.f_bavail * stats.f_frsize;
  }

  if (total_bytes) {
    *total_bytes = zero_size_means_unlimited ? std::numeric_limits<size_t>::max() : stats.f_blocks * stats.f_frsize;
  }
  return true;
}
}  // namespace

namespace common {
namespace system_info {

// static
Optional<size_t> AmountOfFreeDiskSpace(const std::string& path) {
  size_t available;
  if (!GetDiskSpaceInfo(path, &available, nullptr)) {
    return Optional<size_t>();
  }
  return available;
}

// static
Optional<size_t> AmountOfTotalDiskSpace(const std::string& path) {
  size_t total;
  if (!GetDiskSpaceInfo(path, nullptr, &total)) {
    return Optional<size_t>();
  }
  return total;
}

Optional<size_t> GetCurrentProcessRss() {
  long rss = 0L;
  FILE* fp = NULL;
  if ((fp = fopen("/proc/self/statm", "r")) == NULL) {
    return Optional<size_t>();
  }

  if (fscanf(fp, "%*s%ld", &rss) != 1) {
    fclose(fp);
    return Optional<size_t>();
  }
  fclose(fp);
  return rss * sysconf(_SC_PAGESIZE);
}

Optional<size_t> GetProcessRss(pid_t pid) {
  char buff[64] = {0};
  snprintf(buff, sizeof(buff), "ps --no-headers -p %ld -o rss", static_cast<long>(pid));

  FILE* fp = popen(buff, "r");
  if (!fp) {
    return Optional<size_t>();
  }

  char path[16] = {0};
  char* res = fgets(path, sizeof(path) - 1, fp);
  pclose(fp);

  if (!res) {
    return Optional<size_t>();
  }
  return std::stol(res);
}

Optional<double> GetCpuLoad(pid_t pid) {
  char buff[32] = {0};
  snprintf(buff, sizeof(buff), "ps -o pcpu= %ld", static_cast<long>(pid));

  FILE* fp = popen(buff, "r");
  if (!fp) {
    return Optional<double>();
  }

  char path[16] = {0};
  char* res = fgets(path, sizeof(path) - 1, fp);
  pclose(fp);

  if (!res) {
    return Optional<double>();
  }

  return std::stod(res);
}

#if !defined(OS_MACOSX) && !defined(OS_ANDROID)
std::string OperatingSystemName() {
  struct utsname info;
  int res = uname(&info);
  if (res < 0) {
    DNOTREACHED() << "uname: " << res;
    return std::string();
  }

  return std::string(info.sysname);
}
#endif

#if !defined(OS_MACOSX)
std::string OperatingSystemVersion() {
  struct utsname info;
  int res = uname(&info);
  if (res < 0) {
    DNOTREACHED() << "uname: " << res;
    return std::string();
  }

  return std::string(info.release);
}
#endif

std::string OperatingSystemArchitecture() {
  struct utsname info;
  int res = uname(&info);
  if (res < 0) {
    DNOTREACHED() << "uname: " << res;
    return std::string();
  }
  std::string arch(info.machine);
  if (arch == "i386" || arch == "i486" || arch == "i586" || arch == "i686") {
    arch = "i386";
  } else if (arch == "amd64") {
    arch = "x86_64";
  }
  return arch;
}

}  // namespace system_info
}  // namespace common
