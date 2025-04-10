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

#include <common/patterns/singleton_pattern.h>
#include <common/system_info/system_info.h>

namespace common {
namespace system_info {

namespace {

struct CurrentSystemInfo {
  CurrentSystemInfo() : info(OperatingSystemName(), OperatingSystemVersion(), OperatingSystemArchitecture()) {}

  const SystemInfo info;

  static CurrentSystemInfo* GetInstance() { return &patterns::LazySingleton<CurrentSystemInfo>::GetInstance(); }
};

}  // namespace

#if !defined(OS_LINUX)
std::string VirtualizationSystem() {
  return std::string();
}

std::string VirtualizationRole() {
  return std::string();
}
#endif

SystemInfo::SystemInfo(const std::string& name, const std::string& version, const std::string& arch)
    : name_(name), version_(version), arch_(arch) {}

const std::string& SystemInfo::GetName() const {
  return name_;
}

const std::string& SystemInfo::GetVersion() const {
  return version_;
}

const std::string& SystemInfo::GetArch() const {
  return arch_;
}

const SystemInfo& currentSystemInfo() {
  return CurrentSystemInfo::GetInstance()->info;
}

}  // namespace system_info
}  // namespace common
