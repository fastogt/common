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

#include <common/serializer/json_serializer.h>

#include <common/sprintf.h>

namespace common {
namespace {
Error make_failed_to_add(const char* field, int err) {
  return Error(MemSPrintf("Failed to add field: %s, errno: %d", field, err));
}
Error make_invalid_type(const char* field) {
  return Error(MemSPrintf("Invalid type field: %s", field));
}
Error make_not_exists_field(const char* field) {
  return Error(MemSPrintf("Not exists field: %s", field));
}
}  // namespace
namespace serializer {

Error json_set_string(json_object* json, const char* field, const std::string& data) {
  if (!json || !field) {
    return make_error_inval();
  }

  return json_set_string(json, field, data.c_str());
}

Error json_set_string(json_object* json, const char* field, const char* data) {
  if (!json || !field) {
    return make_error_inval();
  }

  json_object* obj = json_object_new_string(data);
  return json_set_object(json, field, obj);
}

Error json_set_int(json_object* json, const char* field, int data) {
  if (!json || !field) {
    return make_error_inval();
  }

  json_object* obj = json_object_new_int(data);
  return json_set_object(json, field, obj);
}

Error json_set_int64(json_object* json, const char* field, int64_t data) {
  if (!json || !field) {
    return make_error_inval();
  }

  json_object* obj = json_object_new_int64(data);
  return json_set_object(json, field, obj);
}

Error json_set_uint64(json_object* json, const char* field, uint64_t data) {
  if (!json || !field) {
    return make_error_inval();
  }

  json_object* obj = json_object_new_uint64(data);
  return json_set_object(json, field, obj);
}

Error json_set_double(json_object* json, const char* field, double data) {
  if (!json || !field) {
    return make_error_inval();
  }

  json_object* obj = json_object_new_double(data);
  return json_set_object(json, field, obj);
}

Error json_set_float(json_object* json, const char* field, float data) {
  if (!json || !field) {
    return make_error_inval();
  }

  json_object* obj = json_object_new_double(data);
  return json_set_object(json, field, obj);
}

Error json_set_bool(json_object* json, const char* field, bool data) {
  if (!json || !field) {
    return make_error_inval();
  }

  json_object* obj = json_object_new_boolean(data);
  return json_set_object(json, field, obj);
}

Error json_set_array(json_object* json, const char* field, json_object* data) {
  if (!json || !field) {
    return make_error_inval();
  }

  if (!json_object_is_type(data, json_type_array)) {
    return make_invalid_type(field);
  }

  return json_set_object(json, field, data);
}

Error json_set_object(json_object* json, const char* field, json_object* data) {
  if (!json || !field) {
    return make_error_inval();
  }

  int result = json_object_object_add(json, field, data);
  if (result != 0) {
    return make_failed_to_add(field, result);
  }

  return common::Error();
}

Error json_get_string(json_object* json, const char* field, std::string* out) {
  if (!json || !field || !out) {
    return make_error_inval();
  }

  json_object* jstring = nullptr;
  json_bool jstring_exists = json_object_object_get_ex(json, field, &jstring);
  if (!jstring_exists) {
    return make_not_exists_field(field);
  }

  if (!json_object_is_type(jstring, json_type_string)) {
    return make_invalid_type(field);
  }

  *out = json_object_get_string(jstring);
  return Error();
}

Error json_get_int(json_object* json, const char* field, int* out) {
  if (!json || !field || !out) {
    return make_error_inval();
  }

  json_object* jint = nullptr;
  json_bool jint_exists = json_object_object_get_ex(json, field, &jint);
  if (!jint_exists) {
    return make_not_exists_field(field);
  }

  if (!json_object_is_type(jint, json_type_int)) {
    return make_invalid_type(field);
  }

  *out = json_object_get_int(jint);
  return Error();
}

Error json_get_int64(json_object* json, const char* field, int64_t* out) {
  if (!json || !field || !out) {
    return make_error_inval();
  }

  json_object* jint = nullptr;
  json_bool jint_exists = json_object_object_get_ex(json, field, &jint);
  if (!jint_exists) {
    return make_not_exists_field(field);
  }

  if (!json_object_is_type(jint, json_type_int)) {
    return make_invalid_type(field);
  }

  *out = json_object_get_int64(jint);
  return Error();
}

Error json_get_uint64(json_object* json, const char* field, uint64_t* out) {
  if (!json || !field || !out) {
    return make_error_inval();
  }

  json_object* jint = nullptr;
  json_bool jint_exists = json_object_object_get_ex(json, field, &jint);
  if (!jint_exists) {
    return make_not_exists_field(field);
  }

  if (!json_object_is_type(jint, json_type_int)) {
    return make_invalid_type(field);
  }

  *out = json_object_get_uint64(jint);
  return Error();
}

Error json_get_double(json_object* json, const char* field, double* out) {
  if (!json || !field || !out) {
    return make_error_inval();
  }

  json_object* jint = nullptr;
  json_bool jint_exists = json_object_object_get_ex(json, field, &jint);
  if (!jint_exists) {
    return make_not_exists_field(field);
  }

  if (!json_object_is_type(jint, json_type_double)) {
    return make_invalid_type(field);
  }

  *out = json_object_get_double(jint);
  return Error();
}

Error json_get_float(json_object* json, const char* field, float* out) {
  if (!json || !field || !out) {
    return make_error_inval();
  }

  json_object* jint = nullptr;
  json_bool jint_exists = json_object_object_get_ex(json, field, &jint);
  if (!jint_exists) {
    return make_not_exists_field(field);
  }

  if (!json_object_is_type(jint, json_type_double)) {
    return make_invalid_type(field);
  }

  *out = json_object_get_double(jint);
  return Error();
}

Error json_get_bool(json_object* json, const char* field, bool* out) {
  if (!json || !field || !out) {
    return make_error_inval();
  }

  json_object* jint = nullptr;
  json_bool jint_exists = json_object_object_get_ex(json, field, &jint);
  if (!jint_exists) {
    return make_not_exists_field(field);
  }

  if (!json_object_is_type(jint, json_type_boolean)) {
    return make_invalid_type(field);
  }

  *out = json_object_get_boolean(jint);
  return Error();
}

Error json_get_array(json_object* json, const char* field, json_object** out, size_t* len) {
  if (!json || !field || !out || !len) {
    return make_error_inval();
  }

  json_object* jarray = nullptr;
  json_bool jarray_exists = json_object_object_get_ex(json, field, &jarray);
  if (!jarray_exists) {
    return make_not_exists_field(field);
  }

  if (!json_object_is_type(jarray, json_type_array)) {
    return make_invalid_type(field);
  }

  *out = jarray;
  *len = json_object_array_length(jarray);
  return Error();
}

Error json_get_object(json_object* json, const char* field, json_object** out) {
  if (!json || !field || !out) {
    return make_error_inval();
  }

  json_object* jobj = nullptr;
  json_bool jobj_exists = json_object_object_get_ex(json, field, &jobj);
  if (!jobj_exists) {
    return make_not_exists_field(field);
  }

  if (!json_object_is_type(jobj, json_type_object)) {
    return make_invalid_type(field);
  }

  *out = jobj;
  return Error();
}

}  // namespace serializer
}  // namespace common
