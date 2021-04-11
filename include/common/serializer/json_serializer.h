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

#include <json-c/json_object.h>
#include <json-c/json_tokener.h>  // for json_tokener_parse

#include <string>
#include <vector>

#include <common/serializer/iserializer.h>  // for ISerializer

namespace common {
namespace serializer {

Error json_set_string(json_object* json, const char* field, const std::string& data);
Error json_set_string(json_object* json, const char* field, const char* data);
Error json_set_int(json_object* json, const char* field, int data);
Error json_set_int64(json_object* json, const char* field, int64_t data);
Error json_set_uint64(json_object* json, const char* field, uint64_t data);
Error json_set_double(json_object* json, const char* field, double data);
Error json_set_float(json_object* json, const char* field, float data);
Error json_set_bool(json_object* json, const char* field, bool data);
Error json_set_array(json_object* json, const char* field, json_object* data);
Error json_set_object(json_object* json, const char* field, json_object* data);

Error json_get_string(json_object* json, const char* field, std::string* out);
Error json_get_int(json_object* json, const char* field, int* out);
Error json_get_int64(json_object* json, const char* field, int64_t* out);
Error json_get_uint64(json_object* json, const char* field, uint64_t* out);
Error json_get_double(json_object* json, const char* field, double* out);
Error json_get_float(json_object* json, const char* field, float* out);
Error json_get_bool(json_object* json, const char* field, bool* out);
Error json_get_array(json_object* json, const char* field, json_object** out, size_t* len);
Error json_get_object(json_object* json, const char* field, json_object** out);

template <typename T>
class JsonSerializerBase : public ISerializer<struct json_object*> {
 public:
  typedef ISerializer<struct json_object*> base_class;
  typedef typename base_class::serialize_type serialize_type;

  static Error SetStringField(serialize_type json, const char* field, const std::string& data) WARN_UNUSED_RESULT {
    return json_set_string(json, field, data);
  }

  static Error SetStringField(serialize_type json, const char* field, const char* data) WARN_UNUSED_RESULT {
    return json_set_string(json, field, data);
  }

  template <typename U>
  static Error SetEnumField(serialize_type json, const char* field, U data) WARN_UNUSED_RESULT;

  static Error SetIntField(serialize_type json, const char* field, int data) WARN_UNUSED_RESULT {
    return json_set_int(json, field, data);
  }

  static Error SetInt64Field(serialize_type json, const char* field, int64_t data) WARN_UNUSED_RESULT {
    return json_set_int64(json, field, data);
  }

  static Error SetUInt64Field(serialize_type json, const char* field, uint64_t data) WARN_UNUSED_RESULT {
    return json_set_uint64(json, field, data);
  }

  static Error SetDoubleField(serialize_type json, const char* field, double data) WARN_UNUSED_RESULT {
    return json_set_double(json, field, data);
  }

  static Error SetFloatField(serialize_type json, const char* field, float data) WARN_UNUSED_RESULT {
    return json_set_float(json, field, data);
  }

  static Error SetBoolField(serialize_type json, const char* field, bool data) WARN_UNUSED_RESULT {
    return json_set_bool(json, field, data);
  }

  static Error SetArrayField(serialize_type json, const char* field, serialize_type data) WARN_UNUSED_RESULT {
    return json_set_array(json, field, data);
  }

  static Error SetObjectField(serialize_type json, const char* field, serialize_type data) WARN_UNUSED_RESULT {
    return json_set_object(json, field, data);
  }

  static Error GetStringField(serialize_type json, const char* field, std::string* out) WARN_UNUSED_RESULT {
    return json_get_string(json, field, out);
  }

  template <typename U>
  static Error GetEnumField(serialize_type json, const char* field, U* out) WARN_UNUSED_RESULT;

  static Error GetIntField(serialize_type json, const char* field, int* out) WARN_UNUSED_RESULT {
    return json_get_int(json, field, out);
  }

  static Error GetInt64Field(serialize_type json, const char* field, int64_t* out) WARN_UNUSED_RESULT {
    return json_get_int64(json, field, out);
  }

  static Error GetUint64Field(serialize_type json, const char* field, uint64_t* out) WARN_UNUSED_RESULT {
    return json_get_uint64(json, field, out);
  }

  static Error GetDoubleField(serialize_type json, const char* field, double* out) WARN_UNUSED_RESULT {
    return json_get_double(json, field, out);
  }

  static Error GetFloatField(serialize_type json, const char* field, float* out) WARN_UNUSED_RESULT {
    return json_get_float(json, field, out);
  }

  static Error GetBoolField(serialize_type json, const char* field, bool* out) WARN_UNUSED_RESULT {
    return json_get_bool(json, field, out);
  }

  static Error GetArrayField(serialize_type json, const char* field, serialize_type* out, size_t* len)
      WARN_UNUSED_RESULT {
    return json_get_array(json, field, out, len);
  }

  static Error GetObjectField(serialize_type json, const char* field, serialize_type* out) WARN_UNUSED_RESULT {
    return json_get_object(json, field, out);
  }

  Error SerializeToString(std::string* out) const override final WARN_UNUSED_RESULT {
    serialize_type des = nullptr;
    Error err = base_class::Serialize(&des);
    if (err) {
      return err;
    }

    *out = json_object_get_string(des);
    json_object_put(des);
    return Error();
  }

  Error SerializeFromString(const std::string& data, serialize_type* out) const override final WARN_UNUSED_RESULT {
    const char* data_ptr = data.c_str();
    serialize_type res = json_tokener_parse(data_ptr);
    if (!res) {
      return make_error_inval();
    }

    *out = res;
    return Error();
  }

  Error DeSerializeFromString(const std::string& data) WARN_UNUSED_RESULT {
    const char* data_ptr = data.c_str();
    serialize_type res = json_tokener_parse(data_ptr);
    if (!res) {
      return make_error_inval();
    }

    Error err = DeSerialize(res);
    json_object_put(res);
    return err;
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
  virtual Error DoDeSerialize(json_object* serialized) override = 0;
  virtual Error SerializeFields(json_object* out) const = 0;

  Error DoSerialize(serialize_type* out) const override final {
    json_object* obj = json_object_new_object();
    Error err = SerializeFields(obj);
    if (err) {
      json_object_put(obj);
      return err;
    }

    *out = obj;
    return Error();
  }
};

template <typename T>
class JsonSerializerArray : public JsonSerializerBase<T> {
 public:
  typedef std::vector<T> container_t;
  typedef JsonSerializerBase<T> base_class;
  typedef typename base_class::serialize_type serialize_type;

  container_t Get() const { return array_; }

  bool Empty() const { return array_.empty(); }

  void Add(const T& value) { array_.push_back(value); }

  bool Equals(const JsonSerializerArray<T>& val) const { return array_ == val.array_; }

  size_t Size() const { return array_.size(); }

 protected:
  common::Error SerializeArray(json_object* deserialized_array) const {
    for (T value : array_) {
      json_object* jvalue = nullptr;
      common::Error err = value.Serialize(&jvalue);
      if (err) {
        continue;
      }
      json_object_array_add(deserialized_array, jvalue);
    }

    return common::Error();
  }

  common::Error DoDeSerialize(json_object* serialized) override {
    container_t array;
    size_t len = json_object_array_length(serialized);
    for (size_t i = 0; i < len; ++i) {
      json_object* jvalue = json_object_array_get_idx(serialized, i);
      T value;
      common::Error err = value.DeSerialize(jvalue);
      if (err) {
        continue;
      }
      array.push_back(value);
    }

    (*this).array_ = array;
    return common::Error();
  }

  Error DoSerialize(serialize_type* out) const final {
    json_object* obj = json_object_new_array();
    Error err = SerializeArray(obj);
    if (err) {
      json_object_put(obj);
      return err;
    }

    *out = obj;
    return Error();
  }

 private:
  container_t array_;
};

template <typename T>
inline bool operator==(const JsonSerializerArray<T>& lhs, const JsonSerializerArray<T>& rhs) {
  return lhs.Equals(rhs);
}

template <typename T>
inline bool operator!=(const JsonSerializerArray<T>& x, const JsonSerializerArray<T>& y) {
  return !(x == y);
}

template <typename T>
template <typename U>
inline Error JsonSerializerBase<T>::SetEnumField(serialize_type json, const char* field, U data) {
  return SetIntField(json, field, data);
}

template <typename T>
template <typename U>
inline Error JsonSerializerBase<T>::GetEnumField(serialize_type json, const char* field, U* out) {
  int res;
  Error err = json_get_int(json, field, &res);
  if (err) {
    return err;
  }

  *out = static_cast<U>(res);
  return Error();
}

}  // namespace serializer
}  // namespace common
