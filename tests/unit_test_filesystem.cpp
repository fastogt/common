#include <gtest/gtest.h>

#include <common/file_system/file_system.h>
#include <common/file_system/file_system_utils.h>
#include <common/file_system/string_path_utils.h>

#include <common/time.h>
#include <common/utf_string_conversions.h>

TEST(file_system, CreateRemoveDirectoryRecursive) {
  const std::string home = getenv("HOME");
  bool is_exist = common::file_system::is_directory_exist(home);
  ASSERT_TRUE(is_exist);

  static const std::string test = "/паша";
  const std::string home_test = home + test;
  common::ErrnoError err = common::file_system::create_directory(home_test, true);
  ASSERT_TRUE(!err);

  is_exist = common::file_system::is_directory_exist(home_test);
  ASSERT_TRUE(is_exist);

  const std::string home_test_test_test = home_test + test + test;
  err = common::file_system::create_directory(home_test_test_test, true);
  ASSERT_TRUE(!err);
  is_exist = common::file_system::is_directory_exist(home_test_test_test);
  ASSERT_TRUE(is_exist);

  const std::string home_test_test_tt = home_test + test + "/саша.txt";
  err = common::file_system::remove_file(home_test_test_tt);
  ASSERT_TRUE(!err);
  err = common::file_system::create_node(home_test_test_tt);
  ASSERT_TRUE(!err);

  common::utctime_t sec;
  common::time64_t cur_utc = common::time::current_utc_mstime();
  err = common::file_system::get_file_time_last_modification(home_test_test_tt, &sec);
  ASSERT_TRUE(!err);
  ASSERT_EQ(cur_utc / 1000, sec);

  is_exist = common::file_system::is_file_exist(home_test_test_tt);
  ASSERT_TRUE(is_exist);

  auto result_ascii = common::file_system::ScanFolder(common::file_system::ascii_directory_string_path(home_test),
                                                      std::string(".txt"), true);
  ASSERT_EQ(result_ascii.size(), 1);

  auto result_utf =
      common::file_system::ScanFolder(common::file_system::utf_directory_string_path(common::UTF8ToUTF16(home_test)),
                                      common::UTF8ToUTF16(".txt"), true);
  ASSERT_EQ(result_utf.size(), 1);

  err = common::file_system::remove_directory(home_test_test_test, false);
  ASSERT_TRUE(!err);

  is_exist = common::file_system::is_directory_exist(home_test_test_test);
  ASSERT_FALSE(is_exist);

  err = common::file_system::remove_directory(home_test, true);
  ASSERT_TRUE(!err);
  is_exist = common::file_system::is_directory_exist(home_test);
  ASSERT_FALSE(is_exist);
}
