/*  Copyright (C) 2014-2021 FastoGT. All right reserved.
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

#include <common/license/hardware_hash.h>

#include <common/hash/md5.h>
#include <common/license/utils.h>

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

bool GenerateHardwareHash(ALGO_TYPE algo, hardware_hash_t* hash) {
  if (!hash) {
    return false;
  }

  const std::string cpu_id = GetNativeCpuID();
  hardware_hash_t mixed;
  mixed[hardware_hash_t::license_size - 1] = algo;

  if (algo == HDD) {
    std::string hdd_id;
    if (!GetHddID(&hdd_id)) {
      return false;
    }

    hardware_hash_t lhash;
    MakeMd5Hash(cpu_id, lhash.data());
    MakeMd5Hash(hdd_id, lhash.data() + hardware_hash_t::license_size / 2);
    for (size_t i = 0; i < lhash.size() / 2; ++i) {
      mixed[i * 2] = lhash[i];
      mixed[i * 2 + 1] = lhash[hardware_hash_t::license_size / 2 + i];
    }
    *hash = mixed;
    return true;
  } else if (algo == MACHINE_ID) {
    std::string system_id;
    if (!GetMachineID(&system_id)) {
      return false;
    }

    hardware_hash_t lhash;
    MakeMd5Hash(cpu_id, lhash.data());
    MakeMd5Hash(system_id, lhash.data() + hardware_hash_t::license_size / 2);
    for (size_t i = 0; i < lhash.size() / 2; ++i) {
      mixed[i * 2] = lhash[i];
      mixed[i * 2 + 1] = lhash[hardware_hash_t::license_size / 2 + i];
    }
    *hash = mixed;
    return true;
  }

  return false;
}

bool IsValidHardwareHash(const hardware_hash_t& hash) {
  hardware_hash_t lic;
  ALGO_TYPE algo = static_cast<ALGO_TYPE>(hash[hardware_hash_t::license_size - 1]);
  if (GenerateHardwareHash(algo, &lic)) {
    if (lic == hash) {
      return true;
    }
  }

  return false;
}

}  // namespace license
}  // namespace common
