/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

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

#include <string>  // for string

#include <common/log_levels.h>  // for LEVEL_LOG::L_ERR, LEVEL_LOG
#include <common/sprintf.h>
#include <common/value.h>  // for Value::ErrorsType, Value, ErrorValue

namespace common {

class ErrnoErrorValue : public ErrorValue {
 public:
  ErrnoErrorValue(const std::string& in_value, ErrorsType errorType, logging::LEVEL_LOG level, int err);
  int GetErrno() const;

 private:
  const int errno_;
};

typedef shared_ptr<ErrorValue> Error;  // if(!err) => not error, if(err && err->IsError()) => error
typedef shared_ptr<ErrnoErrorValue> ErrnoError;

//
Error make_inval_error_value(Value::ErrorsType errorType, logging::LEVEL_LOG level = logging::L_ERR);

Error make_error_value(const std::string& in_value,
                       Value::ErrorsType errorType,
                       logging::LEVEL_LOG level = logging::L_ERR);

ErrnoError make_error_value_errno(int err, Value::ErrorsType errorType, logging::LEVEL_LOG level = logging::L_ERR);

ErrnoError make_error_value_perror(const std::string& function,
                                   int err,
                                   Value::ErrorsType errorType,
                                   logging::LEVEL_LOG level = logging::L_ERR);

}  // namespace common

void DEBUG_MSG_ERROR(common::Error er);
common::ErrnoError DEBUG_MSG_PERROR(const std::string& function, int err, bool notify = true);

template <typename... Args>
common::ErrnoError DEBUG_MSG_PERROR_FORMAT(const char* fmt, int err, Args... args) {
  std::string func_args = common::MemSPrintf(fmt, args...);
  return DEBUG_MSG_PERROR(func_args, err, true);
}
