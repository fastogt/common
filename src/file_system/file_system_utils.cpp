/*  Copyright (C) 2014-2021 FastoGT. All right reserved.

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

#include <common/file_system/file_system_utils.h>

#include <dirent.h>  // for closedir, readdir, DIR, DT_REG, dirent

#include <common/convert2string.h>
#include <common/string_util.h>

#if defined(OS_WIN)
#include <common/file_system/string_path_utils.h>
#endif

namespace common {
namespace file_system {

namespace {
template <typename String>
String ConvertFromCharArray(const char* str);

template <>
std::string ConvertFromCharArray(const char* str) {
  return str;
}

template <>
string16 ConvertFromCharArray(const char* str) {
  return ConvertToString16(str);
}

template <typename CharT, typename Traits = std::char_traits<CharT>>
void DoScanFolder(const DirectoryStringPath<CharT, Traits>& folder,
                  const std::basic_string<CharT, Traits>& pattern,
                  bool recursive,
                  std::function<bool(const FileStringPath<CharT, Traits>&)> filter_predicate,
                  std::vector<FileStringPath<CharT, Traits>>* result) {
  typedef typename DirectoryStringPath<CharT, Traits>::value_type value_type;
  if (!folder.IsValid() || pattern.empty() || !result) {
    return;
  }

  std::string folder_str = ConvertToString(folder.GetPath());
  DIR* dirp = opendir(folder_str.c_str());
  if (!dirp) {
    return;
  }

  struct dirent* dent;
  while ((dent = readdir(dirp)) != nullptr) {
    /* Skip the names "." and ".." as we don't want to recurse on them. */
    if (!strcmp(dent->d_name, ".") || !strcmp(dent->d_name, "..")) {
      continue;
    }

#if defined(OS_WIN)
    const std::string dir_str = make_path(folder_str, dent->d_name);
    tribool is_dir_tr = is_directory(dir_str);
    if (is_dir_tr == INDETERMINATE) {
      continue;
    }
    bool is_dir = is_dir_tr == SUCCESS;
#else
    bool is_dir = dent->d_type == DT_DIR;
#endif
    if (!is_dir) {
      if (MatchPattern(ConvertFromCharArray<value_type>(dent->d_name), pattern)) {
        const value_type vt = ConvertFromCharArray<value_type>(dent->d_name);
        auto file = folder.MakeFileStringPath(vt);
        if (file) {
          if (filter_predicate) {
            if (!filter_predicate(*file)) {
              continue;
            }
          }
          result->push_back(*file);
        }
      }
    } else if (recursive) {
      const value_type vt = ConvertFromCharArray<value_type>(dent->d_name);
      auto dir = folder.MakeDirectoryStringPath(vt);
      if (dir) {
        DoScanFolder(*dir, pattern, recursive, filter_predicate, result);
      }
    }
  }

  closedir(dirp);
}

}  // namespace

template <typename CharT, typename Traits>
std::vector<FileStringPath<CharT, Traits>> ScanFolder(
    const DirectoryStringPath<CharT, Traits>& folder,
    const std::basic_string<CharT, Traits>& pattern,
    bool recursive,
    std::function<bool(const FileStringPath<CharT, Traits>&)> filter_predicate) {
  std::vector<FileStringPath<CharT, Traits>> result;
  DoScanFolder(folder, pattern, recursive, filter_predicate, &result);
  return result;
}

// Instantiate versions we know callers will need.
template std::vector<ascii_file_string_path> ScanFolder<char>(
    const ascii_directory_string_path& folder,
    const std::string& pattern,
    bool recursive,
    std::function<bool(const ascii_file_string_path&)> filter_predicate);
template std::vector<utf_file_string_path> ScanFolder<char16, string16_char_traits>(
    const utf_directory_string_path& folder,
    const std::basic_string<char16, string16_char_traits>& pattern,
    bool recursive,
    std::function<bool(const utf_file_string_path&)> filter_predicate);

}  // namespace file_system
}  // namespace common
