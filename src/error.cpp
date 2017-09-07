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

#include <common/error.h>

#include <common/net/types.h>

namespace common {

#ifdef OS_WIN
const char* common_strerror(int err) {
  if (err == ECONNRESET) {
    return "Connection reset by peer";
  }

  return strerror(err);
}
#else
const char* common_strerror(int err) {
  return strerror(err);
}
#endif

const char* common_strerror(int err, ErrnoType errno_type) {
  if (errno_type == SYSTEM_ERRNO) {
    return common_strerror(err);
  }

  return common_gai_strerror(err);
}

ErrorValue::ErrorValue(const std::string& description, ErrorType error_type)
    : description_(description), error_type_(error_type) {}

bool ErrorValue::IsError() const {
  return error_type_ == NO_ERROR_TYPE;
}

ErrorType ErrorValue::GetErrorType() const {
  return error_type_;
}

const std::string& ErrorValue::GetDescription() const {
  return description_;
}

ErrorValue::~ErrorValue() {}

ErrnoErrorValue::ErrnoErrorValue(int err, ErrnoType errno_type, ErrorType error_type)
    : ErrorValue(common_strerror(err, errno_type), error_type), errno_(err), errno_type_(errno_type) {}

ErrnoErrorValue::ErrnoErrorValue(int err, ErrnoType errno_type, const std::string& description, ErrorType error_type)
    : ErrorValue(description, error_type), errno_(err), errno_type_(errno_type) {}

int ErrnoErrorValue::GetErrno() const {
  return errno_;
}

ErrnoType ErrnoErrorValue::GetErrnoType() const {
  return errno_type_;
}

Error make_inval_error_value(ErrorType error_type) {
  return make_error_value("Invalid input argument(s)", error_type);
}

Error make_error_value(const std::string& description, ErrorType error_type) {
  return std::make_shared<ErrorValue>(description, error_type);
}

ErrnoError make_error_value_errno(int err, ErrnoType errno_type, ErrorType error_type) {
  return std::make_shared<ErrnoErrorValue>(err, errno_type, error_type);
}

ErrnoError make_error_value_errno(int err, ErrnoType errno_type, const std::string& description, ErrorType error_type) {
  return std::make_shared<ErrnoErrorValue>(err, errno_type, description, error_type);
}

ErrnoError make_error_value_perror(const std::string& function, int err, ErrnoType errno_type, ErrorType error_type) {
  const std::string strer = common_strerror(err, errno_type);
  const std::string descr = function + " : " + strer;
  return make_error_value_errno(err, errno_type, descr, error_type);
}

}  // namespace common

void DEBUG_MSG_ERROR(common::Error err, common::logging::LOG_LEVEL level) {
  if (!err) {
    return;
  }

  RUNTIME_LOG(level) << err->GetDescription();
}

common::ErrnoError DEBUG_MSG_PERROR(const std::string& function,
                                    int err,
                                    common::ErrnoType errno_type,
                                    common::logging::LOG_LEVEL level,
                                    bool notify) {
  const std::string strer = common::common_strerror(err, errno_type);
  const std::string descr = function + " : " + strer;

  common::ErrnoError error = common::make_error_value_errno(err, errno_type, descr, common::ERROR_TYPE);
  if (notify) {
    DEBUG_MSG_ERROR(error, level);
  }

  return error;
}
