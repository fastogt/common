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

#include <string>

#include <common/macros.h>

namespace common {
namespace system_info {

#if !defined(OS_MACOSX) && !defined(OS_ANDROID)
std::string OperatingSystemName() {
  struct utsname info;
  if (uname(&info) < 0) {
    NOTREACHED();
    return std::string();
  }

  return std::string(info.sysname);
}
#endif

#if !defined(OS_MACOSX)
std::string OperatingSystemVersion() {
  struct utsname info;
  if (uname(&info) < 0) {
    NOTREACHED();
    return std::string();
  }

  return std::string(info.release);
}
#endif

std::string OperatingSystemArchitecture() {
  struct utsname info;
  if (uname(&info) < 0) {
    NOTREACHED();
    return std::string();
  }
  std::string arch(info.machine);
  if (arch == "i386" || arch == "i486" || arch == "i586" || arch == "i686") {
    arch = "x86";
  } else if (arch == "amd64") {
    arch = "x86_64";
  }
  return arch;
}

}  // namespace system_info
}  // namespace common
