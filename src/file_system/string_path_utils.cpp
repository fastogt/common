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

#include <common/file_system/string_path_utils.h>

#if defined(OS_WIN)
#include <fileapi.h>
#else
#include <sys/stat.h>
#endif
#include <unistd.h>

#include <common/convert2string.h>
#include <common/string_util.h>

namespace common {
namespace file_system {
namespace {
template <typename CharT>
bool realpath_without_exist(const std::basic_string<CharT>& path, std::basic_string<CharT>* realpath) {
  typedef std::basic_string<CharT> string_t;
  if (path.empty() || !realpath) {
    return false;
  }

  std::vector<string_t> parts;
  std::vector<string_t> valid_parts;
  size_t count_del = Tokenize(path, get_separator_string<CharT>(), &parts);
  for (size_t i = 0; i < count_del; ++i) {
    string_t part = parts[i];
    if (part == ".") {
      continue;
    } else if (part == "..") {
      valid_parts.pop_back();
      if (valid_parts.empty()) {
        return false;
      }
      continue;
    }

    valid_parts.push_back(part);
  }

#if defined(OS_WIN)
  *realpath = JoinString(valid_parts, get_separator_string<CharT>());
#else
  *realpath = get_separator_string<CharT>() + JoinString(valid_parts, get_separator_string<CharT>());
#endif
  return true;
}

template <typename CharT>
bool realpath_without_exist(const std::basic_string<CharT>& start_dir,
                            const std::basic_string<CharT>& relative_path,
                            std::basic_string<CharT>* realpath) {
  typedef std::basic_string<CharT> string_t;
  string_t path = fixup_separator_in_path(relative_path);
  string_t stabled_dir = stable_dir_path(start_dir);
  if (stabled_dir.empty() || path.empty() || !realpath) {
    return false;
  }

  return realpath_without_exist(stabled_dir + path, realpath);
}

}  // namespace

std::string remove_dots_from_path(const std::string& path) {
  std::string real_path;
  if (!realpath_without_exist(path, &real_path)) {
    return std::string();
  }

  return prepare_path(real_path);
}

template <>
std::string absolute_path_from_relative(const std::string& path, const std::string& start_dir) {
  std::string real_path;
  if (!realpath_without_exist(start_dir, path, &real_path)) {
    return std::string();
  }

  return prepare_path(real_path);
}

template <>
string16 absolute_path_from_relative(const string16& path, const string16& start_dir) {
  std::string rp = absolute_path_from_relative(ConvertToString(path), ConvertToString(start_dir));
  return ConvertToString16(rp);
}

template <>
tribool is_directory(const std::string& path) {
  if (!is_valid_path(path)) {
    return INDETERMINATE;
  }

  std::string p_path = prepare_path(path);
#if defined(OS_WIN)
  DWORD ftype = ::GetFileAttributesA(p_path.c_str());
  if (ftype == INVALID_FILE_ATTRIBUTES) {
    return INDETERMINATE;
  }

  return (ftype & FILE_ATTRIBUTE_DIRECTORY) ? SUCCESS : FAIL;
#else
  struct stat filestat;
  if (::stat(p_path.c_str(), &filestat) != ERROR_RESULT_VALUE) {
    return S_ISDIR(filestat.st_mode) ? SUCCESS : FAIL;
  }
  return INDETERMINATE;
#endif
}

template <>
tribool is_directory(const string16& path) {
  return is_directory(ConvertToString(path));
}

std::string pwd() {
  char cwd[1024] = {0};
  char* ret = getcwd(cwd, sizeof(cwd));
  if (!ret) {
    return std::string();
  }

  return stable_dir_path(std::string(cwd));
}

std::string app_pwd() {
#if defined(OS_MACOSX)
  return bundle_pwd();
#else
  return pwd();
#endif
}

}  // namespace file_system
}  // namespace common
