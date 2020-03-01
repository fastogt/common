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

#pragma once

#include <string>

#define LICENSE_KEY_LENGHT 64
#define EXPIRE_KEY_LENGHT 96

namespace common {
namespace license {

typedef char license_key_t[LICENSE_KEY_LENGHT];
typedef char expire_key_t[EXPIRE_KEY_LENGHT];

std::string licensekey2string(license_key_t from);
bool string2licensekey(const std::string& from, license_key_t out);

std::string expirekey2string(expire_key_t from);
bool string2expirekey(const std::string& from, expire_key_t out);

}  // namespace license
}  // namespace common
