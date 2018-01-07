/*  Copyright (C) 2014-2018 FastoGT. All right reserved.

    This file is part of FastoTV.

    FastoTV is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    FastoTV is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with FastoTV. If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <common/error.h>

namespace common {
namespace serializer {

template <typename S = std::string>
class ISerializer {
 public:
  typedef S serialize_type;

  virtual ~ISerializer() {}

  Error Serialize(serialize_type* deserialized) const WARN_UNUSED_RESULT {
    if (!deserialized) {
      return make_error_inval();
    }
    return SerializeImpl(deserialized);
  }

  virtual Error SerializeFromString(const std::string& data, serialize_type* out) const WARN_UNUSED_RESULT = 0;

  virtual Error SerializeToString(std::string* deserialized) const WARN_UNUSED_RESULT = 0;

 protected:
  virtual Error SerializeImpl(serialize_type* deserialized) const = 0;
};

}  // namespace serializer
}  // namespace common
