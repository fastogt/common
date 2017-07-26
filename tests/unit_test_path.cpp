#include <gtest/gtest.h>

#include <common/file_system.h>

TEST(Path, is_valid_path) {
  std::string invalid_path0;
  ASSERT_FALSE(common::file_system::is_valid_path(invalid_path0));

  std::string invalid_path1 = "CD:\\";
  ASSERT_FALSE(common::file_system::is_valid_path(invalid_path1));

  std::string invalid_path2 = "/~";
  ASSERT_FALSE(common::file_system::is_valid_path(invalid_path2));

  std::string invalid_path5 = "home/";
  ASSERT_FALSE(common::file_system::is_valid_path(invalid_path5));

  std::string valid_path0 = "/";
  ASSERT_TRUE(common::file_system::is_valid_path(valid_path0));

  std::string valid_path1 = "/home/sasha";
  ASSERT_TRUE(common::file_system::is_valid_path(valid_path1));

  std::string valid_path2 = "/home/sas/";
  ASSERT_TRUE(common::file_system::is_valid_path(valid_path2));

  std::string valid_path3 = "~";
  ASSERT_TRUE(common::file_system::is_valid_path(valid_path3));

  std::string valid_path4 = "F:\\alex.txt";
  ASSERT_TRUE(common::file_system::is_valid_path(valid_path4));

  std::string valid_path5 = "F:\\home\\alex2.txt";
  ASSERT_TRUE(common::file_system::is_valid_path(valid_path5));

  std::string valid_path6 = "F:/home\\alex2.txt";
  ASSERT_TRUE(common::file_system::is_valid_path(valid_path6));

  std::string valid_path7 = "C:/";
  ASSERT_TRUE(common::file_system::is_valid_path(valid_path7));

  std::string valid_path8 = "D:\\";
  ASSERT_TRUE(common::file_system::is_valid_path(valid_path8));

  std::string valid_path9 = "C://";
  ASSERT_TRUE(common::file_system::is_valid_path(valid_path9));

  std::string valid_path10 = "C:/\\";
  ASSERT_TRUE(common::file_system::is_valid_path(valid_path10));

  std::string valid_path11 = "//";
  ASSERT_TRUE(common::file_system::is_valid_path(valid_path11));

  std::string valid_path12 = "/home//";
  ASSERT_TRUE(common::file_system::is_valid_path(valid_path12));
}

TEST(Path, stable_path) {
  std::string valid_path8 = "D:\\\\\\";
  ASSERT_EQ(common::file_system::stable_dir_path(valid_path8), "D:/");

  std::string valid_path9 = "C://";
  ASSERT_EQ(common::file_system::stable_dir_path(valid_path9), "C:/");

  std::string valid_path10 = "C:/\\";
  ASSERT_EQ(common::file_system::stable_dir_path(valid_path10), "C:/");

  std::string valid_path11 = "//";
  ASSERT_EQ(common::file_system::stable_dir_path(valid_path11), "/");

  std::string valid_path12 = "/home//";
  ASSERT_EQ(common::file_system::stable_dir_path(valid_path12), "/home/");
}
