#include <gtest/gtest.h>

#include <common/file_system.h>

#define IMG_OFFLINE_CHANNEL_PATH_RELATIVE "share/resources/offline_channel.png"
#define IMG_CONNECTION_ERROR_PATH_RELATIVE "share/resources/connection_error.png"
#define MAIN_FONT_PATH_RELATIVE "share/fonts/FreeSans.ttf"
#define RELATIVE_SOURCE_DIR ".."

TEST(FileSystem, is_absolute_path) {
  ASSERT_TRUE(common::file_system::is_relative_path(RELATIVE_SOURCE_DIR));
  ASSERT_FALSE(common::file_system::is_absolute_path(RELATIVE_SOURCE_DIR));

  const std::string relative_path = "./algo/aks";
  ASSERT_TRUE(common::file_system::is_relative_path(relative_path));
  ASSERT_FALSE(common::file_system::is_absolute_path(relative_path));

  const std::string abs_path = common::file_system::absolute_path_from_relative(relative_path);
  ASSERT_TRUE(common::file_system::is_absolute_path(abs_path));
}

TEST(ascii_string_path, IsValid) {
  common::file_system::ascii_string_path invalid;
  ASSERT_FALSE(invalid.IsValid());

  common::file_system::ascii_string_path root("/");
  ASSERT_TRUE(root.IsValid());

  common::file_system::ascii_string_path win_root("C:\\");
  ASSERT_TRUE(win_root.IsValid());

  common::file_system::ascii_string_path home("~");
  ASSERT_TRUE(home.IsValid());
}

TEST(ascii_string_path, directory) {
  common::file_system::ascii_string_path com("/home/sasha");  // file
  ASSERT_TRUE(com.IsValid());
  ASSERT_EQ(com.GetDirectory(), "/home/");
  ASSERT_EQ(com.GetParentDirectory(), "/home/");

  const std::string st_dir = common::file_system::stable_dir_path(std::string("/home/sasha"));
  ASSERT_EQ(st_dir, "/home/sasha/");
  common::file_system::ascii_string_path com_dir(st_dir);  // directory
  ASSERT_TRUE(com_dir.IsValid());
  ASSERT_EQ(com_dir.GetDirectory(), "/home/sasha/");
  ASSERT_EQ(com_dir.GetParentDirectory(), "/home/");

  common::file_system::ascii_string_path com_dir_1l("/home/");
  ASSERT_TRUE(com_dir_1l.IsValid());
  ASSERT_EQ(com_dir_1l.GetDirectory(), "/home/");
  ASSERT_EQ(com_dir_1l.GetParentDirectory(), "/");

  common::file_system::ascii_string_path root("/");
  ASSERT_TRUE(root.IsValid());
  ASSERT_EQ(root.GetDirectory(), "/");
  ASSERT_EQ(root.GetParentDirectory(), "/");
}

TEST(ascii_string_path, filename) {
#ifdef OS_POSIX
  const std::string home = getenv("HOME");
#else
  const std::string home = getenv("USERPROFILE");
#endif
  common::file_system::ascii_string_path com("~/1.txt");
  ASSERT_TRUE(com.IsValid());
  ASSERT_EQ(com.GetDirectory(), common::file_system::stable_dir_path(home));
  ASSERT_EQ(com.GetFileName(), "1.txt");
  ASSERT_EQ(com.GetExtension(), "txt");
}

TEST(Path, CreateRemoveDirectoryRecursive) {
  const std::string home = getenv("HOME");
  bool isExist = common::file_system::is_directory_exist(home);
  ASSERT_TRUE(isExist);

  static const std::string test = "/test";
  const std::string home_test = home + test;
  common::ErrnoError err = common::file_system::create_directory(home_test, true);
  ASSERT_TRUE(!err);

  isExist = common::file_system::is_directory_exist(home_test);
  ASSERT_TRUE(isExist);

  const std::string home_test_test_test = home_test + test + test;
  err = common::file_system::create_directory(home_test_test_test, true);
  ASSERT_TRUE(!err);
  isExist = common::file_system::is_directory_exist(home_test_test_test);
  ASSERT_TRUE(isExist);

  const std::string home_test_test_tt = home_test + test + "/tt";
  err = common::file_system::remove_file(home_test_test_tt);
  ASSERT_TRUE(!err);
  err = common::file_system::create_node(home_test_test_tt);
  ASSERT_TRUE(!err);
  isExist = common::file_system::is_file_exist(home_test_test_tt);
  ASSERT_TRUE(isExist);

  err = common::file_system::remove_directory(home_test_test_test, false);
  ASSERT_TRUE(!err);

  isExist = common::file_system::is_directory_exist(home_test_test_test);
  ASSERT_FALSE(isExist);

  err = common::file_system::remove_directory(home_test, true);
  ASSERT_TRUE(!err);
  isExist = common::file_system::is_directory_exist(home_test);
  ASSERT_FALSE(isExist);
}
