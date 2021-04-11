#include <gtest/gtest.h>

#include <common/file_system/path.h>
#include <common/file_system/string_path_utils.h>

#include <common/utf_string_conversions.h>

#define IMG_OFFLINE_CHANNEL_PATH_RELATIVE "share/resources/offline_channel.png"
#define IMG_CONNECTION_ERROR_PATH_RELATIVE "share/resources/connection_error.png"
#define MAIN_FONT_PATH_RELATIVE "share/fonts/FreeSans.ttf"
#define RELATIVE_SOURCE_DIR ".."

TEST(file_system_path, is_absolute_path) {
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

TEST(file_system_path, directory) {
  common::file_system::ascii_string_path com("/home/sasha");  // file
  ASSERT_TRUE(com.IsValid());
  ASSERT_EQ(com.GetDirectory(), "/home/");
  ASSERT_EQ(com.GetParentDirectory(), "/home/");

  const std::string st_dir = common::file_system::stable_dir_path(std::string("/home/sasha"));
  ASSERT_EQ(st_dir, "/home/sasha/");
  common::file_system::ascii_directory_string_path com_dir(st_dir);  // directory
  ASSERT_TRUE(com_dir.IsValid());
  ASSERT_EQ(com_dir.GetDirectory(), "/home/sasha/");
  ASSERT_EQ(com_dir.GetParentDirectory(), "/home/");
  ASSERT_EQ(com_dir.GetFolderName(), "sasha");

  common::file_system::ascii_directory_string_path com_dir_1l("/home/");
  ASSERT_TRUE(com_dir_1l.IsValid());
  ASSERT_EQ(com_dir_1l.GetDirectory(), "/home/");
  ASSERT_EQ(com_dir_1l.GetParentDirectory(), "/");
  ASSERT_EQ(com_dir_1l.GetFolderName(), "home");

  common::file_system::ascii_directory_string_path root("/");
  ASSERT_TRUE(root.IsValid());
  ASSERT_EQ(root.GetDirectory(), "/");
  ASSERT_EQ(root.GetParentDirectory(), "/");
  ASSERT_EQ(root.GetFolderName(), "");
}

TEST(file_system_path, filename) {
#ifdef OS_POSIX
  const std::string home = getenv("HOME");
#else
  const std::string home = getenv("USERPROFILE");
#endif
  common::file_system::ascii_file_string_path com("~/1.txt");
  ASSERT_TRUE(com.IsValid());
  ASSERT_EQ(com.GetDirectory(), common::file_system::stable_dir_path(home));
  ASSERT_EQ(com.GetFileName(), "1.txt");
  ASSERT_EQ(com.GetExtension(), "txt");
  ASSERT_EQ(com.GetBaseFileName(), "1");

  const std::string dir_str = com.GetDirectory();
  common::file_system::ascii_directory_string_path home_dir(dir_str);
  ASSERT_TRUE(home_dir.IsValid());
  ASSERT_EQ(home_dir.GetPath(), common::file_system::stable_dir_path(home));
  ASSERT_EQ(home_dir.GetFolderName(), common::file_system::get_file_or_dir_name(home));

  common::file_system::ascii_file_string_path mp4("~/The.Willoughbys.2020.HDRip.XviD.AC3-EVO.mp4");
  ASSERT_TRUE(mp4.IsValid());
  ASSERT_EQ(mp4.GetDirectory(), common::file_system::stable_dir_path(home));
  ASSERT_EQ(mp4.GetFileName(), "The.Willoughbys.2020.HDRip.XviD.AC3-EVO.mp4");
  ASSERT_EQ(mp4.GetExtension(), "mp4");
  ASSERT_EQ(mp4.GetBaseFileName(), "The.Willoughbys.2020.HDRip.XviD.AC3-EVO");
}

TEST(file_system_path, make_node) {
  common::file_system::utf_directory_string_path com(common::UTF8ToUTF16("/home/саша"));
  ASSERT_TRUE(com.IsValid());
  auto invalid_utf_file = com.MakeFileStringPath(common::UTF8ToUTF16("паша/"));
  ASSERT_FALSE(invalid_utf_file);
  auto valid_utf_file = com.MakeFileStringPath(common::UTF8ToUTF16("паша"));
  ASSERT_TRUE(valid_utf_file);
}
