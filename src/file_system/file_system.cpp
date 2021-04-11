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

#include <common/file_system/file_system.h>

#include <dirent.h>
#include <errno.h>
#include <fcntl.h>     // for open, SEEK_CUR, SEEK_SET, etc
#include <sys/stat.h>  // for stat, S_IRWXG, S_IRWXO, etc
#include <unistd.h>    // for close, lseek, read, ssize_t, etc

#if defined(OS_WIN)
#include <winsock2.h>
#endif

#if defined(COMPILER_MSVC)
#include <direct.h>
#include <io.h>
#include <windows.h>
#define mkdir _mkdir
int ftruncate(int fd, int64_t length) {
  HANDLE fh = reinterpret_cast<HANDLE>(_get_osfhandle(fd));
  if (!fh || _lseeki64(fd, length, SEEK_SET)) {
    return -1;
  }
  return SetEndOfFile(fh) ? 0 : -1;
}
#if !defined(S_ISDIR)
#define S_ISDIR(mode) ((mode & S_IFMT) == S_IFDIR)
#endif
#endif

#include <common/sprintf.h>

#include <common/file_system/string_path_utils.h>

#define FS_BUF_SIZE 1024 * 4

namespace {

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

  while ((nread = read(fd_from, buf, sizeof buf)) > 0) {
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
    return make_error_perror("call_fcntl_flock", EINVAL);
  }
#if defined(OS_POSIX)
  struct flock lock;
  lock.l_type = do_lock ? F_WRLCK : F_UNLCK;
  lock.l_whence = SEEK_SET;
  lock.l_start = 0;
  lock.l_len = 0;  // Lock entire file.
  int res = fcntl(fd_desc, F_SETLK, &lock);
  if (res == -1) {
    return make_error_perror("fcntl", errno);
  }
#else
  HANDLE fh = reinterpret_cast<HANDLE>(_get_osfhandle(fd_desc));
  if (!fh) {
    return make_error_perror("_get_osfhandle", EINVAL);
  }
  BOOL result = do_lock ? LockFile(fh, 0, 0, MAXDWORD, MAXDWORD) : UnlockFile(fh, 0, 0, MAXDWORD, MAXDWORD);
  if (!result) {
    return make_error_perror(do_lock ? "LockFile" : "UnlockFile", errno);
  }
#endif
  return ErrnoError();
}

ErrnoError do_rmdir_directory(const char* path) {
  bool result = rmdir(path) != ERROR_RESULT_VALUE;
  if (!result && errno != ENOENT) {
    return make_error_perror("rmdir", errno);
  }

  return ErrnoError();
}

ErrnoError do_create_directory(const char* path) {
#if defined(OS_WIN)
  bool result = mkdir(path) != ERROR_RESULT_VALUE;
#else
  bool result = mkdir(path, S_IRWXU | S_IRWXG | S_IRWXO) != ERROR_RESULT_VALUE;
#endif
  if (!result && errno != EEXIST) {
    return make_error_perror("mkdir", errno);
  }

  return ErrnoError();
}

}  // namespace

ErrnoError get_file_size_by_descriptor(descriptor_t fd_desc, off_t* size) {
  if (fd_desc == INVALID_DESCRIPTOR || !size) {
    return make_error_perror("get_file_size_by_descriptor", EINVAL);
  }

  struct stat sb;
  if (::fstat(fd_desc, &sb) == ERROR_RESULT_VALUE) {
    return make_error_perror("get_file_size_by_descriptor", errno);
  }

  *size = sb.st_size;
  return ErrnoError();
}

ErrnoError get_file_size_by_path(const std::string& path, off_t* size) {
  if (path.empty() || !size) {
    return make_error_perror("get_file_size_by_path", EINVAL);
  }

  const char* file_path_cstr = path.c_str();
  struct stat sb;
  if (::stat(file_path_cstr, &sb) == ERROR_RESULT_VALUE) {
    return make_error_perror("get_file_size_by_path", errno);
  }

  if (!S_ISREG(sb.st_mode)) {
    DNOTREACHED();
    return make_error_perror("get_file_size_by_path", EINVAL);
  }

  *size = sb.st_size;
  return ErrnoError();
}

ErrnoError Ftruncate(descriptor_t fd_desc, off_t lenght) {
  if (fd_desc == INVALID_DESCRIPTOR) {
    return make_error_perror("ftruncate", EINVAL);
  }

  if (::ftruncate(fd_desc, lenght) == ERROR_RESULT_VALUE) {
    return make_error_perror("ftruncate", errno);
  }

  return ErrnoError();
}

ErrnoError unlink(const std::string& path) {
  if (path.empty()) {
    return make_error_perror("unlink", EINVAL);
  }

  if (::unlink(path.c_str()) == ERROR_RESULT_VALUE) {
    return make_error_perror("unlink", errno);
  }

  return ErrnoError();
}

ErrnoError clear_file_by_descriptor(descriptor_t fd_desc) {
  if (fd_desc == INVALID_DESCRIPTOR) {
    return make_error_perror("clear_file_by_descriptor", EINVAL);
  }

  return Ftruncate(fd_desc, 0);
}

ErrnoError touch(const std::string& path) {
  return create_node(path);
}

ErrnoError set_blocking_descriptor(descriptor_t descr, bool blocking) {
#if defined(OS_POSIX)
  int opts = fcntl(descr, F_GETFL);
  if (opts < 0) {
    return make_error_perror("fcntl(F_GETFL)", errno);
  }

  if (blocking) {
    opts &= ~O_NONBLOCK;
  } else {
    opts |= O_NONBLOCK;
  }

  if (fcntl(descr, F_SETFL, opts) < 0) {
    return make_error_perror("fcntl(F_SETFL)", errno);
  }

  return ErrnoError();
#else
  unsigned long flags = blocking;
  int res = ioctlsocket(descr, FIONBIO, &flags);
  if (res == SOCKET_ERROR) {
    return make_error_perror("ioctlsocket", errno);
  }

  return ErrnoError();
#endif
}

ErrnoError read_file_cb(int in_fd, off_t* offset, size_t count, read_cb cb, void* user_data) {
  if (!cb || in_fd == INVALID_DESCRIPTOR) {
    return make_error_perror("read_file_cb", EINVAL);
  }

  off_t orig = 0;
  char buf[FS_BUF_SIZE] = {0};

  if (offset) {
    /* Save current file offset and set offset to value in '*offset' */

    orig = lseek(in_fd, 0, SEEK_CUR);
    if (orig == -1) {
      return make_error_perror("lseek", errno);
    }
    if (lseek(in_fd, *offset, SEEK_SET) == -1) {
      return make_error_perror("lseek", errno);
    }
  }

  while (count > 0) {
    ssize_t num_read = read(in_fd, buf, FS_BUF_SIZE);
    if (num_read == -1) {
      return make_error_perror("read", errno);
    }
    if (num_read == 0) {
      break; /* EOF */
    }

    size_t num_sent = 0;
    ErrnoError err = cb(buf, num_read, user_data, &num_sent);
    if (err) {
      return err;
    }

    count -= num_sent;
  }

  if (offset) {
    /* Return updated file offset in '*offset', and reset the file offset
       to the value it had when we were called. */

    *offset = lseek(in_fd, 0, SEEK_CUR);
    if (*offset == -1) {
      return make_error_perror("lseek", errno);
    }
    if (lseek(in_fd, orig, SEEK_SET) == -1) {
      return make_error_perror("lseek", errno);
    }
  }

  return ErrnoError();
}

ErrnoError copy_file(const std::string& pathFrom, const std::string& pathTo) {
  if (pathFrom.empty()) {
    return make_error_perror("copy_file", EINVAL);
  }

  if (pathTo.empty()) {
    return make_error_perror("copy_file", EINVAL);
  }

  std::string pr_from = prepare_path(pathFrom);
  std::string pr_to = prepare_path(pathTo);

  int cpRes = cp(pr_from.c_str(), pr_to.c_str());
  bool result = cpRes != ERROR_RESULT_VALUE;
  if (!result) {
    return make_error_perror("cp", errno);
  }

  return ErrnoError();
}

ErrnoError move_file(const std::string& pathFrom, const std::string& pathTo) {
  if (pathFrom.empty()) {
    return make_error_perror("move_file", EINVAL);
  }

  if (pathTo.empty()) {
    return make_error_perror("move_file", EINVAL);
  }

  std::string pr_from = prepare_path(pathFrom);
  std::string pr_to = prepare_path(pathTo);
#if defined(OS_WIN)
  WINBOOL res = MoveFileExA(pr_from.c_str(), pr_to.c_str(), MOVEFILE_COPY_ALLOWED | MOVEFILE_REPLACE_EXISTING);
  bool result = res != 0;
#else
  int res = rename(pr_from.c_str(), pr_to.c_str());
  bool result = res != ERROR_RESULT_VALUE;
#endif
  if (!result) {
    return make_error_perror("rename", errno);
  }
  return ErrnoError();
}

ErrnoError remove_file(const std::string& file_path) {
  if (file_path.empty()) {
    return make_error_perror("remove_file", EINVAL);
  }

  std::string pr_to = prepare_path(file_path);

  int res = remove(pr_to.c_str());
  bool result = res != ERROR_RESULT_VALUE;
  if (!result && errno != ENOENT) {
    return make_error_perror("remove", errno);
  }

  return ErrnoError();
}

ErrnoError node_access(const std::string& node) {
  if (node.empty()) {
    return make_error_perror("node_access", EINVAL);
  }

  std::string pr_node = prepare_path(node);
  if (pr_node.empty()) {
    return make_error_perror("node_access", EINVAL);
  }

  int res = access(pr_node.c_str(), W_OK);
  if (res == ERROR_RESULT_VALUE) {
    return make_error_perror("access", errno);
  }

  return ErrnoError();
}

ErrnoError create_directory(const std::string& path, bool is_recursive) {
  if (path.empty()) {
    return make_error_perror("create_directory", EINVAL);
  }

  std::string pr_path = prepare_path(path);
  if (pr_path.empty()) {
    return make_error_perror("create_directory", EINVAL);
  }

  pr_path = stable_dir_path(pr_path);
  if (pr_path.empty()) {
    return make_error_perror("create_directory", EINVAL);
  }

  const char* pr_path_ptr = pr_path.c_str();
  if (is_recursive) {
    char* p = nullptr;
#if defined(OS_WIN)
    uint8_t shift = 3;
#else
    uint8_t shift = 1;
#endif
    for (p = const_cast<char*>(pr_path_ptr + shift); *p; p++) {
      if (*p == get_separator<char>()) {
        *p = 0;
        const char* path = pr_path_ptr;

        if (!is_directory_exist(path)) {
          ErrnoError err = do_create_directory(path);
          if (err) {
            return err;
          }
        }

        *p = get_separator<char>();
      }
    }
  }

  return do_create_directory(pr_path_ptr);
}

ErrnoError remove_directory(const std::string& path, bool is_recursive) {
  if (path.empty()) {
    return make_error_perror("remove_directory", EINVAL);
  }

  std::string pr_path = prepare_path(path);
  if (pr_path[pr_path.length() - 1] == get_separator<char>()) {
    pr_path[pr_path.length() - 1] = 0;
  }

  const char* pr_path_ptr = pr_path.c_str();
  if (is_recursive) {
    DIR* dirp = opendir(pr_path_ptr);
    if (!dirp) {
      return ErrnoError();
    }

    struct dirent* p;
    while ((p = readdir(dirp)) != nullptr) {
      /* Skip the names "." and ".." as we don't want to recurse on them. */
      if (!strcmp(p->d_name, ".") || !strcmp(p->d_name, "..")) {
        continue;
      }

      char pathBuffer[PATH_MAX] = {0};
      SNPrintf(pathBuffer, sizeof(pathBuffer), "%s/%s", path, p->d_name);
      struct stat statbuf;
      if (!::stat(pathBuffer, &statbuf)) {
        if (S_ISDIR(statbuf.st_mode)) {
          ErrnoError err = remove_directory(pathBuffer, is_recursive);
          if (err) {
            closedir(dirp);
            return err;
          }
        } else {
          ErrnoError err = remove_file(pathBuffer);
          if (err) {
            closedir(dirp);
            return err;
          }
        }
      }
    }
    closedir(dirp);
  }

  return do_rmdir_directory(pr_path_ptr);
}

ErrnoError change_directory(const std::string& path) {
  if (path.empty()) {
    return make_error_perror("change_directory", EINVAL);
  }

  std::string spath = stable_dir_path(path);
  bool result = chdir(spath.c_str()) != ERROR_RESULT_VALUE;
  if (!result) {
    return make_error_perror("chdir", errno);
  }

  return ErrnoError();
}

ErrnoError open_descriptor(const std::string& path, int oflags, descriptor_t* out_desc) {
  if (path.empty() || !out_desc) {
    return make_error_perror("open_descriptor", EINVAL);
  }

  static const int mode = S_IRUSR | S_IWUSR;
  descriptor_t desc = open(path.c_str(), oflags, mode);
  if (desc == INVALID_DESCRIPTOR) {
    return make_error_perror("open", errno);
  }

  *out_desc = desc;
  return ErrnoError();
}

ErrnoError lock_descriptor(descriptor_t fd_desc) {
  return call_fcntl_flock(fd_desc, true);
}

ErrnoError unlock_descriptor(descriptor_t fd_desc) {
  return call_fcntl_flock(fd_desc, false);
}

ErrnoError seek_descriptor(descriptor_t fd_desc, off_t offset, int whence) {
  if (fd_desc == INVALID_DESCRIPTOR) {
    return make_error_perror("open_descriptor", EINVAL);
  }

  int res = ::lseek(fd_desc, offset, whence);
  if (res == ERROR_RESULT_VALUE) {
    return make_error_perror("lseek", errno);
  }

  return ErrnoError();
}

ErrnoError close_descriptor(descriptor_t fd_desc) {
  if (fd_desc == INVALID_DESCRIPTOR) {
    return ErrnoError();
  }

  int res = close(fd_desc);
  if (res == ERROR_RESULT_VALUE) {
    return make_error_perror("close", errno);
  }

  return ErrnoError();
}

ErrnoError write_to_descriptor(descriptor_t fd_desc, const void* buf, size_t size, size_t* nwrite_out) {
  if (fd_desc == INVALID_DESCRIPTOR || !buf || size == 0 || !nwrite_out) {
    return make_error_perror("write_to_descriptor", EINVAL);
  }

  ssize_t res = write(fd_desc, buf, size);
  if (res == ERROR_RESULT_VALUE) {
    return make_error_perror("write", errno);
  }

  *nwrite_out = res;
  return ErrnoError();
}

ErrnoError read_from_descriptor(descriptor_t fd_desc, void* buf, size_t size, size_t* nread_out) {
  if (fd_desc == INVALID_DESCRIPTOR || !buf || size == 0 || !nread_out) {
    return make_error_perror("read_from_descriptor", EINVAL);
  }

  ssize_t lnread = read(fd_desc, buf, size);
  if (lnread == ERROR_RESULT_VALUE) {
    return make_error_perror("read", errno);
  }

  *nread_out = lnread;
  return ErrnoError();
}

ErrnoError get_file_time_last_modification(const std::string& file_path, utctime_t* mod_time_sec) {
  if (file_path.empty() || !mod_time_sec) {
    return make_error_perror("get_file_time_last_modification", EINVAL);
  }

  struct stat attrib;
  if (::stat(file_path.c_str(), &attrib) == ERROR_RESULT_VALUE) {
    return make_errno_error(errno);
  }

  *mod_time_sec = attrib.st_mtime;
  return ErrnoError();
}

bool find_file_in_path(const std::string& file_name, std::string* out_path) {
  if (file_name.empty() || !out_path) {
    return false;
  }

  std::stringstream path(getenv<char>("PATH"));
  while (!path.eof()) {
    std::string test;
    struct stat info;
#if defined(OS_WIN)
    getline(path, test, ';');
#else
    getline(path, test, ':');
#endif
    test = stable_dir_path(test);
    test.append(file_name);
    if (::stat(test.c_str(), &info) == 0) {
      *out_path = test;
      return true;
    }
  }

  return false;
}

}  // namespace file_system
}  // namespace common
