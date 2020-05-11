/*  Copyright (C) 2014-2020 FastoGT. All right reserved.

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
#include <vector>

#include <common/file_system/types.h>

#include <common/optional.h>
#include <common/string16.h>

namespace common {
namespace file_system {

template <typename CharT, typename Traits = std::char_traits<CharT>>
class StringPath {
 public:
  typedef std::basic_string<CharT, Traits> value_type;
  typedef CharT char_type;

  StringPath() : path_() {}

  explicit StringPath(const value_type& path) : path_(prepare_path(path)) {}

  bool IsValid() const { return is_valid_path(path_); }

  value_type GetDirectory() const { return get_dir_path(path_); }

  value_type GetParentDirectory() const { return get_parent_dir_path(path_); }

  value_type GetPath() const { return path_; }

  bool Equals(const StringPath<CharT, Traits>& path) const { return path_ == path.path_; }

  void Append(const value_type& path) {
    if (path.empty()) {
      return;
    }

    path_ = make_path(path_, path);
    DCHECK(is_valid_path(path_));
  }

  value_type GetName() const { return get_file_or_dir_name(path_); }

 private:
  value_type path_;
};

template <typename CharT, typename Traits>
inline bool operator==(const StringPath<CharT, Traits>& left, const StringPath<CharT, Traits>& right) {
  return left.Equals(right);
}

template <typename CharT, typename Traits>
inline bool operator!=(const StringPath<CharT, Traits>& left, const StringPath<CharT, Traits>& right) {
  return !(left == right);
}

template <typename CharT, typename Traits>
inline bool operator<(const StringPath<CharT, Traits>& left, const StringPath<CharT, Traits>& right) {
  return left.GetPath() < right.GetPath();
}

template <typename CharT, typename Traits = std::char_traits<CharT>>
class FileStringPath : public StringPath<CharT, Traits> {
 public:
  typedef StringPath<CharT, Traits> base_class;
  typedef typename base_class::value_type value_type;
  typedef typename base_class::char_type char_type;

  FileStringPath() : base_class() {}

  explicit FileStringPath(const value_type& path) : base_class(path) {}

  value_type GetFileName() const { return base_class::GetName(); }

  value_type GetBaseFileName() const {  // without extension
    const value_type file_name = GetFileName();
    size_t pos = file_name.find_first_of('.');
    if (pos != std::basic_string<CharT, Traits>::npos) {
      return file_name.substr(0, pos);
    }

    return file_name;
  }

  value_type GetExtension() const {
    const value_type path = base_class::GetPath();
    return get_file_extension(path);
  }
};

template <typename CharT, typename Traits = std::char_traits<CharT>>
class DirectoryStringPath : public StringPath<CharT, Traits> {
 public:
  typedef StringPath<CharT, Traits> base_class;
  typedef typename base_class::value_type value_type;
  typedef typename base_class::char_type char_type;

  DirectoryStringPath() : base_class() {}

  explicit DirectoryStringPath(const value_type& path) : base_class(stable_dir_path(path)) {}

  value_type GetFolderName() const { return base_class::GetName(); }

  Optional<FileStringPath<CharT, Traits>> MakeFileStringPath(const value_type& filename) const WARN_UNUSED_RESULT {
    if (!base_class::IsValid()) {
      return Optional<FileStringPath<CharT, Traits>>();
    }

    if (!is_file_name(filename)) {
      return Optional<FileStringPath<CharT, Traits>>();
    }

    const value_type path = base_class::GetPath();  // stabled
    return FileStringPath<CharT, Traits>(path + filename);
  }

  Optional<FileStringPath<CharT, Traits>> MakeConcatFileStringPath(const value_type& last_part) const
      WARN_UNUSED_RESULT {
    if (!base_class::IsValid()) {
      return Optional<FileStringPath<CharT, Traits>>();
    }

    if (!is_relative_path(last_part)) {
      return Optional<FileStringPath<CharT, Traits>>();
    }

    const value_type path = base_class::GetPath();  // stabled
    return FileStringPath<CharT, Traits>(path + last_part);
  }

  Optional<DirectoryStringPath<CharT, Traits>> MakeDirectoryStringPath(const value_type& directory) const
      WARN_UNUSED_RESULT {
    if (!base_class::IsValid()) {
      return Optional<DirectoryStringPath<CharT, Traits>>();
    }

    if (!is_relative_path(directory)) {
      return Optional<DirectoryStringPath<CharT, Traits>>();
    }

    const value_type path = base_class::GetPath();  // stabled
    return DirectoryStringPath<CharT, Traits>(path + directory);
  }

  static DirectoryStringPath MakeHomeDir() {
    const DirectoryStringPath home(get_home_separator_string<CharT>());
    return home;
  }
};

typedef StringPath<char> ascii_string_path;
typedef StringPath<char16, string16_char_traits> utf_string_path;

typedef FileStringPath<char> ascii_file_string_path;
typedef FileStringPath<char16, string16_char_traits> utf_file_string_path;

typedef DirectoryStringPath<char> ascii_directory_string_path;
typedef DirectoryStringPath<char16, string16_char_traits> utf_directory_string_path;

}  // namespace file_system

std::string ConvertToString(const file_system::ascii_string_path& path);
string16 ConvertToString16(const file_system::utf_string_path& path);

bool ConvertFromString(const std::string& from, file_system::ascii_string_path* out) WARN_UNUSED_RESULT;
bool ConvertFromString16(const string16& from, file_system::utf_string_path* out) WARN_UNUSED_RESULT;
}  // namespace common
