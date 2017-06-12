#include <gtest/gtest.h>

#include <common/error.h>

TEST(Error, ErrorOnlyIFIsErrorSet) {
  common::Error err;
  ASSERT_TRUE(!err);  // not error

  err = common::make_error_value_errno(EINVAL, common::Value::E_ERROR);
  ASSERT_TRUE(err && err->IsError());  // error
}
