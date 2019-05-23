/*  Copyright (C) 2014-2018 FastoGT. All right reserved.

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

#include <common/file_system/file_system.h>
#include <common/file_system/file_system_utils.h>
#include <common/file_system/string_path_utils.h>

#include <common/time.h>
#include <common/utf_string_conversions.h>

#ifdef OS_WIN
#define EXIST_EXE "sh.exe"
#else
#define EXIST_EXE "sh"
#endif

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

  auto result_ascii =
      common::file_system::ScanFolder(common::file_system::ascii_directory_string_path(home_test), "*.txt", true);
  ASSERT_EQ(result_ascii.size(), 1);

  auto result_utf =
      common::file_system::ScanFolder(common::file_system::utf_directory_string_path(common::UTF8ToUTF16(home_test)),
                                      common::UTF8ToUTF16("*.txt"), true);
  ASSERT_EQ(result_utf.size(), 1);

  err = common::file_system::remove_directory(home_test_test_test, false);
  ASSERT_TRUE(!err);

  is_exist = common::file_system::is_directory_exist(home_test_test_test);
  ASSERT_FALSE(is_exist);

  err = common::file_system::remove_directory(home_test, true);
  ASSERT_TRUE(!err);
  is_exist = common::file_system::is_directory_exist(home_test);
  ASSERT_FALSE(is_exist);

  std::string exe_path;
  bool found = common::file_system::find_file_in_path(EXIST_EXE, &exe_path);
  ASSERT_TRUE(found);
}
