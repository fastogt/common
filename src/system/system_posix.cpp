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

#include <unistd.h>  // for sync

#define SYSTEM_CALL 1
#define REBOOT_CALL 0

#define POWEROFF_STRATEGY SYSTEM_CALL

#if POWEROFF_STRATEGY == REBOOT_CALL
#if defined(OS_LINUX)
#define OS_SHUTDOWN RB_POWER_OFF
#define OS_LOGOUT RB_POWER_OFF
#define OS_REBOOT RB_AUTOBOOT
#elif defined(OS_FREEBSD)
#define OS_SHUTDOWN RB_POWEROFF
#define OS_LOGOUT RB_POWEROFF
#define OS_REBOOT RB_AUTOBOOT
#endif
#endif

namespace common {
namespace system {

#if !defined(OS_MACOSX) && !defined(OS_ANDROID)
ErrnoError Shutdown(shutdown_t type) {
  sync();
#if POWEROFF_STRATEGY == REBOOT_CALL
  int howto = 0;
  if (type == SHUTDOWN) {
    howto = OS_SHUTDOWN;
  } else if (type == LOGOUT) {
    howto = OS_LOGOUT;
  } else if (type == REBOOT) {
    howto = OS_REBOOT;
  } else {
    return make_error_perror("Shutdown", EINVAL, ERROR_TYPE);
  }
  int res = reboot(howto);
  if (res == ERROR_RESULT_VALUE) {
    return make_error_perror("reboot", errno, ERROR_TYPE);
  }
#else
  int res = SUCCESS_RESULT_VALUE;
  if (type == SHUTDOWN) {
    res = ::system("poweroff");
  } else if (type == LOGOUT) {
    res = ::system("logout");
  } else if (type == REBOOT) {
    res = ::system("reboot");
  } else {
    return make_error_perror("Shutdown", EINVAL, ERROR_TYPE);
  }

  if (res == ERROR_RESULT_VALUE) {
    return make_error_perror("system", errno, ERROR_TYPE);
  }
#endif
  return ErrnoError();
}
#endif

}  // namespace system
}  // namespace common
