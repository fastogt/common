/*  Copyright (C) 2014-2020 FastoGT. All right reserved.
    This file is part of iptv_cloud.
    iptv_cloud is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    iptv_cloud is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with iptv_cloud.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <common/license/types.h>

namespace common {
namespace license {

bool is_valid_license_key(const std::string& key) {
  return key.size() == LICENSE_KEY_LENGHT;
}

std::string licensekey2string(license_key_t from) {
  return std::string(from, LICENSE_KEY_LENGHT);
}

bool string2licensekey(const std::string& from, common::license::license_key_t out) {
  if (!is_valid_license_key(from)) {
    return false;
  }

  for (size_t i = 0; i < LICENSE_KEY_LENGHT; ++i) {
    out[i] = from[i];
  }
  return true;
}

bool is_valid_expire_key(const std::string& key) {
  return key.size() == EXPIRE_KEY_LENGHT;
}

std::string expirekey2string(expire_key_t from) {
  return std::string(from, EXPIRE_KEY_LENGHT);
}

bool string2expirekey(const std::string& from, common::license::expire_key_t out) {
  if (!is_valid_expire_key(from)) {
    return false;
  }

  for (size_t i = 0; i < EXPIRE_KEY_LENGHT; ++i) {
    out[i] = from[i];
  }
  return true;
}

}  // namespace license
}  // namespace common
