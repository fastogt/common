/*  Copyright (C) 2014-2022 FastoGT. All right reserved.
    This file is part of fastocloud.
    fastocloud is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.
    fastocloud is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.
    You should have received a copy of the GNU General Public License
    along with fastocloud.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once

#include <common/license/hardware_hash.h>
#include <common/serializer/json_serializer.h>

namespace common {
namespace daemon {
namespace commands {

class GetHardwareHashProject : public serializer::JsonSerializer<GetHardwareHashProject> {
 public:
  typedef serializer::JsonSerializer<GetHardwareHashProject> base_class;
  typedef license::ALGO_TYPE algo_t;
  GetHardwareHashProject();
  explicit GetHardwareHashProject(algo_t algo);

  algo_t GetAlgoType() const;

 protected:
  Error DoDeSerialize(json_object* serialized) override;
  Error SerializeFields(json_object* out) const override;

 private:
  algo_t algo_;
};

}  // namespace commands
}  // namespace daemon
}  // namespace common
