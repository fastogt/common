/*  Copyright (C) 2014-2021 FastoGT. All right reserved.

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

#include <common/value.h>

#include <string.h>

#include <algorithm>

namespace common {
namespace {

class ValueEquals {
 public:
  explicit ValueEquals(const Value* first) : first_(first) {}

  bool operator()(const Value* second) const { return first_->Equals(second); }

 private:
  const Value* first_;
};
}  // namespace

Value::~Value() {}

// static
Value* Value::CreateNullValue() {
  return new Value(TYPE_NULL);
}

// static
FundamentalValue* Value::CreateBooleanValue(bool in_value) {
  return new FundamentalValue(in_value);
}

// static
FundamentalValue* Value::CreateIntegerValue(int in_value) {
  return new FundamentalValue(in_value);
}

FundamentalValue* Value::CreateUIntegerValue(unsigned int in_value) {
  return new FundamentalValue(in_value);
}

FundamentalValue* Value::CreateLongIntegerValue(long in_value) {
  return new FundamentalValue(in_value);
}

FundamentalValue* Value::CreateULongIntegerValue(unsigned long in_value) {
  return new FundamentalValue(in_value);
}

FundamentalValue* Value::CreateLongLongIntegerValue(long long in_value) {
  return new FundamentalValue(in_value);
}

FundamentalValue* Value::CreateULongLongIntegerValue(unsigned long long in_value) {
  return new FundamentalValue(in_value);
}

// static
FundamentalValue* Value::CreateDoubleValue(double in_value) {
  return new FundamentalValue(in_value);
}

TimeValue* Value::CreateTimeValue(time_t time) {
  return new TimeValue(time);
}

// static
StringValue* Value::CreateEmptyStringValue() {
  return CreateStringValue(string_t());
}

// static
StringValue* Value::CreateStringValue(const string_t& in_value) {
  return new StringValue(in_value);
}

StringValue* Value::CreateStringValueFromBasicString(const std::string& in_value) {
  return new StringValue(string_t(in_value.begin(), in_value.end()));
}

// static
ArrayValue* Value::CreateArrayValue() {
  return new ArrayValue;
}

// static
ByteArrayValue* Value::CreateByteArrayValue(const byte_array_t& array) {
  return new ByteArrayValue(array);
}

// static
SetValue* Value::CreateSetValue() {
  return new SetValue;
}

ZSetValue* Value::CreateZSetValue() {
  return new ZSetValue;
}

HashValue* Value::CreateHashValue() {
  return new HashValue;
}

bool Value::GetAsBoolean(bool* out_value) const {
  UNUSED(out_value);

  return false;
}

bool Value::GetAsInteger(int* out_value) const {
  UNUSED(out_value);

  return false;
}

bool Value::GetAsUInteger(unsigned int* out_value) const {
  UNUSED(out_value);

  return false;
}

bool Value::GetAsLongInteger(long* out_value) const {
  UNUSED(out_value);

  return false;
}

bool Value::GetAsULongInteger(unsigned long* out_value) const {
  UNUSED(out_value);

  return false;
}

bool Value::GetAsLongLongInteger(long long* out_value) const {
  UNUSED(out_value);

  return false;
}

bool Value::GetAsULongLongInteger(unsigned long long* out_value) const {
  UNUSED(out_value);

  return false;
}

bool Value::GetAsDouble(double* out_value) const {
  UNUSED(out_value);

  return false;
}

bool Value::GetAsTime(time_t* out_value) const {
  UNUSED(out_value);

  return false;
}

bool Value::GetAsString(string_t* out_value) const {
  UNUSED(out_value);

  return false;
}

bool Value::GetAsBasicString(std::string* out_value) const {
  if (out_value) {
    string_t str;
    if (!GetAsString(&str)) {
      return false;
    }
    *out_value = str.as_string();
  }

  return true;
}

bool Value::GetAsList(ArrayValue** out_value) {
  UNUSED(out_value);

  return false;
}

bool Value::GetAsList(const ArrayValue** out_value) const {
  UNUSED(out_value);

  return false;
}

bool Value::GetAsByteArray(byte_array_t* out_value) const {
  UNUSED(out_value);

  return false;
}

bool Value::GetAsSet(SetValue** out_value) {
  UNUSED(out_value);

  return false;
}

bool Value::GetAsSet(const SetValue** out_value) const {
  UNUSED(out_value);

  return false;
}

bool Value::GetAsZSet(ZSetValue** out_value) {
  UNUSED(out_value);

  return false;
}

bool Value::GetAsZSet(const ZSetValue** out_value) const {
  UNUSED(out_value);

  return false;
}

bool Value::GetAsHash(HashValue** out_value) {
  UNUSED(out_value);

  return false;
}

bool Value::GetAsHash(const HashValue** out_value) const {
  UNUSED(out_value);

  return false;
}

Value* Value::DeepCopy() const {
  DCHECK(IsType(TYPE_NULL));
  return CreateNullValue();
}

bool Value::Equals(const Value* other) const {
  if (!other) {
    return false;
  }
  DCHECK(IsType(TYPE_NULL));
  return other->IsType(TYPE_NULL);
}

// static
bool Value::Equals(const Value* a, const Value* b) {
  if ((a == nullptr) && (b == nullptr)) {
    return true;
  }
  if ((a == nullptr) ^ (b == nullptr)) {
    return false;
  }

  return a->Equals(b);
}

Value::Value(Type type) : type_(type) {}

Value::Value(const Value& that) : type_(that.type_) {}

Value& Value::operator=(const Value& that) {
  type_ = that.type_;
  return *this;
}

///////////////////// FundamentalValue ////////////////////

FundamentalValue::FundamentalValue(bool in_value) : Value(TYPE_BOOLEAN), boolean_value_(in_value) {}

FundamentalValue::FundamentalValue(int in_value) : Value(TYPE_INTEGER), integer_value_(in_value) {}

FundamentalValue::FundamentalValue(unsigned int in_value)
    : Value(TYPE_UINTEGER), integer_value_(static_cast<int>(in_value)) {}

FundamentalValue::FundamentalValue(long in_value) : Value(TYPE_LONG_INTEGER), long_integer_value_(in_value) {}

FundamentalValue::FundamentalValue(unsigned long in_value)
    : Value(TYPE_ULONG_INTEGER), long_integer_value_(static_cast<long>(in_value)) {}

FundamentalValue::FundamentalValue(long long in_value)
    : Value(TYPE_LONG_LONG_INTEGER), long_long_integer_value_(in_value) {}

FundamentalValue::FundamentalValue(unsigned long long in_value)
    : Value(TYPE_ULONG_LONG_INTEGER), long_long_integer_value_(static_cast<long long>(in_value)) {}

FundamentalValue::FundamentalValue(double in_value) : Value(TYPE_DOUBLE), double_value_(in_value) {}

FundamentalValue::~FundamentalValue() {}

bool FundamentalValue::GetAsBoolean(bool* out_value) const {
  if (out_value && IsType(TYPE_BOOLEAN)) {
    *out_value = boolean_value_;
  }

  return (IsType(TYPE_BOOLEAN));
}

bool FundamentalValue::GetAsInteger(int* out_value) const {
  if (out_value && IsType(TYPE_INTEGER)) {
    *out_value = integer_value_;
  }

  return (IsType(TYPE_INTEGER));
}

bool FundamentalValue::GetAsUInteger(unsigned int* out_value) const {
  if (out_value && IsType(TYPE_UINTEGER)) {
    *out_value = static_cast<unsigned int>(integer_value_);
  }

  return (IsType(TYPE_UINTEGER));
}

bool FundamentalValue::GetAsLongInteger(long* out_value) const {
  if (out_value && IsType(TYPE_LONG_INTEGER)) {
    *out_value = long_integer_value_;
  }

  return (IsType(TYPE_LONG_INTEGER));
}

bool FundamentalValue::GetAsULongInteger(unsigned long* out_value) const {
  if (out_value && IsType(TYPE_ULONG_INTEGER)) {
    *out_value = static_cast<unsigned long>(long_integer_value_);
  }

  return (IsType(TYPE_ULONG_INTEGER));
}

bool FundamentalValue::GetAsLongLongInteger(long long* out_value) const {
  if (out_value && IsType(TYPE_LONG_LONG_INTEGER)) {
    *out_value = long_long_integer_value_;
  }

  return (IsType(TYPE_LONG_LONG_INTEGER));
}

bool FundamentalValue::GetAsULongLongInteger(unsigned long long* out_value) const {
  if (out_value && IsType(TYPE_ULONG_LONG_INTEGER)) {
    *out_value = static_cast<unsigned long long>(long_long_integer_value_);
  }

  return (IsType(TYPE_ULONG_LONG_INTEGER));
}

bool FundamentalValue::GetAsDouble(double* out_value) const {
  if (out_value && IsType(TYPE_DOUBLE)) {
    *out_value = double_value_;
  } else if (out_value && IsType(TYPE_INTEGER)) {
    *out_value = integer_value_;
  } else if (out_value && IsType(TYPE_UINTEGER)) {
    *out_value = integer_value_;
  }

  return (IsType(TYPE_DOUBLE) || IsType(TYPE_INTEGER) || IsType(TYPE_UINTEGER));
}

FundamentalValue* FundamentalValue::DeepCopy() const {
  const Type t = GetType();
  if (t == TYPE_BOOLEAN) {
    return CreateBooleanValue(boolean_value_);
  } else if (t == TYPE_INTEGER) {
    return CreateIntegerValue(integer_value_);
  } else if (t == TYPE_UINTEGER) {
    return CreateUIntegerValue(static_cast<unsigned int>(integer_value_));
  } else if (t == TYPE_LONG_INTEGER) {
    return CreateLongIntegerValue(long_integer_value_);
  } else if (t == TYPE_ULONG_INTEGER) {
    return CreateULongIntegerValue(static_cast<unsigned long>(long_integer_value_));
  } else if (t == TYPE_LONG_LONG_INTEGER) {
    return CreateLongLongIntegerValue(long_long_integer_value_);
  } else if (t == TYPE_ULONG_LONG_INTEGER) {
    return CreateULongLongIntegerValue(static_cast<unsigned long long>(long_long_integer_value_));
  } else if (t == TYPE_DOUBLE) {
    return CreateDoubleValue(double_value_);
  }
  return nullptr;
}

bool FundamentalValue::Equals(const Value* other) const {
  if (other->GetType() != GetType()) {
    return false;
  }

  const Type t = GetType();
  if (t == TYPE_BOOLEAN) {
    bool lhs, rhs;
    return GetAsBoolean(&lhs) && other->GetAsBoolean(&rhs) && lhs == rhs;
  } else if (t == TYPE_INTEGER) {
    int lhs, rhs;
    return GetAsInteger(&lhs) && other->GetAsInteger(&rhs) && lhs == rhs;
  } else if (t == TYPE_UINTEGER) {
    unsigned int lhs, rhs;
    return GetAsUInteger(&lhs) && other->GetAsUInteger(&rhs) && lhs == rhs;
  } else if (t == TYPE_LONG_INTEGER) {
    long lhs, rhs;
    return GetAsLongInteger(&lhs) && other->GetAsLongInteger(&rhs) && lhs == rhs;
  } else if (t == TYPE_ULONG_INTEGER) {
    unsigned long lhs, rhs;
    return GetAsULongInteger(&lhs) && other->GetAsULongInteger(&rhs) && lhs == rhs;
  } else if (t == TYPE_LONG_LONG_INTEGER) {
    long long lhs, rhs;
    return GetAsLongLongInteger(&lhs) && other->GetAsLongLongInteger(&rhs) && lhs == rhs;
  } else if (t == TYPE_ULONG_LONG_INTEGER) {
    unsigned long long lhs, rhs;
    return GetAsULongLongInteger(&lhs) && other->GetAsULongLongInteger(&rhs) && lhs == rhs;
  } else if (t == TYPE_DOUBLE) {
    double lhs, rhs;
    return GetAsDouble(&lhs) && other->GetAsDouble(&rhs) && lhs == rhs;
  }
  return false;
}

TimeValue::TimeValue(time_t time) : Value(TYPE_TIME), value_(time) {}

TimeValue::~TimeValue() {}

TimeValue* TimeValue::DeepCopy() const {
  return CreateTimeValue(value_);
}

bool TimeValue::GetAsTime(time_t* out_value) const {
  if (out_value) {
    *out_value = value_;
  }

  return true;
}

bool TimeValue::Equals(const Value* other) const {
  if (other->GetType() != GetType()) {
    return false;
  }

  time_t lhs, rhs;
  return GetAsTime(&lhs) && other->GetAsTime(&rhs) && lhs == rhs;
}

///////////////////// StringValue ////////////////////

StringValue::StringValue(const string_t& in_value) : Value(TYPE_STRING), value_(in_value) {}

StringValue::~StringValue() {}

bool StringValue::GetAsString(string_t* out_value) const {
  if (out_value) {
    *out_value = value_;
  }

  return true;
}

StringValue* StringValue::DeepCopy() const {
  return CreateStringValue(value_);
}

bool StringValue::Equals(const Value* other) const {
  if (other->GetType() != GetType()) {
    return false;
  }

  string_t lhs, rhs;
  return GetAsString(&lhs) && other->GetAsString(&rhs) && lhs == rhs;
}

ArrayValue::ArrayValue() : Value(TYPE_ARRAY) {}

ArrayValue::~ArrayValue() {
  clear();
}

void ArrayValue::clear() {
  for (ValueVector::iterator i(list_.begin()); i != list_.end(); ++i) {
    delete *i;
  }
  list_.clear();
}

bool ArrayValue::Set(size_t index, Value* in_value) {
  if (!in_value) {
    return false;
  }

  if (index >= list_.size()) {
    // Pad out any intermediate indexes with null settings
    while (index > list_.size()) {
      Append(CreateNullValue());
    }
    Append(in_value);
  } else {
    DCHECK(list_[index] != in_value);
    delete list_[index];
    list_[index] = in_value;
  }
  return true;
}

bool ArrayValue::Get(size_t index, const Value** out_value) const {
  if (index >= list_.size()) {
    return false;
  }

  if (out_value) {
    *out_value = list_[index];
  }

  return true;
}

bool ArrayValue::Get(size_t index, Value** out_value) {
  return static_cast<const ArrayValue&>(*this).Get(index, const_cast<const Value**>(out_value));
}

bool ArrayValue::GetBoolean(size_t index, bool* bool_value) const {
  const Value* value;
  if (!Get(index, &value)) {
    return false;
  }

  return value->GetAsBoolean(bool_value);
}

bool ArrayValue::GetInteger(size_t index, int* out_value) const {
  const Value* value;
  if (!Get(index, &value)) {
    return false;
  }

  return value->GetAsInteger(out_value);
}

bool ArrayValue::GetUInteger(size_t index, unsigned int* out_value) const {
  const Value* value;
  if (!Get(index, &value)) {
    return false;
  }

  return value->GetAsUInteger(out_value);
}

bool ArrayValue::GetLongInteger(size_t index, long* out_value) const {
  const Value* value;
  if (!Get(index, &value)) {
    return false;
  }

  return value->GetAsLongInteger(out_value);
}

bool ArrayValue::GetULongInteger(size_t index, unsigned long* out_value) const {
  const Value* value;
  if (!Get(index, &value)) {
    return false;
  }

  return value->GetAsULongInteger(out_value);
}

bool ArrayValue::GetLongLongInteger(size_t index, long long* out_value) const {
  const Value* value;
  if (!Get(index, &value)) {
    return false;
  }

  return value->GetAsLongLongInteger(out_value);
}

bool ArrayValue::GetULongLongInteger(size_t index, unsigned long long* out_value) const {
  const Value* value;
  if (!Get(index, &value)) {
    return false;
  }

  return value->GetAsULongLongInteger(out_value);
}

bool ArrayValue::GetDouble(size_t index, double* out_value) const {
  const Value* value;
  if (!Get(index, &value)) {
    return false;
  }

  return value->GetAsDouble(out_value);
}

bool ArrayValue::GetString(size_t index, string_t* out_value) const {
  const Value* value;
  if (!Get(index, &value)) {
    return false;
  }

  return value->GetAsString(out_value);
}

bool ArrayValue::GetList(size_t index, const ArrayValue** out_value) const {
  const Value* value;
  bool result = Get(index, &value);
  if (!result || !value->IsType(TYPE_ARRAY)) {
    return false;
  }

  if (out_value) {
    *out_value = static_cast<const ArrayValue*>(value);
  }

  return true;
}

bool ArrayValue::GetList(size_t index, ArrayValue** out_value) {
  return static_cast<const ArrayValue&>(*this).GetList(index, const_cast<const ArrayValue**>(out_value));
}

bool ArrayValue::Remove(size_t index, std::unique_ptr<Value>* out_value) {
  if (index >= list_.size()) {
    return false;
  }

  if (out_value) {
    out_value->reset(list_[index]);
  } else {
    delete list_[index];
  }

  iterator it = list_.begin() + index;
  list_.erase(it);
  return true;
}

bool ArrayValue::Remove(const Value& value, size_t* index) {
  for (ValueVector::iterator i(list_.begin()); i != list_.end(); ++i) {
    if ((*i)->Equals(&value)) {
      size_t previous_index = i - list_.begin();
      delete *i;
      list_.erase(i);

      if (index) {
        *index = previous_index;
      }
      return true;
    }
  }
  return false;
}

ArrayValue::iterator ArrayValue::Erase(iterator iter, std::unique_ptr<Value>* out_value) {
  if (out_value) {
    out_value->reset(*iter);
  } else {
    delete *iter;
  }

  return list_.erase(iter);
}

void ArrayValue::Append(Value* in_value) {
  DCHECK(in_value);
  list_.push_back(in_value);
}

void ArrayValue::AppendBoolean(bool in_value) {
  Append(CreateBooleanValue(in_value));
}

void ArrayValue::AppendInteger(int in_value) {
  Append(CreateIntegerValue(in_value));
}

void ArrayValue::AppendDouble(double in_value) {
  Append(CreateDoubleValue(in_value));
}

void ArrayValue::AppendString(const string_t& in_value) {
  Append(CreateStringValue(in_value));
}

void ArrayValue::AppendStrings(const std::vector<string_t>& in_values) {
  for (std::vector<string_t>::const_iterator it = in_values.begin(); it != in_values.end(); ++it) {
    AppendString(*it);
  }
}

void ArrayValue::AppendBasicString(const std::string& in_value) {
  Append(CreateStringValueFromBasicString(in_value));
}

void ArrayValue::AppendBasicStrings(const std::vector<std::string>& in_values) {
  for (size_t i = 0; i < in_values.size(); ++i) {
    AppendBasicString(in_values[i]);
  }
}

bool ArrayValue::AppendIfNotPresent(Value* in_value) {
  DCHECK(in_value);
  for (ValueVector::const_iterator i(list_.begin()); i != list_.end(); ++i) {
    if ((*i)->Equals(in_value)) {
      delete in_value;
      return false;
    }
  }

  list_.push_back(in_value);
  return true;
}

bool ArrayValue::Insert(size_t index, Value* in_value) {
  DCHECK(in_value);
  if (index > list_.size()) {
    return false;
  }

  iterator it = list_.begin() + index;
  list_.insert(it, in_value);
  return true;
}

ArrayValue::const_iterator ArrayValue::Find(const Value& value) const {
  return std::find_if(list_.begin(), list_.end(), ValueEquals(&value));
}

void ArrayValue::Swap(ArrayValue* other) {
  list_.swap(other->list_);
}

bool ArrayValue::GetAsList(ArrayValue** out_value) {
  if (out_value) {
    *out_value = this;
  }

  return true;
}

bool ArrayValue::GetAsList(const ArrayValue** out_value) const {
  if (out_value) {
    *out_value = this;
  }

  return true;
}

ArrayValue* ArrayValue::DeepCopy() const {
  ArrayValue* result = new ArrayValue;

  for (const_iterator i = list_.begin(); i != list_.end(); ++i) {
    Value* cur = *i;
    Value* val = cur->DeepCopy();
    result->Append(val);
  }

  return result;
}

bool ArrayValue::Equals(const Value* other) const {
  if (other->GetType() != GetType()) {
    return false;
  }

  const ArrayValue* other_list = static_cast<const ArrayValue*>(other);
  const_iterator lhs_it, rhs_it;
  for (lhs_it = begin(), rhs_it = other_list->begin(); lhs_it != end() && rhs_it != other_list->end();
       ++lhs_it, ++rhs_it) {
    if (!(*lhs_it)->Equals(*rhs_it)) {
      return false;
    }
  }

  if (lhs_it != end() || rhs_it != other_list->end()) {
    return false;
  }

  return true;
}

ByteArrayValue::ByteArrayValue(const byte_array_t& array) : Value(TYPE_BYTE_ARRAY), array_(array) {}

ByteArrayValue::~ByteArrayValue() {
  Clear();
}

void ByteArrayValue::Clear() {
  array_.clear();
}

// Convenience forms of Append.
void ByteArrayValue::AppendBoolean(bool in_value) {
  byte_t arr[sizeof(bool)];
  memcpy(arr, &in_value, sizeof(bool));
  for (size_t i = 0; i < sizeof(bool); ++i) {
    byte_t b = arr[i];
    array_.push_back(b);
  }
}

void ByteArrayValue::AppendInteger(int in_value) {
  byte_t arr[sizeof(int)];
  memcpy(arr, &in_value, sizeof(int));
  for (size_t i = 0; i < sizeof(int); ++i) {
    byte_t b = arr[i];
    array_.push_back(b);
  }
}

void ByteArrayValue::AppendDouble(double in_value) {
  byte_t arr[sizeof(double)];
  memcpy(arr, &in_value, sizeof(double));
  for (size_t i = 0; i < sizeof(double); ++i) {
    byte_t b = arr[i];
    array_.push_back(b);
  }
}

void ByteArrayValue::AppendString(const string_t& in_value) {
  for (size_t i = 0; i < in_value.size(); ++i) {
    array_.push_back(in_value[i]);
  }
}

void ByteArrayValue::AppendStrings(const std::vector<string_t>& in_values) {
  for (size_t i = 0; i < in_values.size(); ++i) {
    AppendString(in_values[i]);
  }
}

void ByteArrayValue::AppendBasicString(const std::string& in_value) {
  for (size_t i = 0; i < in_value.size(); ++i) {
    array_.push_back(in_value[i]);
  }
}

void ByteArrayValue::AppendBasicStrings(const std::vector<std::string>& in_values) {
  for (size_t i = 0; i < in_values.size(); ++i) {
    AppendBasicString(in_values[i]);
  }
}

bool ByteArrayValue::GetAsByteArray(byte_array_t* out_value) const {
  if (out_value) {
    *out_value = array_;
  }
  return true;
}

ByteArrayValue* ByteArrayValue::DeepCopy() const {
  return CreateByteArrayValue(array_);
}

bool ByteArrayValue::Equals(const Value* other) const {
  if (other->GetType() != GetType()) {
    return false;
  }

  byte_array_t lhs, rhs;
  return GetAsByteArray(&lhs) && other->GetAsByteArray(&rhs) && lhs == rhs;
}

SetValue::SetValue() : Value(TYPE_SET) {}

SetValue::~SetValue() {
  Clear();
}

void SetValue::Clear() {
  for (ValueSet::iterator i(set_.begin()); i != set_.end(); ++i) {
    delete *i;
  }

  set_.clear();
}

void SetValue::Insert(const string_t& in_value) {
  Insert(CreateStringValue(in_value));
}

bool SetValue::Insert(Value* in_value) {
  DCHECK(in_value);
  if (!in_value) {
    return false;
  }

  set_.insert(in_value);
  return true;
}

bool SetValue::GetAsSet(SetValue** out_value) {
  if (out_value && IsType(TYPE_SET)) {
    *out_value = this;
  }

  return IsType(TYPE_SET);
}

bool SetValue::GetAsSet(const SetValue** out_value) const {
  if (out_value && IsType(TYPE_SET)) {
    *out_value = this;
  }

  return IsType(TYPE_SET);
}

SetValue* SetValue::DeepCopy() const {
  SetValue* result = new SetValue;

  for (const_iterator i = set_.begin(); i != set_.end(); ++i) {
    Value* cur = *i;
    Value* val = cur->DeepCopy();
    result->Insert(val);
  }

  return result;
}

bool SetValue::Equals(const Value* other) const {
  if (other->GetType() != GetType()) {
    return false;
  }

  const SetValue* other_set = static_cast<const SetValue*>(other);
  const_iterator lhs_it, rhs_it;
  for (lhs_it = begin(), rhs_it = other_set->begin(); lhs_it != end() && rhs_it != other_set->end();
       ++lhs_it, ++rhs_it) {
    if (!(*lhs_it)->Equals(*rhs_it)) {
      return false;
    }
  }

  if (lhs_it != end() || rhs_it != other_set->end()) {
    return false;
  }

  return true;
}

ZSetValue::ZSetValue() : Value(TYPE_ZSET) {}

ZSetValue::~ZSetValue() {
  Clear();
}

void ZSetValue::Clear() {
  for (ValueZSet::iterator i(map_.begin()); i != map_.end(); ++i) {
    ValueZSet::value_type m = *i;
    delete m.first;
    delete m.second;
  }
  map_.clear();
}

bool ZSetValue::Insert(Value* key, Value* value) {
  if (!key || !value) {
    return false;
  }

  map_[key] = value;
  return true;
}

void ZSetValue::Insert(const string_t& key, const string_t& value) {
  Insert(Value::CreateStringValue(key), Value::CreateStringValue(value));
}

bool ZSetValue::GetAsZSet(ZSetValue** out_value) {
  if (out_value && IsType(TYPE_ZSET)) {
    *out_value = this;
  }

  return IsType(TYPE_ZSET);
}

bool ZSetValue::GetAsZSet(const ZSetValue** out_value) const {
  if (out_value && IsType(TYPE_ZSET)) {
    *out_value = this;
  }

  return IsType(TYPE_ZSET);
}

ZSetValue* ZSetValue::DeepCopy() const {
  ZSetValue* result = new ZSetValue;

  for (const_iterator i = map_.begin(); i != map_.end(); ++i) {
    auto key_value = *i;
    Value* key = key_value.first->DeepCopy();
    Value* val = key_value.second->DeepCopy();
    result->Insert(key, val);
  }

  return result;
}

bool ZSetValue::Equals(const Value* other) const {
  if (other->GetType() != GetType()) {
    return false;
  }

  const ZSetValue* other_zset = static_cast<const ZSetValue*>(other);
  const_iterator lhs_it, rhs_it;
  for (lhs_it = begin(), rhs_it = other_zset->begin(); lhs_it != end() && rhs_it != other_zset->end();
       ++lhs_it, ++rhs_it) {
    Value* rkey = (*rhs_it).first;
    Value* rval = (*rhs_it).second;
    Value* lkey = (*lhs_it).first;
    Value* lval = (*lhs_it).second;

    if (!lkey->Equals(rkey) || !lval->Equals(rval)) {
      return false;
    }
  }

  if (lhs_it != end() || rhs_it != other_zset->end()) {
    return false;
  }

  return true;
}

HashValue::HashValue() : Value(TYPE_HASH) {}

HashValue::~HashValue() {
  Clear();
}

void HashValue::Clear() {
  for (iterator i(hash_.begin()); i != hash_.end(); ++i) {
    value_type m = *i;
    delete m.second;
  }
  hash_.clear();
}

bool HashValue::Insert(const string_t& key, Value* value) {
  if (key.empty() || !value) {
    return false;
  }

  hash_[key] = value;
  return true;
}

bool HashValue::Insert(const std::string& key, Value* value) {
  if (key.empty() || !value) {
    return false;
  }

  hash_[string_t(key.begin(), key.end())] = value;
  return true;
}

Value* HashValue::Find(const string_t& key) const {
  if (key.empty()) {
    return nullptr;
  }

  const auto it = hash_.find(key);
  if (it == hash_.end()) {
    return nullptr;
  }
  return it->second;
}

Value* HashValue::Find(const std::string& key) const {
  if (key.empty()) {
    return nullptr;
  }

  return Find(string_t(key.begin(), key.end()));
}

bool HashValue::GetAsHash(HashValue** out_value) {
  if (out_value && IsType(TYPE_HASH)) {
    *out_value = this;
  }

  return IsType(TYPE_HASH);
}
bool HashValue::GetAsHash(const HashValue** out_value) const {
  if (out_value && IsType(TYPE_HASH)) {
    *out_value = this;
  }

  return IsType(TYPE_HASH);
}

HashValue* HashValue::DeepCopy() const {
  HashValue* result = new HashValue;

  for (const_iterator i = hash_.begin(); i != hash_.end(); ++i) {
    auto key_value = *i;
    Value* val = key_value.second->DeepCopy();
    result->Insert(key_value.first, val);
  }

  return result;
}

bool HashValue::Equals(const Value* other) const {
  if (other->GetType() != GetType()) {
    return false;
  }

  const HashValue* other_zset = static_cast<const HashValue*>(other);
  const_iterator lhs_it, rhs_it;
  for (lhs_it = begin(), rhs_it = other_zset->begin(); lhs_it != end() && rhs_it != other_zset->end();
       ++lhs_it, ++rhs_it) {
    string_t rkey = (*rhs_it).first;
    Value* rval = (*rhs_it).second;
    string_t lkey = (*lhs_it).first;
    Value* lval = (*lhs_it).second;

    if (lkey != rkey || !lval->Equals(rval)) {
      return false;
    }
  }

  if (lhs_it != end() || rhs_it != other_zset->end()) {
    return false;
  }

  return true;
}

std::ostream& operator<<(std::ostream& out, const Value& value) {
  const Value::Type value_type = value.GetType();
  if (value_type == Value::TYPE_BOOLEAN) {
    bool res;
    if (value.GetAsBoolean(&res)) {
      return out << res;
    }
  } else if (value_type == Value::TYPE_INTEGER) {
    int res;
    if (value.GetAsInteger(&res)) {
      return out << res;
    }
  } else if (value_type == Value::TYPE_UINTEGER) {
    unsigned int res;
    if (value.GetAsUInteger(&res)) {
      return out << res;
    }
  } else if (value_type == Value::TYPE_LONG_INTEGER) {
    long res;
    if (value.GetAsLongInteger(&res)) {
      return out << res;
    }
  } else if (value_type == Value::TYPE_ULONG_INTEGER) {
    unsigned long res;
    if (value.GetAsULongInteger(&res)) {
      return out << res;
    }
  } else if (value_type == Value::TYPE_LONG_LONG_INTEGER) {
    long long res;
    if (value.GetAsLongLongInteger(&res)) {
      return out << res;
    }
  } else if (value_type == Value::TYPE_ULONG_LONG_INTEGER) {
    unsigned long long res;
    if (value.GetAsULongLongInteger(&res)) {
      return out << res;
    }
  } else if (value_type == Value::TYPE_DOUBLE) {
    double res;
    if (value.GetAsDouble(&res)) {
      return out << res;
    }
  } else if (value_type == Value::TYPE_TIME) {
    time_t res;
    if (value.GetAsTime(&res)) {
      return out << res;
    }
  } else if (value_type == Value::TYPE_STRING) {
    Value::string_t res;
    if (value.GetAsString(&res)) {
      return out << '"' << res.as_string() << '"';
    }
  } else if (value_type == Value::TYPE_ARRAY) {
    const ArrayValue* array = nullptr;
    if (value.GetAsList(&array)) {
      out << '[';
      for (auto it = array->begin(); it != array->end(); ++it) {
        Value* val = *it;
        const Value& rval = *val;
        out << rval;
        if (std::next(it) != it) {
          out << ", ";
        }
      }
      out << ']';
      return out;
    }
  } else if (value_type == Value::TYPE_BYTE_ARRAY) {
    byte_array_t res;
    if (value.GetAsByteArray(&res)) {
      return out << '"' << res.as_string() << '"';
    }
    return out;
  } else if (value_type == Value::TYPE_SET) {
    const SetValue* set = nullptr;
    if (value.GetAsSet(&set)) {
      out << '(';
      for (auto it = set->begin(); it != set->end(); ++it) {
        Value* val = *it;
        const Value& rval = *val;
        out << rval;
        if (std::next(it) != it) {
          out << ", ";
        }
      }
      out << ')';
    }
    return out;
  } else if (value_type == Value::TYPE_ZSET) {
    const ZSetValue* zset = nullptr;
    if (value.GetAsZSet(&zset)) {
      out << '{';
      for (auto it = zset->begin(); it != zset->end(); ++it) {
        auto mapped = *it;
        const Value& rkey = *(mapped.first);
        const Value& rval = *(mapped.second);
        out << rkey << ": " << rval;
        if (std::next(it) != zset->end()) {
          out << ", ";
        }
      }
      out << '}';
    }
    return out;
  } else if (value_type == Value::TYPE_HASH) {
    const HashValue* hash = nullptr;
    if (value.GetAsHash(&hash)) {
      out << '{';
      for (auto it = hash->begin(); it != hash->end(); ++it) {
        auto mapped = *it;
        const std::string key = mapped.first.as_string();
        const Value& rval = *(mapped.second);
        out << '"' << key << '"' << ": " << rval;
        if (std::next(it) != hash->end()) {
          out << ", ";
        }
      }
      out << '}';
    }
    return out;
  }
  return out;
}

}  // namespace common
