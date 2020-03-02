/*  Copyright (C) 2014-2020 FastoGT. All right reserved.

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

#include <common/daemon/commands/license_info.h>

#define LICENSE_INFO_KEY_FIELD "license_key"

namespace common {
namespace daemon {
namespace commands {

LicenseInfo::LicenseInfo() : base_class(), license_() {}

LicenseInfo::LicenseInfo(const std::string& license) : base_class(), license_(license) {}

common::Error LicenseInfo::SerializeFields(json_object* out) const {
  if (!IsValid()) {
    return make_error_inval();
  }

  json_object_object_add(out, LICENSE_INFO_KEY_FIELD, json_object_new_string(license_.c_str()));
  return Error();
}

bool LicenseInfo::IsValid() const {
  return !license_.empty();
}

common::Error LicenseInfo::DoDeSerialize(json_object* serialized) {
  LicenseInfo inf;
  json_object* jlicense = nullptr;
  json_bool jlicense_exists = json_object_object_get_ex(serialized, LICENSE_INFO_KEY_FIELD, &jlicense);
  if (!jlicense_exists) {
    return make_error_inval();
  }

  const std::string license = json_object_get_string(jlicense);
  if (license.empty()) {
    return make_error_inval();
  }

  inf.license_ = license;
  *this = inf;
  return common::Error();
}

std::string LicenseInfo::GetLicense() const {
  return license_;
}

}  // namespace commands
}  // namespace daemon
}  // namespace common
