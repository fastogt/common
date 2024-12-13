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

#pragma once

#include <common/serializer/json_serializer.h>

namespace common {
namespace json {

class ErrorJsonMessage : public common::serializer::JsonSerializer<ErrorJsonMessage> {
 public:
  typedef int code_t;
  typedef common::serializer::JsonSerializer<ErrorJsonMessage> base_class;

  ErrorJsonMessage();
  ErrorJsonMessage(code_t code, const std::string& message);

  bool Equals(const ErrorJsonMessage& errj) const;

  code_t GetCode() const;
  std::string GetMessage() const;

 protected:
  common::Error DoDeSerialize(json_object* serialized) override;
  common::Error SerializeFields(json_object* deserialized) const override;

 private:
  code_t code_;
  std::string message_;
};

ErrorJsonMessage MakeErrorJsonMessage(ErrorJsonMessage::code_t code, Error err);

class ErrorJson : public common::serializer::JsonSerializer<ErrorJson> {
 public:
  typedef common::serializer::JsonSerializer<ErrorJson> base_class;

  ErrorJson();

  ErrorJsonMessage GetError() const;

  bool Equals(const ErrorJson& data) const;

 protected:
  common::Error DoDeSerialize(json_object* serialized) override;
  common::Error SerializeFields(json_object* deserialized) const override;

 private:
  ErrorJsonMessage error_;
};

class DataJson : public common::serializer::JsonSerializer<DataJson> {
 public:
  typedef common::serializer::JsonSerializer<DataJson> base_class;

  DataJson();
  virtual ~DataJson();

  json_object* GetData() const;

  bool Equals(const DataJson& data) const;

 protected:
  common::Error DoDeSerialize(json_object* serialized) override;
  common::Error SerializeFields(json_object* deserialized) const override;

 private:
  json_object* data_;
};

}  // namespace json
}  // namespace common
