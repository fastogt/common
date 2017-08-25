/*  Copyright (C) 2014-2017 FastoGT. All right reserved.

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

#include <map>
#include <memory>
#include <set>
#include <unordered_map>

#include <common/string16.h>
#include <common/types.h>  // for byte_array_t

namespace common {

class ArrayValue;
class ByteArrayValue;
class ErrorValue;
class FundamentalValue;
class HashValue;
class SetValue;
class StringValue;
class ZSetValue;

class Value {
 public:
  enum Type {
    TYPE_NULL = 0,
    TYPE_BOOLEAN,
    TYPE_INTEGER,
    TYPE_UINTEGER,
    TYPE_LONG_INTEGER,
    TYPE_ULONG_INTEGER,
    TYPE_LONG_LONG_INTEGER,
    TYPE_ULONG_LONG_INTEGER,
    TYPE_DOUBLE,
    TYPE_STRING,
    TYPE_ARRAY,  // list
    TYPE_BYTE_ARRAY,
    TYPE_SET,  // set
    TYPE_ZSET,
    TYPE_HASH,
    TYPE_ERROR
  };

  enum ErrorsType { E_NONE, E_EXCEPTION, E_ERROR, E_INTERRUPTED };

  virtual ~Value();

  // simple types
  static Value* CreateNullValue();
  static FundamentalValue* CreateBooleanValue(bool in_value);
  static FundamentalValue* CreateIntegerValue(int in_value);
  static FundamentalValue* CreateUIntegerValue(unsigned int in_value);
  static FundamentalValue* CreateLongIntegerValue(long in_value);
  static FundamentalValue* CreateULongIntegerValue(unsigned long in_value);
  static FundamentalValue* CreateLongLongIntegerValue(long long in_value);
  static FundamentalValue* CreateULongLongIntegerValue(unsigned long long in_value);
  static FundamentalValue* CreateDoubleValue(double in_value);

  static StringValue* CreateStringValue(const std::string& in_value);
  static ArrayValue* CreateArrayValue();
  static ByteArrayValue* CreateByteArrayValue(const byte_array_t& array);
  static SetValue* CreateSetValue();
  static ZSetValue* CreateZSetValue();
  static HashValue* CreateHashValue();
  static ErrorValue* CreateErrorValue(const std::string& in_value, ErrorsType errorType, logging::LEVEL_LOG level);

  static Value* CreateEmptyValueFromType(Type t);

  static bool IsIntegral(Type type) {
    return type == TYPE_BOOLEAN || type == TYPE_INTEGER || type == TYPE_UINTEGER || type == TYPE_LONG_INTEGER ||
           type == TYPE_ULONG_INTEGER || type == TYPE_LONG_LONG_INTEGER || type == TYPE_ULONG_LONG_INTEGER ||
           type == TYPE_DOUBLE;
  }

  static std::string GetTypeName(Type t);
  Type GetType() const { return type_; }

  bool IsType(Type type) const { return type == type_; }

  virtual bool GetAsBoolean(bool* out_value) const WARN_UNUSED_RESULT;
  virtual bool GetAsInteger(int* out_value) const WARN_UNUSED_RESULT;
  virtual bool GetAsUInteger(unsigned int* out_value) const WARN_UNUSED_RESULT;
  virtual bool GetAsLongInteger(long* out_value) const WARN_UNUSED_RESULT;
  virtual bool GetAsULongInteger(unsigned long* out_value) const WARN_UNUSED_RESULT;
  virtual bool GetAsLongLongInteger(long long* out_value) const WARN_UNUSED_RESULT;
  virtual bool GetAsULongLongInteger(unsigned long long* out_value) const WARN_UNUSED_RESULT;
  virtual bool GetAsDouble(double* out_value) const WARN_UNUSED_RESULT;
  virtual bool GetAsString(std::string* out_value) const WARN_UNUSED_RESULT;
  virtual bool GetAsError(ErrorValue* out_value) const WARN_UNUSED_RESULT;
  virtual bool GetAsList(ArrayValue** out_value) WARN_UNUSED_RESULT;
  virtual bool GetAsList(const ArrayValue** out_value) const WARN_UNUSED_RESULT;
  virtual bool GetAsByteArray(byte_array_t* out_value) const WARN_UNUSED_RESULT;
  virtual bool GetAsSet(SetValue** out_value) WARN_UNUSED_RESULT;
  virtual bool GetAsSet(const SetValue** out_value) const WARN_UNUSED_RESULT;
  virtual bool GetAsZSet(ZSetValue** out_value) WARN_UNUSED_RESULT;
  virtual bool GetAsZSet(const ZSetValue** out_value) const WARN_UNUSED_RESULT;
  virtual bool GetAsHash(HashValue** out_value) WARN_UNUSED_RESULT;
  virtual bool GetAsHash(const HashValue** out_value) const WARN_UNUSED_RESULT;

  virtual Value* DeepCopy() const;

  virtual bool Equals(const Value* other) const;

  static bool Equals(const Value* a, const Value* b);

 protected:
  explicit Value(Type type);
  Value(const Value& that);
  Value& operator=(const Value& that);

 private:
  Value(Value&& other);
  Value& operator=(Value&& other);
  Type type_;
};

class FundamentalValue : public Value {
 public:
  explicit FundamentalValue(bool in_value);
  explicit FundamentalValue(int in_value);
  explicit FundamentalValue(unsigned int in_value);
  explicit FundamentalValue(long in_value);
  explicit FundamentalValue(unsigned long in_value);
  explicit FundamentalValue(long long in_value);
  explicit FundamentalValue(unsigned long long in_value);
  explicit FundamentalValue(double in_value);

  virtual ~FundamentalValue();

  virtual bool GetAsBoolean(bool* out_value) const override WARN_UNUSED_RESULT;
  virtual bool GetAsInteger(int* out_value) const override WARN_UNUSED_RESULT;
  virtual bool GetAsUInteger(unsigned int* out_value) const override WARN_UNUSED_RESULT;
  virtual bool GetAsLongInteger(long* out_value) const override WARN_UNUSED_RESULT;
  virtual bool GetAsULongInteger(unsigned long* out_value) const override WARN_UNUSED_RESULT;
  virtual bool GetAsLongLongInteger(long long* out_value) const override WARN_UNUSED_RESULT;
  virtual bool GetAsULongLongInteger(unsigned long long* out_value) const override WARN_UNUSED_RESULT;
  virtual bool GetAsDouble(double* out_value) const override WARN_UNUSED_RESULT;
  virtual FundamentalValue* DeepCopy() const override;
  virtual bool Equals(const Value* other) const override;

 private:
  union {
    bool boolean_value_;
    int integer_value_;
    double double_value_;
    long long_integer_value_;
    long long long_long_integer_value_;
  };
  DISALLOW_COPY_AND_ASSIGN(FundamentalValue);
};

class StringValue : public Value {
 public:
  explicit StringValue(const std::string& in_value);
  explicit StringValue(const string16& in_value);
  virtual ~StringValue();

  virtual bool GetAsString(std::string* out_value) const override WARN_UNUSED_RESULT;
  virtual StringValue* DeepCopy() const override;
  virtual bool Equals(const Value* other) const override;

 private:
  std::string value_;
  DISALLOW_COPY_AND_ASSIGN(StringValue);
};

class ArrayValue : public Value {
 public:
  typedef std::vector<Value*> ValueVector;

  typedef ValueVector::iterator iterator;
  typedef ValueVector::const_iterator const_iterator;

  ArrayValue();
  virtual ~ArrayValue();

  void clear();

  size_t GetSize() const { return list_.size(); }

  // Returns whether the list is empty.
  bool IsEmpty() const { return list_.empty(); }

  bool Set(size_t index, Value* in_value);

  bool Get(size_t index, const Value** out_value) const WARN_UNUSED_RESULT;
  bool Get(size_t index, Value** out_value) WARN_UNUSED_RESULT;

  bool GetBoolean(size_t index, bool* out_value) const WARN_UNUSED_RESULT;
  bool GetInteger(size_t index, int* out_value) const WARN_UNUSED_RESULT;
  bool GetDouble(size_t index, double* out_value) const WARN_UNUSED_RESULT;
  bool GetString(size_t index, std::string* out_value) const WARN_UNUSED_RESULT;
  bool GetList(size_t index, const ArrayValue** out_value) const WARN_UNUSED_RESULT;
  bool GetList(size_t index, ArrayValue** out_value) WARN_UNUSED_RESULT;

  virtual bool Remove(size_t index, std::unique_ptr<Value>* out_value);
  iterator Erase(iterator iter, std::unique_ptr<Value>* out_value);

  bool Remove(const Value& value, size_t* index);

  // Appends a Value to the end of the list.
  void Append(Value* in_value);

  // Convenience forms of Append.
  void AppendBoolean(bool in_value);
  void AppendInteger(int in_value);
  void AppendDouble(double in_value);
  void AppendString(const std::string& in_value);
  void AppendStrings(const std::vector<std::string>& in_values);

  bool AppendIfNotPresent(Value* in_value);

  bool Insert(size_t index, Value* in_value);

  const_iterator Find(const Value& value) const;

  // Swaps contents with the |other| list.
  virtual void Swap(ArrayValue* other);

  // Iteration.
  iterator begin() { return list_.begin(); }
  iterator end() { return list_.end(); }

  const_iterator begin() const { return list_.begin(); }
  const_iterator end() const { return list_.end(); }

  virtual bool GetAsList(ArrayValue** out_value) override WARN_UNUSED_RESULT;
  virtual bool GetAsList(const ArrayValue** out_value) const override WARN_UNUSED_RESULT;
  virtual ArrayValue* DeepCopy() const override;
  virtual bool Equals(const Value* other) const override;

 private:
  ValueVector list_;
  DISALLOW_COPY_AND_ASSIGN(ArrayValue);
};

class ByteArrayValue : public Value {
 public:
  typedef byte_array_t::iterator iterator;
  typedef byte_array_t::const_iterator const_iterator;
  typedef byte_array_t::value_type value_type;

  explicit ByteArrayValue(const byte_array_t& array);
  virtual ~ByteArrayValue();

  void Clear();

  size_t GetSize() const { return array_.size(); }

  // Returns whether the list is empty.
  bool IsEmpty() const { return array_.empty(); }

  // Convenience forms of Append.
  void AppendBoolean(bool in_value);
  void AppendInteger(int in_value);
  void AppendDouble(double in_value);
  void AppendString(const std::string& in_value);
  void AppendStrings(const std::vector<std::string>& in_values);

  // Iteration.
  iterator begin() { return array_.begin(); }
  iterator end() { return array_.end(); }

  const_iterator begin() const { return array_.begin(); }
  const_iterator end() const { return array_.end(); }

  virtual bool GetAsByteArray(byte_array_t* out_value) const override WARN_UNUSED_RESULT;
  virtual ByteArrayValue* DeepCopy() const override;
  virtual bool Equals(const Value* other) const override;

 private:
  byte_array_t array_;
  DISALLOW_COPY_AND_ASSIGN(ByteArrayValue);
};

class SetValue : public Value {
 public:
  typedef std::set<Value*> ValueSet;

  typedef ValueSet::iterator iterator;
  typedef ValueSet::const_iterator const_iterator;

  SetValue();
  virtual ~SetValue();

  void Clear();

  size_t GetSize() const { return set_.size(); }

  // Returns whether the list is empty.
  bool IsEmpty() const { return set_.empty(); }

  // Insert a Value to the set.
  bool Insert(Value* in_value);
  void Insert(const std::string& in_value);

  // Iteration.
  iterator begin() { return set_.begin(); }
  iterator end() { return set_.end(); }

  const_iterator begin() const { return set_.begin(); }
  const_iterator end() const { return set_.end(); }

  virtual bool GetAsSet(SetValue** out_value) override WARN_UNUSED_RESULT;
  virtual bool GetAsSet(const SetValue** out_value) const override WARN_UNUSED_RESULT;
  virtual SetValue* DeepCopy() const override;
  virtual bool Equals(const Value* other) const override;

 private:
  ValueSet set_;
  DISALLOW_COPY_AND_ASSIGN(SetValue);
};

class ZSetValue : public Value {
 public:
  typedef std::map<Value*, Value*> ValueZSet;

  typedef ValueZSet::iterator iterator;
  typedef ValueZSet::const_iterator const_iterator;
  typedef ValueZSet::value_type value_type;

  ZSetValue();
  virtual ~ZSetValue();

  void Clear();

  size_t GetSize() const { return map_.size(); }

  // Returns whether the list is empty.
  bool IsEmpty() const { return map_.empty(); }

  // Insert a Value to the map.
  bool Insert(Value* key, Value* value);
  void Insert(const std::string& key, const std::string& value);

  // Iteration.
  iterator begin() { return map_.begin(); }
  iterator end() { return map_.end(); }

  const_iterator begin() const { return map_.begin(); }
  const_iterator end() const { return map_.end(); }

  virtual bool GetAsZSet(ZSetValue** out_value) override WARN_UNUSED_RESULT;
  virtual bool GetAsZSet(const ZSetValue** out_value) const override WARN_UNUSED_RESULT;
  virtual ZSetValue* DeepCopy() const override;
  virtual bool Equals(const Value* other) const override;

 private:
  ValueZSet map_;
  DISALLOW_COPY_AND_ASSIGN(ZSetValue);
};

typedef ZSetValue MapValue;

class HashValue : public Value {
 public:
  typedef std::unordered_map<Value*, Value*> ValueHash;

  typedef ValueHash::iterator iterator;
  typedef ValueHash::const_iterator const_iterator;
  typedef ValueHash::value_type value_type;

  HashValue();
  virtual ~HashValue();

  void Clear();

  size_t GetSize() const { return hash_.size(); }

  // Returns whether the list is empty.
  bool IsEmpty() const { return hash_.empty(); }

  // Insert a Value to the map.
  bool Insert(Value* key, Value* value);
  void Insert(const std::string& key, const std::string& value);

  // Iteration.
  iterator begin() { return hash_.begin(); }
  iterator end() { return hash_.end(); }

  const_iterator begin() const { return hash_.begin(); }
  const_iterator end() const { return hash_.end(); }

  virtual bool GetAsHash(HashValue** out_value) override WARN_UNUSED_RESULT;
  virtual bool GetAsHash(const HashValue** out_value) const override WARN_UNUSED_RESULT;
  virtual HashValue* DeepCopy() const override;
  virtual bool Equals(const Value* other) const override;

 private:
  ValueHash hash_;
  DISALLOW_COPY_AND_ASSIGN(HashValue);
};

class ErrorValue : public Value {
 public:
  ErrorValue(const std::string& in_value, ErrorsType errorType, logging::LEVEL_LOG level);

  bool IsError() const;

  ErrorsType GetErrorType() const;
  logging::LEVEL_LOG GetLevel() const;
  std::string GetDescription() const;
  void SetDescription(const std::string& descr);

  virtual bool GetAsError(ErrorValue* out_value) const override WARN_UNUSED_RESULT;
  virtual ErrorValue* DeepCopy() const override;
  virtual ~ErrorValue();

 private:
  std::string description_;
  ErrorsType error_type_;
  logging::LEVEL_LOG level_;
  DISALLOW_COPY_AND_ASSIGN(ErrorValue);
};

typedef std::shared_ptr<Value> ValueSPtr;

std::ostream& operator<<(std::ostream& out, const Value& value);

inline std::ostream& operator<<(std::ostream& out, const FundamentalValue& value) {
  return out << static_cast<const Value&>(value);
}

inline std::ostream& operator<<(std::ostream& out, const StringValue& value) {
  return out << static_cast<const Value&>(value);
}

inline std::ostream& operator<<(std::ostream& out, const ArrayValue& value) {
  return out << static_cast<const Value&>(value);
}

}  // namespace common
