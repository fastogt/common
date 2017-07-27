#include <gtest/gtest.h>

#include <common/value.h>

template <typename T, typename U>
void CheckValue(U* (*create_ptr)(T t), bool (U::*get_ptr)(T* t) const, T val, common::Value::Type vt) {
  U* cval = create_ptr(val);
  ASSERT_TRUE(cval && cval->GetType() == vt);
  T val_res;
  ASSERT_TRUE((cval->*get_ptr)(&val_res));
  ASSERT_EQ(val_res, val);
  delete cval;
}

TEST(Value, create_and_get_simple_type) {
  common::Value* val_null = common::Value::CreateNullValue();
  ASSERT_TRUE(val_null && val_null->GetType() == common::Value::TYPE_NULL);

  CheckValue(&common::Value::CreateBooleanValue, &common::FundamentalValue::GetAsBoolean, true,
             common::Value::TYPE_BOOLEAN);
  CheckValue(&common::Value::CreateIntegerValue, &common::FundamentalValue::GetAsInteger, 11,
             common::Value::TYPE_INTEGER);
  CheckValue(&common::Value::CreateUIntegerValue, &common::FundamentalValue::GetAsUInteger, 321U,
             common::Value::TYPE_UINTEGER);
  CheckValue(&common::Value::CreateLongIntegerValue, &common::FundamentalValue::GetAsLongInteger, 1341L,
             common::Value::TYPE_LONG_INTEGER);
  CheckValue(&common::Value::CreateULongIntegerValue, &common::FundamentalValue::GetAsULongInteger, 3231UL,
             common::Value::TYPE_ULONG_INTEGER);
  CheckValue(&common::Value::CreateLongLongIntegerValue, &common::FundamentalValue::GetAsLongLongInteger, 161LL,
             common::Value::TYPE_LONG_LONG_INTEGER);
  CheckValue(&common::Value::CreateULongLongIntegerValue, &common::FundamentalValue::GetAsULongLongInteger, 3211ULL,
             common::Value::TYPE_ULONG_LONG_INTEGER);
  CheckValue(&common::Value::CreateDoubleValue, &common::FundamentalValue::GetAsDouble, 11.5,
             common::Value::TYPE_DOUBLE);
}
