#include "common/system/system.h"

#include <windows.h>

namespace common {
namespace system {

Error Shutdown(shutdown_t type) {
  HANDLE hToken = nullptr;

  bool isOk = OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken);
  if (!isOk) {
    return make_error_value_perror("OpenProcessToken", errno, ErrorValue::E_ERROR);
  }

  TOKEN_PRIVILEGES tkp;
  tkp.PrivilegeCount = 1;  // set 1 privilege
  tkp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;

  isOk = LookupPrivilegeValue(NULL, SE_SHUTDOWN_NAME, &tkp.Privileges[0].Luid);
  if (!isOk) {
    return make_error_value_perror("LookupPrivilegeValue", errno, ErrorValue::E_ERROR);
  }

  // get the shutdown privilege for this process
  isOk = AdjustTokenPrivileges(hToken, FALSE, &tkp, 0, (PTOKEN_PRIVILEGES)NULL, 0);
  if (!isOk) {
    return make_error_value_perror("AdjustTokenPrivileges", errno, ErrorValue::E_ERROR);
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
      return make_error_value_perror("systemShutdown", EINVAL, ErrorValue::E_ERROR);
  }

  return Error();
}
}
}
