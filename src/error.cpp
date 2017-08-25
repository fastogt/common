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

namespace common {

ErrnoErrorValue::ErrnoErrorValue(const std::string& in_value, ErrorsType errorType, logging::LEVEL_LOG level, int err)
    : ErrorValue(in_value, errorType, level), errno_(err) {}

int ErrnoErrorValue::GetErrno() const {
  return errno_;
}

Error make_inval_error_value(Value::ErrorsType errorType, logging::LEVEL_LOG level) {
  return make_error_value("Invalid input argument(s)", errorType, level);
}

Error make_error_value(const std::string& in_value, Value::ErrorsType errorType, logging::LEVEL_LOG level) {
  ErrorValue* err = Value::CreateErrorValue(in_value, errorType, level);
  return Error(err);
}

ErrnoError make_error_value_errno(int err, Value::ErrorsType errorType, logging::LEVEL_LOG level) {
  const char* strer = common_strerror(err);
  ErrnoErrorValue* error = new ErrnoErrorValue(strer, errorType, level, err);
  return ErrnoError(error);
}

ErrnoError make_error_value_perror(const std::string& function,
                                   int err,
                                   Value::ErrorsType errorType,
                                   logging::LEVEL_LOG level) {
  std::string strer = common_strerror(err);
  std::string descr = function + " : " + strer;
  ErrnoError error = make_error_value_errno(err, errorType, level);
  error->SetDescription(descr);
  return error;
}

}  // namespace common

void DEBUG_MSG_ERROR(common::Error er) {
  if (!er) {
    return;
  }

  common::logging::LEVEL_LOG level = er->GetLevel();
  RUNTIME_LOG(level) << er->GetDescription();
}

common::ErrnoError DEBUG_MSG_PERROR(const std::string& function, int err, bool notify) {
  std::string strer = common::common_strerror(err);
  std::string descr = function + " : " + strer;

  common::ErrnoError error = common::make_error_value_errno(err, common::Value::E_ERROR, common::logging::L_ERR);
  error->SetDescription(descr);
  if (notify) {
    DEBUG_MSG_ERROR(error);
  }

  return error;
}
