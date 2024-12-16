/*  Copyright (C) 2014-2022 FastoGT. All right reserved.
    This file is part of fastocloud.
    fastocloud is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    fastocloud is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with fastocloud.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <common/daemon/commands/get_hhash_info.h>

#include <string>

#define GET_ALGO_FIELD "algo"

namespace common {
namespace daemon {
namespace commands {

GetHardwareHashInfo::GetHardwareHashInfo() : base_class(), algo_(license::HDD) {}

GetHardwareHashInfo::GetHardwareHashInfo(algo_t algo) : algo_(algo) {}

common::Error GetHardwareHashInfo::SerializeFields(json_object* out) const {
  json_object_object_add(out, GET_ALGO_FIELD, json_object_new_int64(algo_));
  return common::Error();
}

common::Error GetHardwareHashInfo::DoDeSerialize(json_object* serialized) {
  GetHardwareHashInfo inf;
  json_object* jalgo = nullptr;
  json_bool jalgo_exists = json_object_object_get_ex(serialized, GET_ALGO_FIELD, &jalgo);
  if (!jalgo_exists) {
    return make_error_inval();
  }

  inf.algo_ = static_cast<algo_t>(json_object_get_int64(jalgo));
  *this = inf;
  return common::Error();
}

GetHardwareHashInfo::algo_t GetHardwareHashInfo::GetAlgoType() const {
  return algo_;
}

}  // namespace commands
}  // namespace daemon
}  // namespace common
