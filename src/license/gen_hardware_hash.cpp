/*  Copyright (C) 2014-2018 FastoGT. All right reserved.
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

#include <common/convert2string.h>
#include <common/hash/md5.h>

#include <common/license/utils.h>

namespace common {
namespace {
bool MakeMd5Hash(const std::string& data, std::string* out) {
  if (!out) {
    return false;
  }

  hash::MD5_CTX md5;
  hash::MD5_Init(&md5);
  const unsigned char* cdata = reinterpret_cast<const unsigned char*>(data.c_str());
  hash::MD5_Update(&md5, cdata, data.size());
  unsigned char md5_result[16];
  hash::MD5_Final(&md5, md5_result);
  std::string hs(std::begin(md5_result), std::end(md5_result));
  return utils::hex::encode(hs, true, out);
}
}  // namespace
namespace license {

bool GenerateHardwareHash(ALGO_TYPE t, std::string* hash) {
  if (!hash) {
    return false;
  }

  const std::string cpu_id = GetNativeCpuID();
  if (t == HDD) {
    std::string hdd_id;
    if (!GetHddID(&hdd_id)) {
      return false;
    }

    std::string cpu_id_hash;
    MakeMd5Hash(cpu_id, &cpu_id_hash);
    std::string hdd_id_hash;
    MakeMd5Hash(hdd_id, &hdd_id_hash);
    *hash = cpu_id_hash + hdd_id_hash;
    return true;
  } else if (t == MACHINE_ID) {
    std::string system_id;
    if (!GetMachineID(&system_id)) {
      return false;
    }

    std::string cpu_id_hash;
    MakeMd5Hash(cpu_id, &cpu_id_hash);
    std::string machine_id_hash;
    MakeMd5Hash(system_id, &machine_id_hash);
    *hash = cpu_id_hash + machine_id_hash;
    return true;
  }

  NOTREACHED() << "Unknown algo: " << t;
  return false;
}

}  // namespace license
}  // namespace common
