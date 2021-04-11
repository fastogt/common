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

#include <string>

#include <common/types.h>

namespace common {
namespace file_system {

template <typename CharT>
inline CharT get_separator() {
  return '/';
}

template <typename CharT>
inline std::basic_string<CharT> get_separator_string() {
  return "/";
}

template <typename CharT>
inline CharT get_win_separator() {
  return '\\';
}

template <typename CharT>
inline std::basic_string<CharT> get_win_separator_string() {
  return "\\";
}

template <typename CharT>
inline CharT get_home_separator() {
  return '~';
}

template <typename CharT>
inline std::basic_string<CharT> get_home_separator_string() {
  return "~";
}

template <typename CharT, typename Traits = std::char_traits<CharT>>
inline bool is_valid_path(const std::basic_string<CharT, Traits>& path) {
  if (path.empty()) {
    return false;
  }

  size_t len = path.length();
  for (size_t i = 0; i < len; ++i) {
    CharT c = path[i];
    if (c == get_home_separator<CharT>() && i != 0) {  // ~ can be only in zero position
      return false;
    }
  }

  bool res = path[0] == get_separator<CharT>() || path[0] == get_home_separator<CharT>();
  if (!res && path.size() >= 3) {  // maybe windows path
    res = path[1] == ':' && (path[2] == '/' || path[2] == '\\');
  }

  return res;
}

template <typename CharT, typename Traits = std::char_traits<CharT>>
inline tribool is_absolute_path(const std::basic_string<CharT, Traits>& path) {
  if (is_valid_path(path)) {
    return SUCCESS;
  }

  return FAIL;
}

template <typename CharT, typename Traits = std::char_traits<CharT>>
inline tribool is_relative_path(const std::basic_string<CharT, Traits>& path) {
  if (!is_valid_path(path)) {
    return SUCCESS;
  }

  return FAIL;
}

template <typename CharT, typename Traits = std::char_traits<CharT>>
inline tribool is_absolute_path(const CharT* path) {
  return is_absolute_path(std::basic_string<CharT, Traits>(path));
}

template <typename CharT, typename Traits = std::char_traits<CharT>>
inline tribool is_relative_path(const CharT* path) {
  return is_relative_path(std::basic_string<CharT, Traits>(path));
}

template <typename CharT, typename Traits = std::char_traits<CharT>>
inline std::basic_string<CharT, Traits> fixup_separator_in_path(const std::basic_string<CharT, Traits>& path) {
  std::basic_string<CharT, Traits> path_without_dup;
  for (size_t i = 0; i < path.size(); ++i) {
    CharT c = path[i];
    if (c == get_win_separator<CharT>()) {
      c = get_separator<CharT>();
    }

    if (c == get_separator<CharT>() && i + 1 < path.size()) {
      CharT c2 = path[i + 1];
      if (c2 == get_win_separator<CharT>() || c2 == get_separator<CharT>()) {
        continue;
      }
    }
    path_without_dup += c;
  }

  return path_without_dup;
}

template <typename CharT, typename Traits = std::char_traits<CharT>>
std::basic_string<CharT, Traits> getenv(const char* env);

template <typename CharT, typename Traits = std::char_traits<CharT>>
inline std::basic_string<CharT, Traits> prepare_path(std::basic_string<CharT, Traits> path) {  // should be absolute
                                                                                               // path
  if (!is_valid_path(path)) {
    return std::basic_string<CharT, Traits>();
  }

  if (path[0] == get_home_separator<CharT>()) {
#if defined(OS_POSIX)
    std::basic_string<CharT, Traits> home = getenv<CharT, Traits>("HOME");
#else
    std::basic_string<CharT, Traits> home = getenv<CharT, Traits>("USERPROFILE");
#endif
    if (!home.empty()) {
      std::basic_string<CharT, Traits> tmp = path;
      path = home;
      path += tmp.substr(1);
    }
  }

  return fixup_separator_in_path(path);
}

template <typename CharT, typename Traits = std::char_traits<CharT>>
inline std::basic_string<CharT, Traits> prepare_path(const CharT* path) {
  return prepare_path(std::basic_string<CharT, Traits>(path));
}

// is_file_name
template <typename CharT, typename Traits = std::char_traits<CharT>>
bool is_file_name(const std::basic_string<CharT, Traits>& filename) {
  if (filename.empty()) {
    return false;
  }

  for (size_t i = 0; i < filename.size(); ++i) {
    CharT c = filename[i];
    if (c == get_win_separator<CharT>() || c == get_separator<CharT>()) {
      return false;
    }
  }

  return true;
}

template <typename CharT, typename Traits = std::char_traits<CharT>>
bool is_file_name(const CharT* path) {
  return is_file_name(std::basic_string<CharT, Traits>(path));
}

template <typename CharT, typename Traits = std::char_traits<CharT>>
inline std::basic_string<CharT, Traits> get_file_or_dir_name(std::basic_string<CharT, Traits> path) {
  if (!is_valid_path(path)) {
    return std::basic_string<CharT, Traits>();
  }

  size_t lenght = path.length();
  if (path[lenght - 1] == get_separator<CharT>()) {  // folder
    size_t pos = path.find_last_of(get_separator<CharT>(), lenght - 2);
    if (pos != std::basic_string<CharT, Traits>::npos) {
      std::string res = path.substr(pos + 1, lenght - pos - 2);
      return res;
    }
    return get_separator_string<CharT>();
  }

  size_t pos = path.find_last_of(get_separator<CharT>());
  if (pos != std::basic_string<CharT, Traits>::npos) {
    return path.substr(pos + 1);
  }
  return std::basic_string<CharT, Traits>();
}

template <typename CharT, typename Traits = std::char_traits<CharT>>
inline std::basic_string<CharT, Traits> get_file_extension(std::basic_string<CharT, Traits> path) {  // extenstion
  if (!is_valid_path(path)) {
    return std::basic_string<CharT, Traits>();
  }

  size_t pos = path.find_last_of('.');
  if (pos != std::basic_string<CharT, Traits>::npos) {
    return path.substr(pos + 1);
  }

  return std::basic_string<CharT, Traits>();
}

template <typename CharT, typename Traits = std::char_traits<CharT>>
inline std::basic_string<CharT, Traits> stable_dir_path_separator(std::basic_string<CharT, Traits> path) {
  size_t lenght = path.length();
  if (lenght > 1 && path[lenght - 1] != file_system::get_separator<CharT>()) {
    path += get_separator<CharT>();
  }
  return path;
}

template <typename CharT, typename Traits = std::char_traits<CharT>>
inline std::basic_string<CharT, Traits> stable_dir_path(std::basic_string<CharT, Traits> path) {
  if (!is_valid_path(path)) {
    return std::basic_string<CharT, Traits>();
  }

  path = prepare_path(path);
  return stable_dir_path_separator(path);
}

template <typename CharT>
inline std::basic_string<CharT> get_dir_path(const std::basic_string<CharT>& path) {
  if (!is_valid_path(path)) {
    return std::basic_string<CharT>();
  }

  if (path == get_separator_string<CharT>()) {
    return path;
  }

  std::basic_string<CharT> ppath = prepare_path(path);
  size_t pos = ppath.find_last_of(get_separator<CharT>());
  if (pos != std::basic_string<CharT>::npos) {
    ppath = stable_dir_path(ppath.substr(0, pos + 1));
  }
  return ppath;
}

template <typename CharT>
inline std::basic_string<CharT> get_dir_path(const CharT* path) {
  return get_dir_path(std::basic_string<CharT>(path));
}

template <typename CharT>
inline std::basic_string<CharT> get_parent_dir_path(const std::basic_string<CharT>& path) {
  if (!is_valid_path(path)) {
    return std::basic_string<CharT>();
  }

  if (path == get_separator_string<CharT>()) {
    return path;
  }

  std::basic_string<CharT> ppath = prepare_path(path);
  size_t lenght = ppath.length();
  size_t pos = ppath.find_last_of(get_separator<CharT>());
  if (pos != std::basic_string<CharT>::npos) {
    if (pos == lenght - 1) {
      ppath.pop_back();
      pos = ppath.find_last_of(get_separator<CharT>());
    }
    if (pos != std::basic_string<CharT>::npos) {
      ppath = stable_dir_path(ppath.substr(0, pos + 1));
    }
  }
  return ppath;
}

template <typename CharT>
inline std::basic_string<CharT> get_parent_dir_path(const CharT* path) {
  return get_parent_dir_path(std::basic_string<CharT>(path));
}

template <typename CharT, typename Traits = std::char_traits<CharT>>
inline std::basic_string<CharT, Traits> make_path(const std::basic_string<CharT, Traits>& absolute_path,
                                                  const std::basic_string<CharT, Traits>& relative_path) {
  if (!is_valid_path(absolute_path)) {
    return std::basic_string<CharT, Traits>();
  }

  const std::basic_string<CharT, Traits> stabled_dir_path = stable_dir_path(absolute_path);
  return stabled_dir_path + relative_path;
}

template <typename CharT, typename Traits = std::char_traits<CharT>>
inline std::basic_string<CharT, Traits> make_path(const CharT* absolute_path, const CharT* relative_path) {
  return make_path(std::basic_string<CharT, Traits>(absolute_path), std::basic_string<CharT, Traits>(relative_path));
}

template <typename CharT, typename Traits = std::char_traits<CharT>>
inline std::basic_string<CharT, Traits> make_path(const std::basic_string<CharT>& absolute_path,
                                                  const CharT* relative_path) {
  return make_path(absolute_path, std::basic_string<CharT, Traits>(relative_path));
}

template <typename CharT, typename Traits = std::char_traits<CharT>>
inline std::basic_string<CharT, Traits> make_path(const CharT* absolute_path,
                                                  const std::basic_string<CharT>& relative_path) {
  return make_path(std::basic_string<CharT, Traits>(absolute_path), relative_path);
}

}  // namespace file_system
}  // namespace common
