#include <gtest/gtest.h>

#include <common/error.h>

TEST(Error, ErrorOnlyIFIsErrorSet) {
  common::Error err;
  ASSERT_TRUE(!err);  // not error

  common::ErrnoError errn = common::make_error_value_errno(EINVAL, common::Value::E_ERROR);
  ASSERT_TRUE(errn && errn->IsError());  // error
  ASSERT_EQ(errn->GetErrno(), EINVAL);
  ASSERT_NE(errn->GetDescription(), std::string());
  ASSERT_EQ(errn->GetErrorType(), common::Value::E_ERROR);
  ASSERT_EQ(errn->GetLevel(), common::logging::L_ERR);
}
