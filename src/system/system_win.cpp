#include "common/system/system.h"

#include <windows.h>

namespace common {
namespace system {

ErrnoError Shutdown(shutdown_t type) {
  HANDLE hToken = NULL;

  bool is_ok = OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);
  if (!is_ok) {
    return make_error_perror("OpenProcessToken", errno);
  }

  TOKEN_PRIVILEGES tkp;
  tkp.PrivilegeCount = 1;  // set 1 privilege
  tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

  is_ok = LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
  if (!is_ok) {
    return make_error_perror("LookupPrivilegeValue", errno);
  }

  // get the shutdown privilege for this process
  is_ok = AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
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
}
}
