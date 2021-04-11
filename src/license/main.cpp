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

#include <string.h>

#include <iostream>

#include <common/license/hardware_hash.h>

#define HELP_TEXT                        \
  "Usage: license_gen"                   \
  " [options]\n"                         \
  "  License generation for license_gen" \
  " project.\n\n"                        \
  "    --hdd [default] hash algorithm\n" \
  "    --machine-id hash algorithm\n"

int main(int argc, char** argv) {
  common::license::ALGO_TYPE algo = common::license::HDD;
  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "--hdd") == 0) {
      algo = common::license::HDD;
    } else if (strcmp(argv[i], "--machine-id") == 0) {
      algo = common::license::MACHINE_ID;
    } else if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
      std::cout << HELP_TEXT << std::endl;
      return EXIT_SUCCESS;
    } else {
      std::cout << HELP_TEXT << std::endl;
      return EXIT_SUCCESS;
    }
  }

  common::license::hardware_hash_t hash;
  if (!common::license::GenerateHardwareHash(algo, &hash)) {
    return EXIT_FAILURE;
  }

  if (!common::license::IsValidHardwareHash(hash)) {
    return EXIT_FAILURE;
  }

  std::cout << std::string(hash.data(), hash.size()) << std::endl;
  return EXIT_SUCCESS;
}
