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

#pragma once

#include <algorithm>
#include <array>
#include <string>

#include <common/optional.h>

namespace common {
namespace license {

template <size_t N>
class License : public std::array<char, N> {
 public:
  typedef std::array<char, N> base_class;
  enum : size_t { license_size = N };

  std::string as_string() const { return std::string(base_class::data(), N); }
};

typedef License<65> hardware_hash_t;
typedef License<97> expire_key_t;

template <typename Lic>
Optional<Lic> make_license(const std::string& data) {
  if (data.size() != Lic::license_size) {
    return Optional<Lic>();
  }

  Lic lic;
  std::copy(data.begin(), data.end(), lic.data());
  return lic;
}

}  // namespace license
}  // namespace common
