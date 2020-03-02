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

#include <common/license/gen_hardware_hash.h>

#include <common/sys_byteorder.h>
#include <memory.h>

#include <common/convert2string.h>
#include <common/hash/md5.h>
#include <common/license/utils.h>
#include <common/macros.h>
#include <common/time.h>

#define HALF_LICENSE_KEY_LENGHT LICENSE_KEY_LENGHT / 2

namespace common {
namespace license {

namespace {
char to_hex(char code) {
  static char hex[] = "0123456789abcdef";
  return hex[code & 15];
}

bool MakeMd5Hash(const std::string& data, char* out) {
  if (!out) {
    return false;
  }

  hash::MD5_CTX md5;
  hash::MD5_Init(&md5);
  const unsigned char* cdata = reinterpret_cast<const unsigned char*>(data.c_str());
  hash::MD5_Update(&md5, cdata, data.size());
  unsigned char md5_result[MD5_HASH_LENGHT];
  hash::MD5_Final(&md5, md5_result);
  for (size_t i = 0; i < MD5_HASH_LENGHT; ++i) {
    out[i * 2] = to_hex(md5_result[i] >> 4);
    out[i * 2 + 1] = to_hex(md5_result[i] & 15);
  }
  return true;
}
}  // namespace

bool GenerateHardwareHash(ALGO_TYPE algo, common::license::license_key_t hash) {
  if (!hash) {
    return false;
  }

  const std::string cpu_id = GetNativeCpuID();
  if (algo == HDD) {
    std::string hdd_id;
    if (!GetHddID(&hdd_id)) {
      return false;
    }

    MakeMd5Hash(cpu_id, hash);
    MakeMd5Hash(hdd_id, hash + HALF_LICENSE_KEY_LENGHT);
    return true;
  } else if (algo == MACHINE_ID) {
    std::string system_id;
    if (!GetMachineID(&system_id)) {
      return false;
    }

    MakeMd5Hash(cpu_id, hash);
    MakeMd5Hash(system_id, hash + HALF_LICENSE_KEY_LENGHT);
    return true;
  }

  NOTREACHED() << "Unknown algo: " << algo;
  return false;
}

Error CheckExpireKey(const std::string& project, const license_key_t license_key, const expire_key_t expire_key) {
  time64_t res;
  Error err = GetExpireTimeFromKey(project, license_key, expire_key, &res);
  if (err) {
    return err;
  }

  time64_t utc_msec = time::current_utc_mstime();
  if (utc_msec > res) {
    return make_error("License time expired");
  }
  return Error();
}

Error GetExpireTimeFromKey(const std::string& project,
                           const license_key_t license_key,
                           const expire_key_t expire_key,
                           time64_t* time) {
  if (project.empty() || !time) {
    return make_error_inval();
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
    return make_error("Invalid project");
  }

  time64_t res;
  if (!common::HexStringToInt64(res_str, &res)) {
    return make_error("Invalid time");
  }

  *time = res;
  return Error();
}

}  // namespace license
}  // namespace common
