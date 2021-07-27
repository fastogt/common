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

#include <gtest/gtest.h>

#include <common/value.h>

template <typename T, typename U>
void CheckValue(U* (*create_ptr)(T t), bool (U::*get_ptr)(T* t) const, T val, common::Value::Type vt) {
  U* cval = create_ptr(val);
  ASSERT_TRUE(cval && cval->GetType() == vt);
  T val_res;
  ASSERT_TRUE((cval->*get_ptr)(&val_res));
  ASSERT_EQ(val_res, val);

  U* copy = cval->DeepCopy();
  T val_res2;
  ASSERT_TRUE((copy->*get_ptr)(&val_res2));
  ASSERT_EQ(val_res, val_res2);
  delete copy;

  delete cval;
}

TEST(Value, create_and_get_simple_type) {
  common::Value* val_null = common::Value::CreateNullValue();
  ASSERT_TRUE(val_null && val_null->GetType() == common::Value::TYPE_NULL);
  delete val_null;

  CheckValue(&common::Value::CreateBooleanValue, &common::FundamentalValue::GetAsBoolean, true,
             common::Value::TYPE_BOOLEAN);
  CheckValue(&common::Value::CreateInteger32Value, &common::FundamentalValue::GetAsInteger32, 11,
             common::Value::TYPE_INTEGER32);
  CheckValue(&common::Value::CreateUInteger32Value, &common::FundamentalValue::GetAsUInteger32, 321U,
             common::Value::TYPE_UINTEGER32);
  CheckValue(&common::Value::CreateInteger64Value, &common::FundamentalValue::GetAsInteger64, 1341L,
             common::Value::TYPE_INTEGER64);
  CheckValue(&common::Value::CreateUInteger64Value, &common::FundamentalValue::GetAsUInteger64, 3231UL,
             common::Value::TYPE_UINTEGER64);
  CheckValue(&common::Value::CreateDoubleValue, &common::FundamentalValue::GetAsDouble, 11.5,
             common::Value::TYPE_DOUBLE);
}

TEST(Value, create_and_get_complex_type) {
  const common::Value::string_t data = {'d', 'a', 't', 'a'};
  common::Value* val_string = common::Value::CreateStringValue(data);
  ASSERT_TRUE(val_string && val_string->GetType() == common::Value::TYPE_STRING);
  common::Value::string_t data2;
  ASSERT_TRUE(val_string->GetAsString(&data2));
  ASSERT_EQ(data, data2);
  delete val_string;

  common::Value* val_arr = common::Value::CreateArrayValue();
  ASSERT_TRUE(val_arr && val_arr->GetType() == common::Value::TYPE_ARRAY);
  delete val_arr;

  const common::byte_array_t bt = {0, 1};
  common::Value* val_barr = common::Value::CreateByteArrayValue(bt);
  ASSERT_TRUE(val_barr && val_barr->GetType() == common::Value::TYPE_BYTE_ARRAY);
  common::byte_array_t bt2;
  ASSERT_TRUE(val_barr->GetAsByteArray(&bt2));
  ASSERT_EQ(bt2, bt);
  delete val_barr;

  common::Value* val_set = common::Value::CreateSetValue();
  ASSERT_TRUE(val_set && val_set->GetType() == common::Value::TYPE_SET);
  delete val_set;

  common::Value* val_zset = common::Value::CreateZSetValue();
  ASSERT_TRUE(val_zset && val_zset->GetType() == common::Value::TYPE_ZSET);
  delete val_zset;

  common::Value* val_hash = common::Value::CreateHashValue();
  ASSERT_TRUE(val_hash && val_hash->GetType() == common::Value::TYPE_HASH);
  delete val_hash;
}
