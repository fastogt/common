/*  Copyright (C) 2014-2022 FastoGT. All right reserved.

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

#include <common/daemon/commands/hhash_info.h>

#define PROJECT_FIELD "project"
#define LICENSE_INFO_KEY_FIELD "key"

namespace common {
namespace daemon {
namespace commands {

Project::Project() : base_class(), project_() {}

Project::Project(project_t proj) : base_class(), project_(proj) {}

Project::project_t Project::GetProject() const {
  return project_;
}

bool Project::IsValid() const {
  if (!project_.empty()) {
    return true;
  }
  return false;
}

common::Error Project::SerializeFields(json_object* out) const {
  if (!IsValid()) {
    return make_error_inval();
  }

  json_object_object_add(out, PROJECT_FIELD, json_object_new_string_len(project_.data(), project_.size()));
  return Error();
}

common::Error Project::DoDeSerialize(json_object* serialized) {
  Project inf;
  json_object* jproj = nullptr;
  json_bool jproj_exists = json_object_object_get_ex(serialized, PROJECT_FIELD, &jproj);
  if (!jproj_exists) {
    return make_error_inval();
  }

  inf.project_ = json_object_get_string(jproj);
  *this = inf;
  return common::Error();
}

HardwareHashProject::HardwareHashProject() : base_class(), license_() {}

HardwareHashProject::HardwareHashProject(project_t proj, license_t license) : base_class(proj), license_(license) {}

common::Error HardwareHashProject::SerializeFields(json_object* out) const {
  if (!IsValid()) {
    return make_error_inval();
  }

  common::Error err = base_class::SerializeFields(out);
  if (err) {
    return err;
  }

  const auto license_str = *license_;
  json_object_object_add(out, LICENSE_INFO_KEY_FIELD,
                         json_object_new_string_len(license_str.data(), license_str.size()));
  return common::Error();
}

bool HardwareHashProject::IsValid() const {
  if (base_class::IsValid() && license_) {
    return true;
  }
  return false;
}

common::Error HardwareHashProject::DoDeSerialize(json_object* serialized) {
  HardwareHashProject inf;
  common::Error err = inf.base_class::DoDeSerialize(serialized);
  if (err) {
    return err;
  }

  json_object* jlicense = nullptr;
  json_bool jlicense_exists = json_object_object_get_ex(serialized, LICENSE_INFO_KEY_FIELD, &jlicense);
  if (!jlicense_exists) {
    return make_error_inval();
  }

  const std::string license = json_object_get_string(jlicense);
  const auto lic = common::license::make_license<raw_license_t>(license);
  if (!lic) {
    return make_error_inval();
  }

  inf.license_ = lic;
  *this = inf;
  return common::Error();
}

HardwareHashProject::license_t HardwareHashProject::GetLicense() const {
  return license_;
}

}  // namespace commands
}  // namespace daemon
}  // namespace common
