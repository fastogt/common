#include <gtest/gtest.h>

#include <common/error.h>

#include <common/optional.h>

TEST(Error, ErrorOnlyIFIsErrorSet) {
  common::Error err;
  ASSERT_TRUE(!err);  // not error

  common::ErrnoError errn = common::make_errno_error(EINVAL, common::ERROR_TYPE);
  ASSERT_TRUE(errn);  // error
  ASSERT_EQ(errn->GetErrorCode(), EINVAL);
  ASSERT_NE(errn->GetDescription(), std::string());
  ASSERT_EQ(errn->GetErrorType(), common::ERROR_TYPE);
}

enum FError {
  FILE_ERROR_FAILED = -1,
  FILE_ERROR_IN_USE = -2,
  FILE_ERROR_EXISTS = -3,
  FILE_ERROR_NOT_FOUND = -4,
  FILE_ERROR_ACCESS_DENIED = -5,
  FILE_ERROR_TOO_MANY_OPENED = -6,
  FILE_ERROR_NO_MEMORY = -7,
  FILE_ERROR_NO_SPACE = -8,
  FILE_ERROR_NOT_A_DIRECTORY = -9,
  FILE_ERROR_INVALID_OPERATION = -10,
  FILE_ERROR_SECURITY = -11,
  FILE_ERROR_ABORT = -12,
  FILE_ERROR_NOT_A_FILE = -13,
  FILE_ERROR_NOT_EMPTY = -14,
  FILE_ERROR_INVALID_URL = -15,
  FILE_ERROR_IO = -16,
  // Put new entries here and increment FILE_ERROR_MAX.
  FILE_ERROR_MAX = -17
};

struct FileErrorTraits {
  static std::string GetTextFromErrorCode(FError error) {
    if (error == FILE_ERROR_FAILED) {
      return "Failed";
    }

    return std::string();
  }
};
typedef common::ErrorBase<FError, FileErrorTraits> FileError;
typedef common::Optional<FileError> FileErrors;

enum CError { COMMON_INVALID_INPUT_VALUE = -1 };

struct CommonErrorTraits {
  static std::string GetTextFromErrorCode(CError error) {
    if (error == COMMON_INVALID_INPUT_VALUE) {
      return "Invalid";
    }

    return std::string();
  }
};
typedef common::ErrorBase<CError, CommonErrorTraits> CommonError;
typedef common::Optional<CommonError> CommonErrors;

FileErrors FileOpenFailed() {
  return FileError(FILE_ERROR_FAILED, common::ERROR_TYPE);
}

TEST(ErrorNew, Instance) {
  FileErrors file_no_err;
  ASSERT_FALSE(file_no_err);
  FileErrors file_open_failed = FileOpenFailed();
  ASSERT_TRUE(file_open_failed);
  ASSERT_EQ(file_open_failed->GetErrorCode(), FILE_ERROR_FAILED);
  ASSERT_EQ(file_open_failed->GetErrorType(), common::ERROR_TYPE);
  ASSERT_EQ(file_open_failed->GetDescription(), "Failed");

  CommonErrors common_err;
  ASSERT_FALSE(common_err);
}
