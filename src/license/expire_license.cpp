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

#include <common/license/expire_license.h>

#include <common/license/hardware_hash.h>

#include <common/convert2string.h>

namespace common {
namespace license {

bool IsValidExpireKey(const std::string& project,
                      const common::license::hardware_hash_t& license_key,
                      const common::license::expire_key_t& expire_key) {
  time64_t res;
  bool is_ok = GetExpireTimeFromKey(project, license_key, expire_key, &res);
  if (!is_ok) {
    return false;
  }

  time64_t utc_msec = time::current_utc_mstime();
  if (utc_msec > res) {
    return false;
  }
  return true;
}

bool GetExpireTimeFromKey(const std::string& project,
                          const common::license::hardware_hash_t& license_key,
                          const common::license::expire_key_t& expire_key,
                          time64_t* time) {
  if (project.empty() || !time) {
    return false;
  }

  if (!IsValidHardwareHash(license_key)) {
    return false;
  }

  uint64_t crc = 0;
  for (size_t i = 0; i < project.size(); ++i) {
    crc += project[i];
  }

  std::string res_str;
  std::string project_calc_str;
  for (size_t i = 0; i < sizeof(time64_t); ++i) {
    res_str += expire_key[i * 12];
    res_str += expire_key[i * 12 + 11];

    project_calc_str += expire_key[i * 12 + 1];
    project_calc_str += expire_key[i * 12 + 10];
  }

  uint64_t project_calc;
  if (!common::HexStringToUInt64(project_calc_str, &project_calc) || crc != project_calc) {
    return false;
  }

  time64_t res;
  if (!common::HexStringToInt64(res_str, &res)) {
    return false;
  }

  *time = res;
  return true;
}

}  // namespace license
}  // namespace common
