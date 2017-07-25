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

#include <common/file_system.h>

#include <dirent.h>    // for closedir, readdir, DIR, DT_REG, dirent
#include <errno.h>     // for errno, EINVAL, EEXIST, etc
#include <fcntl.h>     // for open, SEEK_CUR, SEEK_SET, etc
#include <limits.h>    // for PATH_MAX
#include <stdlib.h>    // for free, getenv, malloc
#include <string.h>    // for NULL, strlen
#include <sys/stat.h>  // for stat, S_IRWXG, S_IRWXO, etc
#include <unistd.h>    // for close, lseek, read, ssize_t, etc

#if defined(OS_WIN)
#include <fileapi.h>
#endif

#include <sstream>  // for stringstream
#include <string>   // for string, char_traits, etc

#if defined(COMPILER_MSVC)
#include <direct.h>
#include <io.h>
#include <windows.h>
#define mkdir _mkdir
int ftruncate(descriptor_t fd, int64_t length) {
  HANDLE fh = reinterpret_cast<HANDLE>(_get_osfhandle(fd));
  if (!fh || _lseeki64(fd, length, SEEK_SET))
    return -1;
  return SetEndOfFile(fh) ? 0 : -1;
}
#if !defined(S_ISDIR)
#define S_ISDIR(mode) ((mode & S_IFMT) == S_IFDIR)
#endif
#endif

#include <common/convert2string.h>  // for ConvertToString16
#include <common/error.h>           // for ErrnoError, Error
#include <common/macros.h>          // for ERROR_RESULT_VALUE, etc
#include <common/string16.h>        // for string16
#include <common/types.h>           // for buffer_t, byte_t, tribool, triboo...

#define FS_BUF_SIZE 1024 * 4

namespace {
common::ErrnoError rmdir_directory_impl(const char* path) {
  bool result = rmdir(path) != ERROR_RESULT_VALUE;
  if (!result && errno != ENOENT) {
    return common::make_error_value_perror("rmdir", errno, common::ErrorValue::E_ERROR);
  }

  return common::ErrnoError();
}

int cp(const char* from, const char* to) {
  int fd_to, fd_from;
  char buf[4096];
  ssize_t nread;
  int saved_errno;

  fd_from = open(from, O_RDONLY);
  if (fd_from < 0) {
    return ERROR_RESULT_VALUE;
  }

  fd_to = open(to, O_WRONLY | O_CREAT | O_EXCL, 0666);
  if (fd_to < 0) {
    goto out_error;
  }

  while (nread = read(fd_from, buf, sizeof buf), nread > 0) {
    char* out_ptr = buf;
    ssize_t nwritten;
    do {
      nwritten = write(fd_to, out_ptr, nread);

      if (nwritten >= 0) {
        nread -= nwritten;
        out_ptr += nwritten;
      } else if (errno != EINTR) {
        goto out_error;
      }
    } while (nread > 0);
  }

  if (nread == 0) {
    if (close(fd_to) < 0) {
      fd_to = INVALID_DESCRIPTOR;
      goto out_error;
    }
    close(fd_from);

    /* Success! */
    return 0;
  }

out_error:
  saved_errno = errno;

  close(fd_from);
  if (fd_to >= 0) {
    close(fd_to);
  }

  errno = saved_errno;
  return ERROR_RESULT_VALUE;
}

}  // namespace

namespace common {
namespace file_system {

namespace {
ErrnoError call_fcntl_flock(descriptor_t fd_desc, bool do_lock) {
  if (fd_desc == INVALID_DESCRIPTOR) {
    return make_error_value_perror("call_fcntl_flock", EINVAL, ErrorValue::E_ERROR);
  }
#ifdef OS_POSIX
  struct flock lock;
  lock.l_type = do_lock ? F_WRLCK : F_UNLCK;
  lock.l_whence = SEEK_SET;
  lock.l_start = 0;
  lock.l_len = 0;  // Lock entire file.
  int res = fcntl(fd_desc, F_SETLK, &lock);
  if (res == -1) {
    return make_error_value_perror("fcntl", errno, common::ErrorValue::E_ERROR);
  }
#else
  HANDLE fh = reinterpret_cast<HANDLE>(_get_osfhandle(fd_desc));
  if (!fh) {
    return make_error_value_perror("_get_osfhandle", EINVAL, ErrorValue::E_ERROR);
  }
  BOOL result = do_lock ? LockFile(fh, 0, 0, MAXDWORD, MAXDWORD) : UnlockFile(fh, 0, 0, MAXDWORD, MAXDWORD);
  if (!result) {
    return make_error_value_perror(do_lock ? "LockFile" : "UnlockFile", errno, ErrorValue::E_ERROR);
  }
#endif
  return ErrnoError();
}

template <typename CharT>
bool realpath_without_exist(const std::basic_string<CharT>& relative_path, std::basic_string<CharT>* realpath) {
  typedef std::basic_string<CharT> string_t;
  string_t path = fixup_separator_in_path(relative_path);
  if (path.empty() || !realpath) {
    return false;
  }

  string_t cpwd = pwd();
  std::vector<string_t> parts;
  std::vector<string_t> valid_parts;
  size_t count_del = Tokenize(cpwd + path, get_separator_string<CharT>(), &parts);
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

#ifdef OS_WIN
  *realpath = JoinString(valid_parts, get_separator_string<CharT>());
#else
  *realpath = get_separator_string<CharT>() + JoinString(valid_parts, get_separator_string<CharT>());
#endif
  return true;
}

common::ErrnoError create_directory_impl(const char* path) {
#ifdef OS_WIN
  bool result = mkdir(path) != ERROR_RESULT_VALUE;
#else
  bool result = mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO) != ERROR_RESULT_VALUE;
#endif
  if (!result && errno != EEXIST) {
    return common::make_error_value_perror("mkdir", errno, common::ErrorValue::E_ERROR);
  }

  return common::ErrnoError();
}
}  // namespace

template <>
std::string getenv(const char* env) {
  return ::getenv(env);
}

template <>
string16 getenv(const char* env) {
  char* res = ::getenv(env);
  return ConvertToString16(res);
}

template <>
std::string absolute_path_from_relative(const std::string& path) {
  std::string real_path;
  if (!realpath_without_exist(path, &real_path)) {
    return std::string();
  }

  return prepare_path(real_path);
}

template <>
string16 absolute_path_from_relative(const string16& path) {
  std::string rp = absolute_path_from_relative(ConvertToString(path));
  return ConvertToString16(rp);
}

template <>
std::string absolute_path_from_filename(const std::string& filename) {
  std::string p = pwd();
  return p + filename;
}

template <>
string16 absolute_path_from_filename(const string16& filename) {
  std::string rp = absolute_path_from_filename(ConvertToString(filename));
  return ConvertToString16(rp);
}

template <>
tribool is_directory(const std::string& path) {
  if (!is_valid_path(path)) {
    return INDETERMINATE;
  }

  struct stat filestat;
  std::string p_path = prepare_path(path);
  if (::stat(p_path.c_str(), &filestat) != ERROR_RESULT_VALUE) {
    return S_ISDIR(filestat.st_mode) ? SUCCESS : FAIL;
  }
  return INDETERMINATE;
}

template <>
tribool is_directory(const string16& path) {
  return is_directory(ConvertToString(path));
}

ErrnoError clear_file_by_descriptor(descriptor_t fd_desc) {
  if (fd_desc == INVALID_DESCRIPTOR) {
    return make_error_value_perror("clear_file_by_descriptor", EINVAL, ErrorValue::E_ERROR);
  }

  bool result = ftruncate(fd_desc, 0) != ERROR_RESULT_VALUE;
  if (!result) {
    return make_error_value_perror("ftruncate", errno, ErrorValue::E_ERROR);
  }

  return ErrnoError();
}

ErrnoError create_node(const std::string& path) {
  if (path.empty()) {
    return make_error_value_perror("create_node", EINVAL, ErrorValue::E_ERROR);
  }

  File fl;
  return fl.Open(path, File::FLAG_OPEN | File::FLAG_CREATE | File::FLAG_WRITE);
}

ErrnoError touch(const std::string& path) {
  return create_node(path);
}

Error read_file_cb(int in_fd, off_t* offset, size_t count, read_cb cb, void* user_data) {
  if (!cb || in_fd == INVALID_DESCRIPTOR) {
    return make_error_value_perror("read_file_cb", EINVAL, ErrorValue::E_ERROR);
  }

  off_t orig = 0;
  char buf[FS_BUF_SIZE] = {0};

  if (offset) {
    /* Save current file offset and set offset to value in '*offset' */

    orig = lseek(in_fd, 0, SEEK_CUR);
    if (orig == -1) {
      return make_error_value_perror("lseek", errno, ErrorValue::E_ERROR);
    }
    if (lseek(in_fd, *offset, SEEK_SET) == -1) {
      return make_error_value_perror("lseek", errno, ErrorValue::E_ERROR);
    }
  }

  while (count > 0) {
    ssize_t numRead = read(in_fd, buf, FS_BUF_SIZE);
    if (numRead == -1) {
      return make_error_value_perror("read", errno, ErrorValue::E_ERROR);
    }
    if (numRead == 0) {
      break; /* EOF */
    }

    uint32_t numSent = 0;
    Error err = cb(buf, numRead, user_data, &numSent);
    if (err && err->IsError()) {
      return err;
    }

    count -= numSent;
  }

  if (offset) {
    /* Return updated file offset in '*offset', and reset the file offset
       to the value it had when we were called. */

    *offset = lseek(in_fd, 0, SEEK_CUR);
    if (*offset == -1) {
      return make_error_value_perror("lseek", errno, ErrorValue::E_ERROR);
    }
    if (lseek(in_fd, orig, SEEK_SET) == -1) {
      return make_error_value_perror("lseek", errno, ErrorValue::E_ERROR);
    }
  }

  return ErrnoError();
}

ErrnoError copy_file(const std::string& pathFrom, const std::string& pathTo) {
  if (pathFrom.empty()) {
    return make_error_value_perror("copy_file", EINVAL, ErrorValue::E_ERROR);
  }

  if (pathTo.empty()) {
    return make_error_value_perror("copy_file", EINVAL, ErrorValue::E_ERROR);
  }

  std::string pr_from = prepare_path(pathFrom);
  std::string pr_to = prepare_path(pathTo);

  int cpRes = cp(pr_from.c_str(), pr_to.c_str());
  bool result = cpRes != ERROR_RESULT_VALUE;
  if (!result) {
    return make_error_value_perror("cp", errno, ErrorValue::E_ERROR);
  }

  return ErrnoError();
}

ErrnoError move_file(const std::string& pathFrom, const std::string& pathTo) {
  if (pathFrom.empty()) {
    return make_error_value_perror("move_file", EINVAL, ErrorValue::E_ERROR);
  }

  if (pathTo.empty()) {
    return make_error_value_perror("move_file", EINVAL, ErrorValue::E_ERROR);
  }

  std::string pr_from = prepare_path(pathFrom);
  std::string pr_to = prepare_path(pathTo);

  int res = rename(pr_from.c_str(), pr_to.c_str());
  bool result = res != ERROR_RESULT_VALUE;
  if (!result) {
    return make_error_value_perror("rename", errno, ErrorValue::E_ERROR);
  }

  return ErrnoError();
}

ErrnoError remove_file(const std::string& file_path) {
  if (file_path.empty()) {
    return make_error_value_perror("remove_file", EINVAL, ErrorValue::E_ERROR);
  }

  std::string pr_to = prepare_path(file_path);

  int res = remove(pr_to.c_str());
  bool result = res != ERROR_RESULT_VALUE;
  if (!result && errno != ENOENT) {
    return make_error_value_perror("remove", errno, ErrorValue::E_ERROR);
  }

  return ErrnoError();
}

ErrnoError node_access(const std::string& node) {
  if (node.empty()) {
    return make_error_value_perror("node_access", EINVAL, ErrorValue::E_ERROR);
  }

  std::string pr_node = prepare_path(node);
  if (pr_node.empty()) {
    return make_error_value_perror("node_access", EINVAL, ErrorValue::E_ERROR);
  }

  int res = access(pr_node.c_str(), W_OK);
  if (res == ERROR_RESULT_VALUE) {
    return make_error_value_perror("access", errno, ErrorValue::E_ERROR);
  }

  return ErrnoError();
}

ErrnoError create_directory(const std::string& path, bool is_recursive) {
  if (path.empty()) {
    return make_error_value_perror("create_directory", EINVAL, ErrorValue::E_ERROR);
  }

  std::string pr_path = prepare_path(path);
  if (pr_path.empty()) {
    return make_error_value_perror("create_directory", EINVAL, ErrorValue::E_ERROR);
  }

  pr_path = stable_dir_path(pr_path);
  if (pr_path.empty()) {
    return make_error_value_perror("create_directory", EINVAL, ErrorValue::E_ERROR);
  }

  const char* pr_path_ptr = pr_path.c_str();
  if (is_recursive) {
    char* p = NULL;
#ifdef OS_WIN
    uint8_t shift = 3;
#else
    uint8_t shift = 1;
#endif
    for (p = const_cast<char*>(pr_path_ptr + shift); *p; p++) {
      if (*p == get_separator<char>()) {
        *p = 0;
        const char* path = pr_path_ptr;

        bool needCreate = false;
        struct stat filestat;
        if (::stat(path, &filestat) == ERROR_RESULT_VALUE) {
          needCreate = true;
        } else {
          if (!S_ISDIR(filestat.st_mode)) {
            needCreate = true;
          }
        }

        if (needCreate) {
          create_directory_impl(path);
        }

        *p = get_separator<char>();
      }
    }
  }

  return create_directory_impl(pr_path_ptr);
}

ErrnoError remove_directory(const std::string& path, bool is_recursive) {
  if (path.empty()) {
    return make_error_value_perror("remove_directory", EINVAL, ErrorValue::E_ERROR);
  }

  std::string prPath = prepare_path(path);
  if (prPath[prPath.length() - 1] == get_separator<char>()) {
    prPath[prPath.length() - 1] = 0;
  }

  const char* prPathPtr = prPath.c_str();
  if (is_recursive) {
    DIR* dirp = opendir(prPathPtr);
    struct dirent* p;
    while ((p = readdir(dirp)) != NULL) {
      /* Skip the names "." and ".." as we don't want to recurse on them. */
      if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, "..")) {
        continue;
      }

      char pathBuffer[PATH_MAX] = {0};
      common::SNPrintf(pathBuffer, sizeof(pathBuffer), "%s/%s", path, p->d_name);
      struct stat statbuf;
      if (!stat(pathBuffer, &statbuf)) {
        if (S_ISDIR(statbuf.st_mode)) {
          ErrnoError err = remove_directory(pathBuffer, is_recursive);
          if (err && err->IsError()) {
            closedir(dirp);
            return err;
          }
        } else {
          ErrnoError err = remove_file(pathBuffer);
          if (err && err->IsError()) {
            closedir(dirp);
            return err;
          }
        }
      }
    }
    closedir(dirp);
  }

  return rmdir_directory_impl(prPathPtr);
}

ErrnoError change_directory(const std::string& path) {
  if (path.empty()) {
    return make_error_value_perror("change_directory", EINVAL, ErrorValue::E_ERROR);
  }

  std::string spath = stable_dir_path(path);
  bool result = chdir(spath.c_str()) != ERROR_RESULT_VALUE;
  if (!result) {
    return make_error_value_perror("chdir", errno, ErrorValue::E_ERROR);
  }

  return ErrnoError();
}

std::string pwd() {
  char cwd[1024] = {0};
  char* ret = getcwd(cwd, sizeof(cwd));
  if (!ret) {
    return std::string();
  }

  return stable_dir_path(std::string(cwd));
}

ErrnoError open_descriptor(const std::string& path, int oflags, descriptor_t* out_desc) {
  if (path.empty() || !out_desc) {
    return make_error_value_perror("open_descriptor", EINVAL, ErrorValue::E_ERROR);
  }

  static const int mode = S_IRUSR | S_IWUSR;
  descriptor_t desc = HANDLE_EINTR(open(path.c_str(), oflags, mode));
  if (desc == INVALID_DESCRIPTOR) {
    return make_error_value_perror("open", errno, ErrorValue::E_ERROR);
  }

  *out_desc = desc;
  return ErrnoError();
}

ErrnoError close_descriptor(descriptor_t fd_desc) {
  if (fd_desc == INVALID_DESCRIPTOR) {
    return ErrnoError();
  }

  int res = close(fd_desc);
  if (res == ERROR_RESULT_VALUE) {
    return make_error_value_perror("close", errno, ErrorValue::E_ERROR);
  }

  return ErrnoError();
}

ErrnoError write_to_descriptor(descriptor_t fd_desc, const void* buf, size_t len, size_t* nwrite_out) {
  if (fd_desc == INVALID_DESCRIPTOR || !nwrite_out) {
    return make_error_value_perror("write_to_descriptor", EINVAL, ErrorValue::E_ERROR);
  }

  ssize_t res = write(fd_desc, buf, len);
  if (res == ERROR_RESULT_VALUE) {
    return make_error_value_perror("write", errno, ErrorValue::E_ERROR);
  }

  *nwrite_out = res;
  return ErrnoError();
}

ErrnoError read_from_descriptor(descriptor_t fd_desc, void* buf, size_t len, size_t* readlen) {
  if (fd_desc == INVALID_DESCRIPTOR || !readlen) {
    return make_error_value_perror("read_from_descriptor", EINVAL, ErrorValue::E_ERROR);
  }

  ssize_t res = read(fd_desc, buf, len);
  bool result = res != ERROR_RESULT_VALUE;
  if (!result) {
    return make_error_value_perror("read", errno, ErrorValue::E_ERROR);
  }

  *readlen = res;
  return ErrnoError();
}

off_t get_file_size_by_descriptor(descriptor_t fd_desc) {
  if (fd_desc == INVALID_DESCRIPTOR) {
    return 0;
  }

  struct stat stat_buf;
  fstat(fd_desc, &stat_buf);

  return stat_buf.st_size;
}

bool find_file_in_path(const std::string& file_name, std::string* out_path) {
  if (file_name.empty() || !out_path) {
    return false;
  }

  std::stringstream path(getenv<char>("PATH"));
  while (!path.eof()) {
    std::string test;
    struct stat info;
#ifdef OS_WIN
    getline(path, test, ';');
#else
    getline(path, test, ':');
#endif
    test = stable_dir_path(test);
    test.append(file_name);
    if (stat(test.c_str(), &info) == 0) {
      *out_path = test;
      return true;
    }
  }

  return false;
}

DescriptorHolder::DescriptorHolder(descriptor_t fd) : fd_(fd) {}

DescriptorHolder::~DescriptorHolder() {}

bool DescriptorHolder::IsValid() const {
  return fd_ != INVALID_DESCRIPTOR;
}

descriptor_t DescriptorHolder::GetFd() const {
  DCHECK(IsValid());
  return fd_;
}

ErrnoError DescriptorHolder::Write(const buffer_t& data, size_t* nwrite_out) {
  DCHECK(IsValid());
  return write_to_descriptor(fd_, data.data(), data.size(), nwrite_out);
}

ErrnoError DescriptorHolder::Write(const std::string& data, size_t* nwrite_out) {
  DCHECK(IsValid());
  return write_to_descriptor(fd_, data.data(), data.size(), nwrite_out);
}

ErrnoError DescriptorHolder::Write(const void* data, size_t size, size_t* nwrite_out) {
  DCHECK(IsValid());
  return write_to_descriptor(fd_, data, size, nwrite_out);
}

ErrnoError DescriptorHolder::Read(buffer_t* out_data, size_t max_size, size_t* nread_out) {
  DCHECK(IsValid());
  return read_from_descriptor(fd_, out_data->data(), max_size, nread_out);
}

ErrnoError DescriptorHolder::Read(std::string* out_data, size_t max_size, size_t* nread_out) {
  DCHECK(IsValid());
  char* buff = new char[max_size];
  ErrnoError err = read_from_descriptor(fd_, buff, max_size, nread_out);
  *out_data = std::string(buff, max_size);
  delete[] buff;
  return err;
}

ErrnoError DescriptorHolder::Read(void* out, size_t len, size_t* nread_out) {
  DCHECK(IsValid());
  return read_from_descriptor(fd_, out, len, nread_out);
}

ErrnoError DescriptorHolder::Close() {
  DCHECK(IsValid());
  return close_descriptor(fd_);
}

ErrnoError DescriptorHolder::Lock() {
  DCHECK(IsValid());
  return call_fcntl_flock(fd_, true);
}

ErrnoError DescriptorHolder::Unlock() {
  DCHECK(IsValid());
  return call_fcntl_flock(fd_, false);
}

File::File() : holder_(nullptr), file_path_() {}

File::~File() {
  delete holder_;
}

File::path_type File::Path() const {
  return file_path_;
}

bool File::IsValid() const {
  return holder_ && holder_->IsValid();
}

ErrnoError File::Write(const buffer_t& data, size_t* nwrite_out) {
  DCHECK(IsValid());
  return holder_->Write(data, nwrite_out);
}

ErrnoError File::Write(const std::string& data, size_t* nwrite_out) {
  DCHECK(IsValid());
  return holder_->Write(data, nwrite_out);
}

ErrnoError File::Write(const void* data, size_t size, size_t* nwrite_out) {
  DCHECK(IsValid());
  return holder_->Write(data, size, nwrite_out);
}

ErrnoError File::Read(buffer_t* out_data, size_t max_size, size_t* nread_out) {
  DCHECK(IsValid());
  return holder_->Read(out_data, max_size, nread_out);
}

ErrnoError File::Read(std::string* out_data, size_t max_size, size_t* nread_out) {
  DCHECK(IsValid());
  return holder_->Read(out_data, max_size, nread_out);
}

ErrnoError File::Read(void* out, size_t len, size_t* nread_out) {
  DCHECK(IsValid());
  return holder_->Read(out, len, nread_out);
}
ErrnoError File::Close() {
  DCHECK(IsValid());
  return holder_->Close();
}

ErrnoError File::Lock() {
  DCHECK(IsValid());
  return holder_->Lock();
}
ErrnoError File::Unlock() {
  DCHECK(IsValid());
  return holder_->Unlock();
}

ErrnoError File::Open(const path_type::value_type& file_path, uint32_t flags) {
  return Open(path_type(file_path), flags);
}

ErrnoError File::Open(const path_type& file_path, uint32_t flags) {
  DCHECK(!IsValid());

  int open_flags = 0;
  if (flags & FLAG_CREATE) {
    open_flags = O_CREAT | O_EXCL;
  }

  if (flags & FLAG_OPEN_TRUNCATED) {
    DCHECK(!open_flags);
    DCHECK(flags & FLAG_WRITE);
    open_flags = O_TRUNC;
  }

#ifdef OS_WIN  // binary can be only on windows
  if (flags & FLAG_OPEN_BINARY) {
    open_flags |= O_BINARY;
  }
#endif

  if (!open_flags && !(flags & FLAG_OPEN)) {
    NOTREACHED();
    errno = EOPNOTSUPP;
    return make_error_value_perror("write_to_descriptor", EOPNOTSUPP, ErrorValue::E_ERROR);
  }

  if (flags & FLAG_WRITE && flags & FLAG_READ) {
    open_flags |= O_RDWR;
  } else if (flags & FLAG_WRITE) {
    open_flags |= O_WRONLY;
  } else if (!(flags & FLAG_READ) && !(flags & FLAG_APPEND)) {
    NOTREACHED();
  }

  if (flags & FLAG_APPEND && flags & FLAG_READ) {
    open_flags |= O_APPEND | O_RDWR;
  } else if (flags & FLAG_APPEND) {
    open_flags |= O_APPEND | O_WRONLY;
  }

  int fd = INVALID_DESCRIPTOR;
  const std::string path_str = file_path.Path();
  ErrnoError err = open_descriptor(path_str, open_flags, &fd);
  if (err && err->IsError()) {
    return err;
  }

  if (flags & FLAG_DELETE_ON_CLOSE) {
    unlink(path_str.c_str());
  }

  holder_ = new DescriptorHolder(fd);
  file_path_ = file_path;
  return ErrnoError();
}

ANSIFile::ANSIFile(const path_type& file_path) : path_(file_path), file_(NULL) {}

ANSIFile::~ANSIFile() {}

ErrnoError ANSIFile::Open(const char* mode) {
  if (!file_) {
    std::string spath = path_.Path();
    const char* path = spath.c_str();
    file_ = fopen(path, mode);
    if (file_) {
      return ErrnoError();
    }
    return make_error_value_perror("Open", errno, ErrorValue::E_ERROR);
  }

  return ErrnoError();
}

ErrnoError ANSIFile::Lock() {
  if (!file_) {
    return make_error_value_perror("Unlock", EINVAL, ErrorValue::E_ERROR);
  }

  int fd = fileno(file_);
  return call_fcntl_flock(fd, true);
}

ErrnoError ANSIFile::Unlock() {
  if (!file_) {
    return make_error_value_perror("Unlock", EINVAL, ErrorValue::E_ERROR);
  }

  int fd = fileno(file_);
  return call_fcntl_flock(fd, false);
}

bool ANSIFile::Read(buffer_t* out_data, uint32_t max_size) {
  if (!file_ || !out_data) {
    return false;
  }

  byte_t* data = reinterpret_cast<byte_t*>(malloc(max_size));
  if (!data) {
    return false;
  }

  size_t res = fread(data, sizeof(byte_t), max_size, file_);
  if (res > 0) {
    *out_data = buffer_t(data, data + res);
  } else if (feof(file_)) {
  }
  free(data);

  return true;
}

bool ANSIFile::Read(std::string* out_data, uint32_t max_size) {
  if (!file_ || !out_data) {
    return false;
  }

  char* data = reinterpret_cast<char*>(malloc(max_size));
  if (!data) {
    return false;
  }

  size_t res = fread(data, sizeof(char), max_size, file_);
  if (res > 0) {
    *out_data = std::string(data, res);
  } else if (feof(file_)) {
  }
  free(data);

  return true;
}

bool ANSIFile::ReadLine(buffer_t* out_data) {
  if (!file_ || !out_data) {
    return false;
  }

  char buff[1024] = {0};
  char* res = fgets(buff, sizeof(buff), file_);
  if (res) {
    size_t buf_len = strlen(buff);
    *out_data = buffer_t(buff, buff + buf_len);
  }

  return true;
}

bool ANSIFile::ReadLine(std::string* out_data) {
  if (!file_ || !out_data) {
    return false;
  }

  char buff[1024] = {0};
  char* res = fgets(buff, sizeof(buff), file_);
  if (res) {
    size_t buf_len = strlen(buff);
    if (buff[buf_len - 1] == '\n') {
      buff[buf_len - 1] = 0;
      buf_len--;
    }
    *out_data = std::string(buff, buf_len);
  }

  return true;
}

bool ANSIFile::IsEOF() const {
  if (!file_) {
    return true;
  }

  return feof(file_) != 0;
}

bool ANSIFile::Write(const buffer_t& data) {
  if (!file_) {
    return false;
  }

  if (!data.size()) {
    NOTREACHED();
    return false;
  }

  size_t res = fwrite(data.data(), sizeof(byte_t), data.size(), file_);
  return res == data.size();
}

bool ANSIFile::Write(const std::string& data) {
  if (!file_) {
    return false;
  }

  if (!data.length()) {
    NOTREACHED();
    return false;
  }

  size_t res = fwrite(data.c_str(), sizeof(byte_t), data.length(), file_);
  return res == data.length();
}

bool ANSIFile::Write(const common::string16& data) {
  if (!file_) {
    return false;
  }

  if (!data.length()) {
    NOTREACHED();
    return false;
  }

  size_t res = fwrite(data.c_str(), sizeof(string16::value_type), data.length(), file_);
  return res == data.length();
}

bool ANSIFile::Truncate(off_t pos) {
  if (!file_) {
    return false;
  }

  bool result = ftruncate(fileno(file_), pos) != ERROR_RESULT_VALUE;
  return result;
}

void ANSIFile::Flush() {
  if (!file_) {
    return;
  }

  fflush(file_);
}

bool ANSIFile::IsOpened() const {
  return file_ != NULL;
}

void ANSIFile::Close() {
  if (file_) {
    fclose(file_);
    file_ = NULL;
  }
}

}  // namespace file_system

bool ConvertFromString(const std::string& from, file_system::ascii_string_path* out) {
  if (!out) {
    return false;
  }

  *out = file_system::ascii_string_path(from);
  return true;
}

bool ConvertFromString16(const string16& from, file_system::utf_string_path* out) {
  if (!out) {
    return false;
  }

  *out = file_system::utf_string_path(from);
  return true;
}

std::string ConvertToString(const file_system::ascii_string_path& path) {
  return path.Path();
}

string16 ConvertFromString16(const file_system::utf_string_path& path) {
  return path.Path();
}

}  // namespace common
