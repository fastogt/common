#pragma once

#include <common/error.h>

namespace common {
namespace system {

enum shutdown_t { SHUTDOWN, LOGOUT, REBOOT };

ErrnoError Shutdown(shutdown_t type) WARN_UNUSED_RESULT;

}  // namespace system
}  // namespace common
