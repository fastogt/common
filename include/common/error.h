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

#include <common/sprintf.h>
#include <common/value.h>  // for Value::ErrorsType, Value, ErrorValue

namespace common {

enum ErrnoType { SYSTEM_ERRNO, NETWORK_ERRNO };

const char* common_strerror(int err);
extern const char* common_gai_strerror(int err);
const char* common_strerror(int err, ErrnoType errno_type);

class ErrnoErrorValue : public ErrorValue {
 public:
  ErrnoErrorValue(int err, ErrnoType errno_type, ErrorsType error_type, logging::LOG_LEVEL level);
  ErrnoErrorValue(int err,
                  ErrnoType errno_type,
                  const std::string& description,
                  ErrorsType error_type,
                  logging::LOG_LEVEL level);

  int GetErrno() const;
  ErrnoType GetErrnoType() const;

 private:
  const int errno_;
  const ErrnoType errno_type_;
};

typedef std::shared_ptr<ErrorValue> Error;  // if(!err) => no error, if(err && err->IsError()) => error
typedef std::shared_ptr<ErrnoErrorValue> ErrnoError;

//
Error make_inval_error_value(Value::ErrorsType error_type, logging::LOG_LEVEL level = logging::LOG_LEVEL_ERR);

Error make_error_value(const std::string& description,
                       Value::ErrorsType error_type,
                       logging::LOG_LEVEL level = logging::LOG_LEVEL_ERR);

ErrnoError make_error_value_errno(int err,
                                  ErrnoType errno_type,
                                  Value::ErrorsType error_type,
                                  logging::LOG_LEVEL level = logging::LOG_LEVEL_ERR);

ErrnoError make_error_value_errno(int err,
                                  ErrnoType errno_type,
                                  const std::string& description,
                                  Value::ErrorsType error_type,
                                  logging::LOG_LEVEL level = logging::LOG_LEVEL_ERR);

ErrnoError make_error_value_perror(const std::string& function,
                                   int err,
                                   ErrnoType errno_type,
                                   Value::ErrorsType error_type,
                                   logging::LOG_LEVEL level = logging::LOG_LEVEL_ERR);

}  // namespace common

void DEBUG_MSG_ERROR(common::Error err);
common::ErrnoError DEBUG_MSG_PERROR(const std::string& function,
                                    int err,
                                    common::ErrnoType errno_type,
                                    bool notify = true);

template <typename... Args>
common::ErrnoError DEBUG_MSG_PERROR_FORMAT(const char* fmt, int err, common::ErrnoType errno_type, Args... args) {
  const std::string func_args = common::MemSPrintf(fmt, args...);
  return DEBUG_MSG_PERROR(func_args, err, errno_type, true);
}
