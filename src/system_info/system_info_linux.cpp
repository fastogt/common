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

#include <common/file_system/file_system.h>
#include <common/file_system/path.h>
#include <common/file_system/string_path_utils.h>
#include <common/macros.h>  // for DNOTREACHED
#include <common/string_number_conversions.h>
#include <common/string_piece.h>
#include <common/string_split.h>
#include <common/string_util.h>
#include <common/system_info/system_info.h>
#include <unistd.h>  // for sysconf, _SC_AVPHYS_PAGES, etc

namespace {
const char kProcDir[] = "/proc";

std::string HostRootWithContext(const std::string& path) {
  return common::file_system::make_path("/", path);
}

std::string HostEtcWithContext(const std::string& path) {
  return common::file_system::make_path("/etc", path);
}

common::Optional<common::file_system::ascii_file_string_path> HostProcFile(const std::string& path) {
  common::file_system::ascii_directory_string_path proc_dir(kProcDir);
  return proc_dir.MakeFileStringPath(path);
}

common::Optional<common::file_system::ascii_directory_string_path> HostProcDir(const std::string& path) {
  common::file_system::ascii_directory_string_path proc_dir(kProcDir);
  return proc_dir.MakeDirectoryStringPath(path);
}

common::Optional<common::file_system::ascii_file_string_path> HostProcConcatFile(const std::string& path) {
  common::file_system::ascii_directory_string_path proc_dir(kProcDir);
  return proc_dir.MakeConcatFileStringPath(path);
}

std::pair<std::string, std::string> GetVirtualizationSystemAndRole() {
  std::string system, role;
  auto filedir = HostProcDir("xen");
  if (filedir && common::file_system::is_directory_exist(filedir->GetPath())) {
    system = "xen";
    role = "guest";  // assume guest
    auto cap = filedir->MakeFileStringPath("capabilities");
    if (cap && common::file_system::is_file_exist(cap->GetPath())) {
      std::string contents;
      if (common::file_system::read_file_to_string(cap->GetPath(), &contents)) {
        std::istringstream iss(contents);
        std::string line;
        while (std::getline(iss, line)) {
          if (line.find("control_d") != std::string::npos) {
            role = "host";
            break;
          }
        }
      }
    }
  }

  auto filename = HostProcFile("modules");
  if (filename && common::file_system::is_file_exist(filename->GetPath())) {
    std::string contents;
    if (common::file_system::read_file_to_string(filename->GetPath(), &contents)) {
      std::istringstream iss(contents);
      std::string line;
      while (std::getline(iss, line)) {
        if (line.find("kvm") != std::string::npos) {
          system = "kvm";
          role = "host";
          break;
        } else if (line.find("hv_util") != std::string::npos) {
          system = "hyperv";
          role = "guest";
          break;
        } else if (line.find("vboxdrv") != std::string::npos) {
          system = "vbox";
          role = "host";
          break;
        } else if (line.find("vboxguest") != std::string::npos) {
          system = "vbox";
          role = "guest";
          break;
        } else if (line.find("vmware") != std::string::npos) {
          system = "vmware";
          role = "guest";
          break;
        }
      }
    }
  }

  filename = HostProcFile("cpuinfo");
  if (filename && common::file_system::is_file_exist(filename->GetPath())) {
    std::string contents;
    if (common::file_system::read_file_to_string(filename->GetPath(), &contents)) {
      std::istringstream iss(contents);
      std::string line;
      while (std::getline(iss, line)) {
        if (line.find("QEMU Virtual CPU") != std::string::npos) {
          system = "kvm";
          role = "guest";
          break;
        } else if (line.find("Common KVM processor") != std::string::npos) {
          system = "kvm";
          role = "guest";
          break;
        } else if (line.find("Common 32-bit KVM processor") != std::string::npos) {
          system = "kvm";
          role = "guest";
          break;
        }
      }
    }
  }

  filename = HostProcConcatFile("bus/pci/devices");
  if (filename && common::file_system::is_file_exist(filename->GetPath())) {
    std::string contents;
    if (common::file_system::read_file_to_string(filename->GetPath(), &contents)) {
      std::istringstream iss(contents);
      std::string line;
      while (std::getline(iss, line)) {
        if (line.find("virtio-pci") != std::string::npos) {
          role = "guest";
          break;
        }
      }
    }
  }

  filename = HostProcConcatFile("/bc/0");
  if (filename && common::file_system::is_file_exist(filename->GetPath())) {
    system = "openvz";
    role = "host";
  }
  filename = HostProcFile("vz");
  if (common::file_system::is_file_exist(filename->GetPath())) {
    system = "openvz";
    role = "guest";
  }

  filename = HostProcConcatFile("/self/status");
  if (filename && common::file_system::is_file_exist(filename->GetPath())) {
    std::string contents;
    if (common::file_system::read_file_to_string(filename->GetPath(), &contents)) {
      std::istringstream iss(contents);
      std::string line;
      while (std::getline(iss, line)) {
        if (line.find("s_context") != std::string::npos) {
          system = "linux-vserver";
          break;
        } else if (line.find("VxID:") != std::string::npos) {
          system = "linux-vserver";
          break;
        }
      }
    }
  }

  filename = HostProcConcatFile("/1/environ");
  if (filename && common::file_system::is_file_exist(filename->GetPath())) {
    std::string contents;
    if (common::file_system::read_file_to_string(filename->GetPath(), &contents)) {
      std::istringstream iss(contents);
      std::string line;
      while (std::getline(iss, line)) {
        if (line.find("container=lxc") != std::string::npos) {
          system = "lxc";
          role = "guest";
          break;
        }
      }
    }
  }

  filename = HostProcConcatFile("/self/cgroup");
  if (filename && common::file_system::is_file_exist(filename->GetPath())) {
    std::string contents;
    if (common::file_system::read_file_to_string(filename->GetPath(), &contents)) {
      std::istringstream iss(contents);
      std::string line;
      while (std::getline(iss, line)) {
        if (line.find("lxc") != std::string::npos) {
          system = "lxc";
          role = "guest";
          break;
        } else if (line.find("docker") != std::string::npos) {
          system = "docker";
          role = "guest";
          break;
        } else if (line.find("machine-rkt") != std::string::npos) {
          system = "rkt";
          role = "guest";
          break;
        }
      }

      if (common::file_system::is_file_exist("/usr/bin/lxc-version")) {
        system = "lxc";
        role = "host";
      }
    }
  }

  if (filename && common::file_system::is_file_exist(HostEtcWithContext("os-release"))) {
    if (common::system_info::OperatingSystemName() == "coreos") {
      system = "rkt";  // Is it true?
      role = "host";
    }
  }

  if (filename && common::file_system::is_file_exist(HostRootWithContext(".dockerenv"))) {
    system = "docker";
    role = "guest";
  }

  return {system, role};
}

bool AmountOfMemory(int pages_name, size_t* size) {
  long pages = sysconf(pages_name);
  long page_size = sysconf(_SC_PAGESIZE);
  if (pages == -1 || page_size == -1) {
    DNOTREACHED();
    return false;
  }

  *size = pages * page_size;
  return true;
}

}  // namespace

namespace common {
namespace {
struct SystemMemoryInfoKB {
  int total = 0;
  int free = 0;
  int available = 0;
  int buffers = 0;
  int cached = 0;
};

bool ParseProcMeminfo(StringPiece meminfo_data, SystemMemoryInfoKB* meminfo) {
  // The format of /proc/meminfo is:
  //
  // MemTotal:      8235324 kB
  // MemFree:       1628304 kB
  // Buffers:        429596 kB
  // Cached:        4728232 kB
  // ...
  // There is no guarantee on the ordering or position
  // though it doesn't appear to change very often

  // As a basic sanity check at the end, make sure the MemTotal value will be at
  // least non-zero. So start off with a zero total.
  meminfo->total = 0;

  for (const StringPiece& line : SplitStringPiece(meminfo_data, "\n", KEEP_WHITESPACE, SPLIT_WANT_NONEMPTY)) {
    std::vector<StringPiece> tokens = SplitStringPiece(line, kWhitespaceASCII, TRIM_WHITESPACE, SPLIT_WANT_NONEMPTY);
    // HugePages_* only has a number and no suffix so there may not be exactly 3
    // tokens.
    if (tokens.size() <= 1) {
      continue;
    }

    int* target = nullptr;
    if (tokens[0] == "MemTotal:")
      target = &meminfo->total;
    else if (tokens[0] == "MemFree:")
      target = &meminfo->free;
    else if (tokens[0] == "MemAvailable:")
      target = &meminfo->available;
    else if (tokens[0] == "Buffers:")
      target = &meminfo->buffers;
    else if (tokens[0] == "Cached:")
      target = &meminfo->cached;

    if (target)
      StringToInt(tokens[1], target);
  }

  // Make sure the MemTotal is valid.
  return meminfo->total > 0;
}

}  // namespace
namespace system_info {

bool GetSystemMemoryInfo(SystemMemoryInfoKB* meminfo) {
  std::string meminfo_data;
  if (!file_system::read_file_to_string("/proc/meminfo", &meminfo_data)) {
    return false;
  }

  if (!ParseProcMeminfo(meminfo_data, meminfo)) {
    return false;
  }

  return true;
}

Optional<size_t> AmountOfPhysicalMemory() {
  size_t res;
  if (!AmountOfMemory(_SC_PHYS_PAGES, &res)) {
    return Optional<size_t>();
  }

  return res;
}

Optional<size_t> AmountOfAvailablePhysicalMemory() {
  size_t res;
  if (!AmountOfMemory(_SC_AVPHYS_PAGES, &res)) {
    return Optional<size_t>();
  }

  return res;
}

Optional<size_t> AmountOfAvailableRAM() {
  SystemMemoryInfoKB mem;
  if (!GetSystemMemoryInfo(&mem)) {
    return Optional<size_t>();
  }

  return size_t(mem.available) * 1024;
}

Optional<size_t> AmountOfTotalRAM() {
  return AmountOfPhysicalMemory();
}

std::string VirtualizationSystem() {
  return GetVirtualizationSystemAndRole().first;
}

std::string VirtualizationRole() {
  return GetVirtualizationSystemAndRole().second;
}

}  // namespace system_info
}  // namespace common
