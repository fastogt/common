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

#include <common/system_info/cpu_info.h>
#include <common/system_info/system_info.h>

#define HELP_TEXT "Usage: system_info [options]\n"

int main(int argc, char** argv) {
  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "--help") == 0 || strcmp(argv[i], "-h") == 0) {
      std::cout << HELP_TEXT << std::endl;
      return EXIT_SUCCESS;
    } else {
      std::cout << HELP_TEXT << std::endl;
      return EXIT_SUCCESS;
    }
  }

  size_t ram_bytes_total = 0;
  const auto total = common::system_info::AmountOfPhysicalMemory();
  if (total) {
    ram_bytes_total = *total;
  }

  size_t ram_bytes_free = 0;
  const auto avail = common::system_info::AmountOfAvailablePhysicalMemory();
  if (avail) {
    ram_bytes_free = *avail;
  }

  size_t hdd_bytes_total = 0;
  const auto hdd_total = common::system_info::AmountOfTotalDiskSpace("/");
  if (hdd_total) {
    hdd_bytes_total = *hdd_total;
  }

  size_t hdd_bytes_free = 0;
  const auto hdd_avail = common::system_info::AmountOfFreeDiskSpace("/");
  if (hdd_avail) {
    hdd_bytes_free = *hdd_avail;
  }

  static const std::string name = common::system_info::OperatingSystemName();
  static const std::string version = common::system_info::OperatingSystemVersion();
  static const std::string arch = common::system_info::OperatingSystemArchitecture();
  static const auto cpu = common::system_info::CPU();

  std::cout << "Operation system: " << name << std::endl;
  std::cout << "Version: " << version << std::endl;
  std::cout << "Architecture: " << arch << std::endl;
  std::cout << "Vendor: " << cpu.vendor_name() << std::endl;
  std::cout << "Cpu brand: " << cpu.cpu_brand() << std::endl;
  std::cout << "RAM bytes total: " << ram_bytes_total << std::endl;
  std::cout << "RAM bytes available: " << ram_bytes_free << std::endl;
  std::cout << "HDD bytes total: " << hdd_bytes_total << std::endl;
  std::cout << "HDD bytes available: " << hdd_bytes_free << std::endl;
  return EXIT_SUCCESS;
}
