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

#include <common/system_info/system_info.h>

#include <windows.h>

#include <psapi.h>

#include <limits>

#include <common/sprintf.h>

namespace {

bool AmountOfMemory(DWORDLONG MEMORYSTATUSEX::*memory_field, size_t* amount) {
  MEMORYSTATUSEX memory_info;
  memory_info.dwLength = sizeof(memory_info);
  if (!GlobalMemoryStatusEx(&memory_info)) {
    return false;
  }

  size_t rv = memory_info.*memory_field;
  if (rv == SIZE_MAX) {
    return false;
  }

  *amount = rv;
  return true;
}

bool GetDiskSpaceInfo(const std::string& path, size_t* available_bytes, size_t* total_bytes) {
  ULARGE_INTEGER available;
  ULARGE_INTEGER total;
  ULARGE_INTEGER free;
  if (!GetDiskFreeSpaceExA(path.c_str(), &available, &total, &free)) {
    return false;
  }

  if (available_bytes) {
    size_t lavailable_bytes = available.QuadPart;
    if (lavailable_bytes == std::numeric_limits<size_t>::max()) {
      return false;
    }
    *available_bytes = lavailable_bytes;
  }

  if (total_bytes) {
    size_t ltotal_bytes = total.QuadPart;
    if (ltotal_bytes == std::numeric_limits<size_t>::max()) {
      return false;
    }
    *total_bytes = ltotal_bytes;
  }
  return true;
}

}  // namespace

namespace common {
namespace system_info {

Optional<size_t> AmountOfPhysicalMemory() {
  size_t memory;
  bool res = AmountOfMemory(&MEMORYSTATUSEX::ullTotalPhys, &memory);
  if (!res) {
    return Optional<size_t>();
  }

  return memory;
}

Optional<size_t> AmountOfAvailablePhysicalMemory() {
  size_t memory;
  bool res = AmountOfMemory(&MEMORYSTATUSEX::ullAvailPhys, &memory);
  if (!res) {
    return Optional<size_t>();
  }

  return memory;
}

Optional<size_t> AmountOfTotalRAM() {
  return AmountOfPhysicalMemory();
}

Optional<size_t> AmountOfAvailableRAM() {
  return AmountOfAvailablePhysicalMemory();
}

Optional<size_t> AmountOfFreeDiskSpace(const std::string& path) {
  size_t available;
  if (!GetDiskSpaceInfo(path, &available, nullptr)) {
    return Optional<size_t>();
  }
  return available;
}

Optional<size_t> AmountOfTotalDiskSpace(const std::string& path) {
  size_t total;
  if (!GetDiskSpaceInfo(path, nullptr, &total)) {
    return Optional<size_t>();
  }
  return total;
}

std::string OperatingSystemName() {
  return "windows";
}

std::string OperatingSystemVersion() {
  enum Version {
    VERSION_PRE_XP = 0,  // Not supported.
    VERSION_XP,
    VERSION_SERVER_2003,  // Also includes XP Pro x64 and Server 2003 R2.
    VERSION_VISTA,        // Also includes Windows Server 2008.
    VERSION_WIN7,         // Also includes Windows Server 2008 R2.
    VERSION_WIN8,         // Also includes Windows Server 2012.
    VERSION_WIN8_1,       // Also includes Windows Server 2012 R2.
    VERSION_WIN10,        // Also includes Windows 10 Server.
    VERSION_WIN_LAST,     // Indicates error condition.
  };

  OSVERSIONINFOEX version_info;
  ZeroMemory(&version_info, sizeof(OSVERSIONINFOEX));
  version_info.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
  BOOL res = ::GetVersionEx(reinterpret_cast<OSVERSIONINFO*>(&version_info));
  if (!res) {
    return "0.0";
  }
  DWORD major = version_info.dwMajorVersion;
  DWORD minor = version_info.dwMinorVersion;
  DWORD build = version_info.dwBuildNumber;
  Version version;

  if ((major == 5) && (minor > 0)) {
    // Treat XP Pro x64, Home Server, and Server 2003 R2 as Server 2003.
    version = (minor == 1) ? VERSION_XP : VERSION_SERVER_2003;
  } else if (major == 6) {
    switch (minor) {
      case 0:
        // Treat Windows Server 2008 the same as Windows Vista.
        version = VERSION_VISTA;
        break;
      case 1:
        // Treat Windows Server 2008 R2 the same as Windows 7.
        version = VERSION_WIN7;
        break;
      case 2:
        // Treat Windows Server 2012 the same as Windows 8.
        version = VERSION_WIN8;
        break;
      default:
        DCHECK_EQ(minor, 3);
        version = VERSION_WIN8_1;
        break;
    }
  } else if (major == 10) {
    version = VERSION_WIN10;
  } else if (major > 6) {
    NOTREACHED();
    version = VERSION_WIN_LAST;
  }

  UNUSED(build);
  UNUSED(version);

  DWORD sp_major = version_info.wServicePackMajor;
  DWORD sp_minor = version_info.wServicePackMinor;

  std::string versionstr = common::MemSPrintf("%d.%d", major, minor);
  if (sp_major != 0) {
    versionstr += common::MemSPrintf(" SP%d", sp_major);
    if (sp_minor != 0) {
      versionstr += common::MemSPrintf(".%d", sp_minor);
    }
  }
  return versionstr;
}

std::string OperatingSystemArchitecture() {
  SYSTEM_INFO system_info;
  ZeroMemory(&system_info, sizeof(SYSTEM_INFO));
  ::GetNativeSystemInfo(&system_info);
  switch (system_info.wProcessorArchitecture) {
    case PROCESSOR_ARCHITECTURE_INTEL:
      return "i386";
    case PROCESSOR_ARCHITECTURE_AMD64:
      return "x86_64";
    case PROCESSOR_ARCHITECTURE_IA64:
      return "ia64";
    default:
      return "unknown";
  }
}

}  // namespace system_info
}  // namespace common
