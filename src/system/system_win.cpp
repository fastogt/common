#include "common/system/system.h"

#include <windows.h>

namespace common {
namespace system {

Error Shutdown(shutdown_t type) {
  HANDLE hToken = NULL;

  bool is_ok = OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);
  if (!is_ok) {
    return make_error_value_perror("OpenProcessToken", errno, SYSTEM_ERRNO, ERROR_TYPE);
  }

  TOKEN_PRIVILEGES tkp;
  tkp.PrivilegeCount = 1;  // set 1 privilege
  tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

  is_ok = LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
  if (!is_ok) {
    return make_error_value_perror("LookupPrivilegeValue", errno, SYSTEM_ERRNO, ERROR_TYPE);
  }

  // get the shutdown privilege for this process
  is_ok = AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
  if (!is_ok) {
    return make_error_value_perror("AdjustTokenPrivileges", errno, SYSTEM_ERRNO, ERROR_TYPE);
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
      return make_error_value_perror("systemShutdown", EINVAL, SYSTEM_ERRNO, ERROR_TYPE);
  }

  return Error();
}
}
}
