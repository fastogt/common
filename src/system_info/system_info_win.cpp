#include "common/system_info/cpu_info.h"

#include <windows.h>
#include <limits>

#include <common/sprintf.h>

namespace {

int64_t AmountOfMemory(DWORDLONG MEMORYSTATUSEX::*memory_field) {
  MEMORYSTATUSEX memory_info;
  memory_info.dwLength = sizeof(memory_info);
  if (!GlobalMemoryStatusEx(&memory_info)) {
    NOTREACHED();
    return 0;
  }
  int64_t rv = static_cast<int64_t>(memory_info.*memory_field);
  return rv < 0 ? std::numeric_limits<int64_t>::max() : rv;
}

}  // namespace

namespace common {
namespace system_info {

int64_t AmountOfPhysicalMemory() {
  return AmountOfMemory(&MEMORYSTATUSEX::ullTotalPhys);
}

int64_t AmountOfAvailablePhysicalMemory() {
  return AmountOfMemory(&MEMORYSTATUSEX::ullAvailPhys);
}

std::string OperatingSystemName() {
  return "Windows NT";
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
  BOOL res = ::GetVersionEx((OSVERSIONINFO*)&version_info);
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
      return "x86";
    case PROCESSOR_ARCHITECTURE_AMD64:
      return "x86_64";
    case PROCESSOR_ARCHITECTURE_IA64:
      return "ia64";
    default:
      return "unknown";
  }
}
}
}
