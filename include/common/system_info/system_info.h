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

#include <stdint.h>  // for int64_t
#include <unistd.h>

#include <string>  // for string

#include <common/optional.h>

namespace common {
namespace system_info {

// system
Optional<size_t> AmountOfPhysicalMemory();
Optional<size_t> AmountOfAvailablePhysicalMemory();
Optional<size_t> AmountOfFreeDiskSpace(const std::string& path);
Optional<size_t> AmountOfTotalDiskSpace(const std::string& path);

std::string OperatingSystemName();
std::string OperatingSystemVersion();
std::string OperatingSystemArchitecture();

class SystemInfo {
 public:
  SystemInfo(const std::string& name, const std::string& version, const std::string& arch);

  const std::string& GetName() const;
  const std::string& GetVersion() const;
  const std::string& GetArch() const;

 private:
  const std::string name_;
  const std::string version_;
  const std::string arch_;
};

const SystemInfo& currentSystemInfo();

}  // namespace system_info
}  // namespace common
