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

#pragma once

#include <common/optional.h>

namespace common {

std::string common_strerror(int err);
extern std::string common_gai_strerror(int err);

template <typename T, typename trait>
class ErrorBase {
 public:
  typedef T error_code_type;
  typedef trait trait_type;
  typedef void payload_type;

  explicit ErrorBase(error_code_type error_code) : error_code_(error_code), user_data_(nullptr) {}
  std::string GetDescription() const { return trait_type::GetTextFromErrorCode(error_code_); }
  error_code_type GetErrorCode() const { return error_code_; }

  void SetPayload(payload_type* user_data) { user_data_ = user_data; }

  payload_type* GetPayload() const { return user_data_; }

 private:
  error_code_type error_code_;
  payload_type* user_data_;
};

// common error
enum CommonErrorCode { COMMON_INVALID_INPUT = -1, COMMON_TEXT_ERROR = -2, COMMON_EINTR = -3 };
struct CommonErrorTraits {
  static std::string GetTextFromErrorCode(CommonErrorCode error);
};

class ErrorValue : public ErrorBase<CommonErrorCode, CommonErrorTraits> {
 public:
  typedef ErrorBase<CommonErrorCode, CommonErrorTraits> base_class;
  explicit ErrorValue(CommonErrorCode error_code) : base_class(error_code), text_error_description_() {}
  explicit ErrorValue(const std::string& description)
      : base_class(COMMON_TEXT_ERROR), text_error_description_(description) {}

  bool IsTextError() const { return GetErrorCode() == COMMON_TEXT_ERROR; }

  std::string GetDescription() const {
    if (IsTextError()) {
      return text_error_description_;
    }
    return base_class::trait_type::GetTextFromErrorCode(GetErrorCode());
  }

 private:
  std::string text_error_description_;
};
typedef Optional<ErrorValue> Error;

Error make_error_inval();
Error make_error(CommonErrorCode err);
Error make_error(const std::string& description);

// errno error
struct ErrnoTraits {
  static std::string GetTextFromErrorCode(int error);
};

class ErrnoErrorValue : public ErrorBase<int, ErrnoTraits> {
 public:
  typedef ErrorBase<int, ErrnoTraits> base_class;
  explicit ErrnoErrorValue(int error_code) : base_class(error_code), text_error_description_() {}
  ErrnoErrorValue(const std::string& description, int error_code)
      : base_class(error_code), text_error_description_(description) {}

  std::string GetDescription() const {
    if (!text_error_description_.empty()) {
      return text_error_description_;
    }
    return base_class::trait_type::GetTextFromErrorCode(GetErrorCode());
  }

 private:
  std::string text_error_description_;
};

typedef Optional<ErrnoErrorValue> ErrnoError;

ErrnoError make_errno_error_inval();
ErrnoError make_errno_error(int err);
ErrnoError make_errno_error(const std::string& description, int err);
ErrnoError make_error_perror(const std::string& function, int err);
Error make_error_from_errno(ErrnoError err);

}  // namespace common

template <typename T>
inline void DEBUG_MSG_ERROR(common::Optional<T> err, common::logging::LOG_LEVEL level) {
  if (!err) {
    return;
  }

  RUNTIME_LOG(level) << err->GetDescription();
}

common::ErrnoError DEBUG_MSG_PERROR(const std::string& function,
                                    int err,
                                    common::logging::LOG_LEVEL level,
                                    bool notify = true);
