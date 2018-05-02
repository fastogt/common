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

#pragma once

#include <json-c/json_object.h>
#include <json-c/json_tokener.h>  // for json_tokener_parse

#include <common/serializer/iserializer.h>  // for ISerializer

namespace common {
namespace serializer {

template <typename T>
class JsonSerializerBase : public ISerializer<struct json_object*> {
 public:
  typedef ISerializer<struct json_object*> base_class;
  typedef typename base_class::serialize_type serialize_type;

  virtual Error SerializeToString(std::string* out) const override final WARN_UNUSED_RESULT {
    serialize_type des = NULL;
    Error err = base_class::Serialize(&des);
    if (err) {
      return err;
    }

    *out = json_object_get_string(des);
    json_object_put(des);
    return Error();
  }

  virtual Error SerializeFromString(const std::string& data,
                                    serialize_type* out) const override final WARN_UNUSED_RESULT {
    const char* data_ptr = data.c_str();
    serialize_type res = json_tokener_parse(data_ptr);
    if (!res) {
      return make_error_inval();
    }

    *out = res;
    return Error();
  }

  Error DeSerialize(const serialize_type& serialized) WARN_UNUSED_RESULT {
    if (!serialized) {
      return make_error_inval();
    }

    return DoDeSerialize(serialized);
  }

 protected:
  virtual Error DoDeSerialize(json_object* serialized) = 0;
};

template <typename T>
class JsonSerializer : public JsonSerializerBase<T> {
 public:
  typedef JsonSerializerBase<T> base_class;
  typedef typename base_class::serialize_type serialize_type;

 protected:
  virtual common::Error DoDeSerialize(json_object* serialized) = 0;
  virtual common::Error SerializeFields(json_object* out) const = 0;

  virtual common::Error DoSerialize(serialize_type* out) const override final {
    json_object* obj = json_object_new_object();
    common::Error err = SerializeFields(obj);
    if (err) {
      json_object_put(obj);
      return err;
    }

    *out = obj;
    return common::Error();
  }
};

template <typename T>
class JsonSerializerArray : public JsonSerializerBase<T> {
 public:
  typedef JsonSerializerBase<T> base_class;
  typedef typename base_class::serialize_type serialize_type;

 protected:
  virtual common::Error DoDeSerialize(json_object* serialized_array) = 0;
  virtual common::Error SerializeArray(json_object* out_array) const = 0;

  virtual common::Error DoSerialize(serialize_type* out) const override final {
    json_object* obj = json_object_new_array();
    common::Error err = SerializeArray(obj);
    if (err) {
      json_object_put(obj);
      return err;
    }

    *out = obj;
    return common::Error();
  }
};

}  // namespace serializer
}  // namespace common
