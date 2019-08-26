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

#include <common/system/system.h>

#include <windows.h>

namespace common {
namespace system {

ErrnoError Shutdown(shutdown_t type) {
  HANDLE hToken = nullptr;

  bool is_ok = OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);
  if (!is_ok) {
    return make_error_perror("OpenProcessToken", errno);
  }

  TOKEN_PRIVILEGES tkp;
  tkp.PrivilegeCount = 1;  // set 1 privilege
  tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

  is_ok = LookupPrivilegeValue(nullptr, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
  if (!is_ok) {
    return make_error_perror("LookupPrivilegeValue", errno);
  }

  // get the shutdown privilege for this process
  is_ok = AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, nullptr, nullptr);
  if (!is_ok) {
    return make_error_perror("AdjustTokenPrivileges", errno);
  }

  switch (type) {
    case SHUTDOWN:
      ::ExitWindowsEx(EWX_SHUTDOWN | EWX_FORCE, 0);
      break;
    case LOGOUT:
      ::ExitWindowsEx(EWX_POWEROFF | EWX_FORCE, 0);
      break;
    case REBOOT:
      ::ExitWindowsEx(EWX_REBOOT | EWX_FORCE, 0);
      break;
    default:
      return make_error_perror("systemShutdown", EINVAL);
  }

  return ErrnoError();
}
}  // namespace system
}  // namespace common
