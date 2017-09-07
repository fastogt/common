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

#pragma once

#include <memory>

#include <common/sprintf.h>

namespace common {

enum ErrorType { NO_ERROR_TYPE, EXCEPTION_TYPE, ERROR_TYPE, INTERRUPTED_TYPE };
enum ErrnoType { SYSTEM_ERRNO, NETWORK_ERRNO };

const char* common_strerror(int err);
extern const char* common_gai_strerror(int err);
const char* common_strerror(int err, ErrnoType errno_type);

class ErrorValue {
 public:
  ErrorValue(const std::string& description, ErrorType error_type);

  bool IsError() const;

  ErrorType GetErrorType() const;
  const std::string& GetDescription() const;

  virtual ~ErrorValue();

 private:
  const std::string description_;
  const ErrorType error_type_;
  DISALLOW_COPY_AND_ASSIGN(ErrorValue);
};

class ErrnoErrorValue : public ErrorValue {
 public:
  ErrnoErrorValue(int err, ErrnoType errno_type, ErrorType error_type);
  ErrnoErrorValue(int err, ErrnoType errno_type, const std::string& description, ErrorType error_type);

  int GetErrno() const;
  ErrnoType GetErrnoType() const;

 private:
  const int errno_;
  const ErrnoType errno_type_;
};

typedef std::shared_ptr<ErrorValue> Error;  // if(!err) => no error, if(err && err->IsError()) => error
typedef std::shared_ptr<ErrnoErrorValue> ErrnoError;

//
Error make_inval_error_value(ErrorType error_type);

Error make_error_value(const std::string& description, ErrorType error_type);

ErrnoError make_error_value_errno(int err, ErrnoType errno_type, ErrorType error_type);

ErrnoError make_error_value_errno(int err, ErrnoType errno_type, const std::string& description, ErrorType error_type);

ErrnoError make_error_value_perror(const std::string& function, int err, ErrnoType errno_type, ErrorType error_type);

}  // namespace common

void DEBUG_MSG_ERROR(common::Error err, common::logging::LOG_LEVEL level);
common::ErrnoError DEBUG_MSG_PERROR(const std::string& function,
                                    int err,
                                    common::ErrnoType errno_type,
                                    common::logging::LOG_LEVEL level,
                                    bool notify = true);

template <typename... Args>
common::ErrnoError DEBUG_MSG_PERROR_FORMAT(const char* fmt,
                                           int err,
                                           common::ErrnoType errno_type,
                                           common::logging::LOG_LEVEL level,
                                           Args... args) {
  const std::string func_args = common::MemSPrintf(fmt, args...);
  return DEBUG_MSG_PERROR(func_args, err, errno_type, level, true);
}
