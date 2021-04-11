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

#include <common/error.h>

#include <errno.h>
#include <string.h>

#include <common/sprintf.h>

namespace common {

#if defined(OS_WIN)
std::string common_strerror(int err) {
  if (err == ECONNRESET) {
    return "Connection reset by peer";
  }

  const char* error_str = strerror(err);
  if (error_str) {
    return error_str;
  }

  return MemSPrintf("Unknown error (%d)", err);
}
#else
std::string common_strerror(int err) {
  const char* error_str = strerror(err);
  if (error_str) {
    return error_str;
  }

  return MemSPrintf("Unknown error (%d)", err);
}
#endif

std::string CommonErrorTraits::GetTextFromErrorCode(CommonErrorCode error) {
  if (error == COMMON_INVALID_INPUT) {
    return "Invalid input argument(s)";
  } else if (error == COMMON_EINTR) {
    return "Interrupted function call";
  }
  return MemSPrintf("Unknown common error code: %d.", static_cast<int>(error));
}

std::string ErrnoTraits::GetTextFromErrorCode(int error) {
  return common_strerror(error);
}

Error make_error_inval() {
  return make_error(COMMON_INVALID_INPUT);
}

Error make_error(CommonErrorCode err) {
  return ErrorValue(err);
}

Error make_error(const std::string& description) {
  return ErrorValue(description);
}

ErrnoError make_errno_error_inval() {
  return make_errno_error(EINVAL);
}

ErrnoError make_errno_error(int err) {
  return ErrnoErrorValue(err);
}

ErrnoError make_errno_error(const std::string& description, int err) {
  return ErrnoErrorValue(description, err);
}

ErrnoError make_error_perror(const std::string& function, int err) {
  const std::string strer = common_strerror(err);
  const std::string descr = function + " : " + strer;
  return make_errno_error(descr, err);
}

Error make_error_from_errno(ErrnoError err) {
  return make_error(err->GetDescription());
}

}  // namespace common

common::ErrnoError DEBUG_MSG_PERROR(const std::string& function,
                                    int err,
                                    common::logging::LOG_LEVEL level,
                                    bool notify) {
  const std::string strer = common::common_strerror(err);
  const std::string descr = function + " : " + strer;

  common::ErrnoError error = common::make_errno_error(err);
  if (notify) {
    RUNTIME_LOG(level) << descr;
  }

  return error;
}
