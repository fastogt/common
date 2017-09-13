/*  Copyright (C) 2014-2017 FastoGT. All right reserved.

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

#include <common/error.h>     // for ErrnoError, Error
#include <common/string16.h>  // for string16, char16
#include <common/types.h>

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
std::basic_string<CharT, Traits> getenv(const char* env);

// Find the real name of path, by removing all ".", ".."
template <typename CharT, typename Traits = std::char_traits<CharT>>
std::basic_string<CharT, Traits> absolute_path_from_relative(const std::basic_string<CharT, Traits>& relative_path);
template <typename CharT, typename Traits = std::char_traits<CharT>>
inline std::basic_string<CharT, Traits> absolute_path_from_relative(const CharT* path) {
  return absolute_path_from_relative(std::basic_string<CharT, Traits>(path));
}

// pwd + filename
template <typename CharT, typename Traits = std::char_traits<CharT>>
std::basic_string<CharT, Traits> absolute_path_from_filename(const std::basic_string<CharT, Traits>& filename);
template <typename CharT, typename Traits = std::char_traits<CharT>>
inline std::basic_string<CharT, Traits> absolute_path_from_filename(const CharT* path) {
  return absolute_path_from_filename(std::basic_string<CharT, Traits>(path));
}

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
inline std::basic_string<CharT, Traits> prepare_path(
    std::basic_string<CharT, Traits> path) {  // should be absolute path
  if (!is_valid_path(path)) {
    DNOTREACHED();
    return std::basic_string<CharT, Traits>();
  }

  if (path[0] == get_home_separator<CharT>()) {
#ifdef OS_POSIX
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
inline std::basic_string<CharT, Traits> get_file_name(std::basic_string<CharT, Traits> path) {  // filename + extension
  if (!is_valid_path(path)) {
    return std::basic_string<CharT, Traits>();
  }

  size_t lenght = path.length();
  if (path[lenght - 1] == get_separator<CharT>()) {
    size_t pos = path.find_last_of(get_separator<CharT>(), lenght - 2);
    if (pos != std::basic_string<CharT, Traits>::npos) {
      std::string res = path.substr(pos + 1, lenght - pos - 2);
      return res;
    }
    return get_separator_string<CharT>();
  } else {
    size_t pos = path.find_last_of(get_separator<CharT>());
    if (pos != std::basic_string<CharT, Traits>::npos) {
      return path.substr(pos + 1);
    }
    return std::basic_string<CharT, Traits>();
  }
}

template <typename CharT, typename Traits = std::char_traits<CharT>>
inline std::basic_string<CharT, Traits> get_file_extension(std::basic_string<CharT, Traits> path) {  // extenstion
  if (!is_valid_path(path)) {
    return std::basic_string<CharT, Traits>();
  }

  size_t pos = path.find_first_of('.');
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

off_t get_file_size_by_descriptor(descriptor_t fd_desc);
ErrnoError clear_file_by_descriptor(descriptor_t fd_desc) WARN_UNUSED_RESULT;
ErrnoError close_descriptor(descriptor_t fd_desc) WARN_UNUSED_RESULT;

ErrnoError open_descriptor(const std::string& path, int oflags, descriptor_t* out_desc) WARN_UNUSED_RESULT;
ErrnoError create_node(const std::string& path) WARN_UNUSED_RESULT;
ErrnoError touch(const std::string& path) WARN_UNUSED_RESULT;
typedef ErrnoError (*read_cb)(const char* buff, uint32_t buff_len, void* user_data, uint32_t* processed);
ErrnoError read_file_cb(int in_fd, off_t* offset, size_t count, read_cb cb, void* user_data) WARN_UNUSED_RESULT;

ErrnoError copy_file(const std::string& path_from, const std::string& path_to) WARN_UNUSED_RESULT;
ErrnoError move_file(const std::string& path_from, const std::string& path_to) WARN_UNUSED_RESULT;
ErrnoError remove_file(const std::string& file_path) WARN_UNUSED_RESULT;
ErrnoError node_access(const std::string& node) WARN_UNUSED_RESULT;
ErrnoError create_directory(const std::string& path, bool is_recursive) WARN_UNUSED_RESULT;
ErrnoError remove_directory(const std::string& path, bool is_recursive) WARN_UNUSED_RESULT;
ErrnoError change_directory(const std::string& path) WARN_UNUSED_RESULT;

#ifdef OS_MACOSX
std::string bundle_pwd();
#endif
std::string pwd();

ErrnoError write_to_descriptor(descriptor_t fd_desc, const void* buf, size_t len, size_t* nwrite_out)
    WARN_UNUSED_RESULT;
ErrnoError read_from_descriptor(descriptor_t fd_desc, void* buf, size_t len, size_t* readlen) WARN_UNUSED_RESULT;

ErrnoError get_file_time_last_modification(const std::string& file_path,
                                           time64_t* mod_time_sec) WARN_UNUSED_RESULT;  // utc time millisecond

bool find_file_in_path(const std::string& file_name, std::string* out_path) WARN_UNUSED_RESULT;

//  ==============================Path=====================================  //

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

template <typename CharT, typename Traits = std::char_traits<CharT>>
class FileStringPath : public StringPath<CharT, Traits> {
 public:
  typedef StringPath<CharT, Traits> base_class;
  typedef typename base_class::value_type value_type;
  typedef typename base_class::char_type char_type;

  FileStringPath() : base_class() {}

  explicit FileStringPath(const value_type& path) : base_class(path) {}

  value_type GetFileName() const {
    const value_type path = base_class::GetPath();
    return get_file_name(path);
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
};

typedef StringPath<char> ascii_string_path;
typedef StringPath<char16, string16_char_traits> utf_string_path;

typedef FileStringPath<char> ascii_file_string_path;
typedef FileStringPath<char16, string16_char_traits> utf_file_string_path;

typedef DirectoryStringPath<char> ascii_directory_string_path;
typedef DirectoryStringPath<char16, string16_char_traits> utf_directory_string_path;

//  ==============================File=====================================  //

class DescriptorHolder {
 public:
  explicit DescriptorHolder(descriptor_t fd);
  ~DescriptorHolder();

  bool IsValid() const;
  descriptor_t GetFd() const;

  ErrnoError Write(const buffer_t& data, size_t* nwrite_out) WARN_UNUSED_RESULT;
  ErrnoError Write(const std::string& data, size_t* nwrite_out) WARN_UNUSED_RESULT;
  ErrnoError Write(const void* data, size_t size, size_t* nwrite_out) WARN_UNUSED_RESULT;

  ErrnoError Read(buffer_t* out_data, size_t max_size, size_t* nread_out) WARN_UNUSED_RESULT;
  ErrnoError Read(std::string* out_data, size_t max_size, size_t* nread_out) WARN_UNUSED_RESULT;
  ErrnoError Read(void* out, size_t len, size_t* nread_out) WARN_UNUSED_RESULT;
  ErrnoError Close() WARN_UNUSED_RESULT;

  ErrnoError Lock() WARN_UNUSED_RESULT;
  ErrnoError Unlock() WARN_UNUSED_RESULT;

 private:
  DISALLOW_COPY_AND_ASSIGN(DescriptorHolder);
  descriptor_t fd_;
};

class File {
 public:
  typedef ascii_string_path path_type;
  enum Flags {
    FLAG_OPEN = 1 << 0,            // Opens a file, only if it exists.
    FLAG_CREATE = 1 << 1,          // Creates a new file, only if it does not
    FLAG_OPEN_TRUNCATED = 1 << 2,  // Opens a file and truncates it, only if it
                                   // exists.
    FLAG_OPEN_BINARY = 1 << 3,
    FLAG_READ = 1 << 4,
    FLAG_WRITE = 1 << 5,
    FLAG_APPEND = 1 << 6,
    FLAG_DELETE_ON_CLOSE = 1 << 7
  };

  File();
  ~File();

  ErrnoError Open(const path_type::value_type& file_path, uint32_t flags) WARN_UNUSED_RESULT;
  ErrnoError Open(const path_type& file_path, uint32_t flags) WARN_UNUSED_RESULT;
  path_type GetPath() const;
  bool IsValid() const;

  ErrnoError Write(const buffer_t& data, size_t* nwrite_out) WARN_UNUSED_RESULT;
  ErrnoError Write(const std::string& data, size_t* nwrite_out) WARN_UNUSED_RESULT;
  ErrnoError Write(const void* data, size_t size, size_t* nwrite_out) WARN_UNUSED_RESULT;

  ErrnoError Read(buffer_t* out_data, size_t max_size, size_t* nread_out) WARN_UNUSED_RESULT;
  ErrnoError Read(std::string* out_data, size_t max_size, size_t* nread_out) WARN_UNUSED_RESULT;
  ErrnoError Read(void* out, size_t len, size_t* nread_out) WARN_UNUSED_RESULT;
  ErrnoError Close() WARN_UNUSED_RESULT;

  ErrnoError Lock() WARN_UNUSED_RESULT;
  ErrnoError Unlock() WARN_UNUSED_RESULT;

 private:
  DISALLOW_COPY_AND_ASSIGN(File);
  DescriptorHolder* holder_;
  path_type file_path_;
};

class ANSIFile {
 public:
  typedef ascii_string_path path_type;

  explicit ANSIFile(const path_type& file_path);
  ~ANSIFile();

  ErrnoError Open(const char* mode) WARN_UNUSED_RESULT;
  bool IsOpened() const;

  ErrnoError Lock() WARN_UNUSED_RESULT;
  ErrnoError Unlock() WARN_UNUSED_RESULT;

  bool Read(buffer_t* out_data, uint32_t max_size);
  bool Read(std::string* out_data, uint32_t max_size);
  bool ReadLine(buffer_t* out_data);
  bool ReadLine(std::string* out_data);

  bool Write(const buffer_t& data);
  bool Write(const std::string& data);
  bool Write(const string16& data);
  template <typename... Args>
  bool WriteFormated(const char* format, Args... args) {
    if (!file_) {
      return false;
    }

    std::string data = MemSPrintf(format, args...);
    return Write(data);
  }

  bool Truncate(off_t pos);

  void Flush();
  void Close();

  bool IsEOF() const;

 private:
  DISALLOW_COPY_AND_ASSIGN(ANSIFile);
  path_type path_;
  FILE* file_;
};

}  // namespace file_system
std::string ConvertToString(const file_system::ascii_string_path& path);
string16 ConvertToString16(const file_system::utf_string_path& path);

bool ConvertFromString(const std::string& from, file_system::ascii_string_path* out) WARN_UNUSED_RESULT;
bool ConvertFromString16(const string16& from, file_system::utf_string_path* out) WARN_UNUSED_RESULT;
}  // namespace common
