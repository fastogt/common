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

#pragma once

#include <common/file_system/types.h>

namespace common {
namespace file_system {

// is_directory
template <typename CharT, typename Traits = std::char_traits<CharT>>
tribool is_directory(const std::basic_string<CharT, Traits>& path);

template <typename CharT, typename Traits = std::char_traits<CharT>>
tribool is_directory(const CharT* path) {
  return is_directory(std::basic_string<CharT, Traits>(path));
}

// is_file
template <typename CharT, typename Traits = std::char_traits<CharT>>
tribool is_file(const std::basic_string<CharT, Traits>& path) {
  tribool result = is_directory(path);
  if (result != INDETERMINATE) {
    result = result == SUCCESS ? FAIL : SUCCESS;
  }
  return result;
}

template <typename CharT, typename Traits = std::char_traits<CharT>>
bool is_file(const CharT* path) {
  return is_file(std::basic_string<CharT, Traits>(path));
}

// is_file_exist
template <typename CharT, typename Traits = std::char_traits<CharT>>
bool is_file_exist(const std::basic_string<CharT, Traits>& path) {
  return is_file(path) == SUCCESS;
}

template <typename CharT, typename Traits = std::char_traits<CharT>>
bool is_file_exist(const CharT* path) {
  return is_file_exist(std::basic_string<CharT, Traits>(path));
}

// is_directory_exist
template <typename CharT, typename Traits = std::char_traits<CharT>>
bool is_directory_exist(const std::basic_string<CharT, Traits>& path) {
  return is_directory(path) == SUCCESS;
}

template <typename CharT, typename Traits = std::char_traits<CharT>>
bool is_directory_exist(const CharT* path) {
  return is_directory_exist(std::basic_string<CharT, Traits>(path));
}

template <typename CharT, typename Traits = std::char_traits<CharT>>
std::basic_string<CharT, Traits> remove_dots_from_path(const std::basic_string<CharT, Traits>& path);

#if defined(OS_MACOSX)
std::string bundle_pwd();
#endif
std::string pwd();
std::string app_pwd();

// Find the real name of path, by removing all ".", ".."
template <typename CharT, typename Traits = std::char_traits<CharT>>
std::basic_string<CharT, Traits> absolute_path_from_relative(const std::basic_string<CharT, Traits>& relative_path,
                                                             const std::basic_string<CharT, Traits>& start_dir = pwd());
template <typename CharT, typename Traits = std::char_traits<CharT>>
inline std::basic_string<CharT, Traits> absolute_path_from_relative(
    const CharT* path,
    const std::basic_string<CharT, Traits>& start_dir = pwd()) {
  return absolute_path_from_relative(std::basic_string<CharT, Traits>(path), start_dir);
}

}  // namespace file_system
}  // namespace common
