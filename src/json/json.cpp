/*  Copyright (C) 2014-2022 FastoGT. All right reserved.

 Redistribution and use in source and binary forms, with or without
 modification, are permitted provided that the following conditions are
 met:

     * Redistributions of source code must retain the above copyright
 notice, this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above
 copyright notice, this list of conditions and the following disclaimer
 in the documentation and/or other materials provided with the
 distribution.
     * Neither the name of FastoGT. nor the names of its
 contributors may be used to endorse or promote products derived from
 this software without specific prior written permission.

 THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
 A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
 OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
 LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#include <common/json/json.h>

#define ERROR_JSON_MESSAGE_CODE_FIELD "code"
#define ERROR_JSON_MESSAGE_MESSAGE_FIELD "message"

#define ERROR_JSON_ERROR_FIELD "error"

#define DATA_JSON_DATA_FIELD "data"

namespace common {
namespace json {

ErrorJsonMessage MakeErrorJsonMessage(ErrorJsonMessage::code_t code, Error err) {
  return ErrorJsonMessage(code, err->GetDescription());
}

ErrorJsonMessage::ErrorJsonMessage() : ErrorJsonMessage(0, std::string()) {}

ErrorJsonMessage::ErrorJsonMessage(code_t code, const std::string& message)
    : base_class(), code_(code), message_(message) {}

bool ErrorJsonMessage::Equals(const ErrorJsonMessage& errj) const {
  return code_ == errj.code_ && message_ == errj.message_;
}

ErrorJsonMessage::code_t ErrorJsonMessage::GetCode() const {
  return code_;
}

std::string ErrorJsonMessage::GetMessage() const {
  return message_;
}

common::Error ErrorJsonMessage::DoDeSerialize(json_object* serialized) {
  ErrorJsonMessage inf;
  json_object* jcode = nullptr;
  json_bool jcode_exists = json_object_object_get_ex(serialized, ERROR_JSON_MESSAGE_CODE_FIELD, &jcode);
  if (!jcode_exists) {
    return make_error_inval();
  }
  inf.message_ = json_object_get_int(jcode);

  json_object* jmessage = nullptr;
  json_bool jmessage_exists = json_object_object_get_ex(serialized, ERROR_JSON_MESSAGE_MESSAGE_FIELD, &jmessage);
  if (!jmessage_exists) {
    return make_error_inval();
  }
  inf.message_ = json_object_get_string(jmessage);

  *this = inf;
  return common::Error();
}

common::Error ErrorJsonMessage::SerializeFields(json_object* out) const {
  json_object_object_add(out, ERROR_JSON_MESSAGE_CODE_FIELD, json_object_new_int(code_));
  json_object_object_add(out, ERROR_JSON_MESSAGE_MESSAGE_FIELD, json_object_new_string(message_.c_str()));
  return common::Error();
}

ErrorJson::ErrorJson() : error_() {}

ErrorJsonMessage ErrorJson::GetError() const {
  return error_;
}

bool ErrorJson::Equals(const ErrorJson& data) const {
  return error_.Equals(data.error_);
}

common::Error ErrorJson::DoDeSerialize(json_object* serialized) {
  ErrorJson inf;
  json_object* jerror = nullptr;
  json_bool jerror_exists = json_object_object_get_ex(serialized, ERROR_JSON_ERROR_FIELD, &jerror);
  if (!jerror_exists) {
    return make_error_inval();
  }

  common::Error err = inf.error_.DeSerialize(jerror);
  if (err) {
    return err;
  }

  *this = inf;
  return common::Error();
}

common::Error ErrorJson::SerializeFields(json_object* out) const {
  json_object* jerror;
  common::Error err = error_.Serialize(&jerror);
  if (err) {
    return err;
  }
  json_object_object_add(out, ERROR_JSON_ERROR_FIELD, jerror);
  return common::Error();
}

DataJson::DataJson() : DataJson(json_object_new_null()) {}

DataJson::DataJson(json_object* data) : base_class(), data_(data) {}

json_object* DataJson::GetData() const {
  return data_;
}

bool DataJson::Equals(const DataJson& data) const {
  return json_object_equal(data_, data.data_) == 1;
}

common::Error DataJson::DoDeSerialize(json_object* serialized) {
  json_object* jdata = nullptr;
  json_bool jdata_exists = json_object_object_get_ex(serialized, DATA_JSON_DATA_FIELD, &jdata);
  if (!jdata_exists) {
    return make_error_inval();
  };
  json_object_put(data_);
  data_ = jdata;

  return common::Error();
}

common::Error DataJson::SerializeFields(json_object* out) const {
  json_object* copy = nullptr;
  json_object_deep_copy(data_, &copy, nullptr);
  json_object_object_add(out, DATA_JSON_DATA_FIELD, copy);
  return common::Error();
}

DataJson::~DataJson() {
  json_object_put(data_);
}

DataJson MakeSuccessDataJson(json_object* data) {
  return DataJson(data);
}

}  // namespace json
}  // namespace common
