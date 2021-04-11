/*  Copyright (C) 2014-2021 FastoGT. All right reserved.
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

#include <common/daemon/commands/get_log_info.h>

#include <string>

#define GET_LOG_INFO_PATH_FIELD "path"

namespace common {
namespace daemon {
namespace commands {

GetLogInfo::GetLogInfo() : base_class(), path_() {}

GetLogInfo::GetLogInfo(const url_t& path) : path_(path) {}

common::Error GetLogInfo::SerializeFields(json_object* out) const {
  const std::string path_str = path_.spec();
  json_object_object_add(out, GET_LOG_INFO_PATH_FIELD, json_object_new_string(path_str.c_str()));
  return common::Error();
}

common::Error GetLogInfo::DoDeSerialize(json_object* serialized) {
  GetLogInfo inf;
  json_object* jpath = nullptr;
  json_bool jpath_exists = json_object_object_get_ex(serialized, GET_LOG_INFO_PATH_FIELD, &jpath);
  if (jpath_exists) {
    inf.path_ = url_t(json_object_get_string(jpath));
  }

  *this = inf;
  return common::Error();
}

GetLogInfo::url_t GetLogInfo::GetLogPath() const {
  return path_;
}

}  // namespace commands
}  // namespace daemon
}  // namespace common
