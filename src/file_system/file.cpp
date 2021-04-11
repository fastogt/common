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

#include <common/file_system/file.h>

#include <limits>

#include <errno.h>
#include <fcntl.h>
#include <string.h>

#include <common/file_system/descriptor_holder.h>
#include <common/file_system/file_system.h>

namespace common {
namespace file_system {

File::File() : holder_(nullptr), file_path_() {}

File::~File() {
  destroy(&holder_);
}

File::path_type File::GetPath() const {
  return file_path_;
}

bool File::IsValid() const {
  return holder_ && holder_->IsValid();
}

bool File::IsOpen() const {
  return IsValid();
}

descriptor_t File::GetFd() const {
  return holder_->GetFd();
}

ErrnoError File::Write(const void* data, size_t size, size_t* nwrite_out) {
  DCHECK(IsValid());

  if (!holder_) {
    return make_error_perror("File::Write", EINVAL);
  }

  return holder_->Write(data, size, nwrite_out);
}

ErrnoError File::Write(const char* data, size_t size, size_t* nwrite_out) {
  DCHECK(IsValid());

  if (!holder_) {
    return make_error_perror("File::Write", EINVAL);
  }

  return holder_->Write(data, size, nwrite_out);
}

ErrnoError File::WriteBuffer(const std::string& data, size_t* nwrite_out) {
  DCHECK(IsValid());

  if (!holder_) {
    return make_error_perror("File::Write", EINVAL);
  }

  return holder_->WriteBuffer(data, nwrite_out);
}

ErrnoError File::WriteBuffer(const char_buffer_t& data, size_t* nwrite_out) {
  DCHECK(IsValid());

  if (!holder_) {
    return make_error_perror("File::Write", EINVAL);
  }

  return holder_->WriteBuffer(data, nwrite_out);
}

ErrnoError File::Read(void* out, size_t len, size_t* nread_out) {
  DCHECK(IsValid());

  if (!holder_) {
    return make_error_perror("File::Read", EINVAL);
  }

  return holder_->Read(out, len, nread_out);
}

ErrnoError File::Close() {
  DCHECK(IsValid());

  if (!holder_) {
    return make_error_perror("File::Close", EINVAL);
  }

  file_path_ = path_type();
  return holder_->Close();
}

ErrnoError File::Lock() {
  DCHECK(IsValid());

  if (!holder_) {
    return make_error_perror("File::Lock", EINVAL);
  }

  return holder_->Lock();
}

ErrnoError File::Unlock() {
  DCHECK(IsValid());

  if (!holder_) {
    return make_error_perror("File::Unlock", EINVAL);
  }

  return holder_->Unlock();
}

ErrnoError File::Seek(off_t offset, int whence) {
  DCHECK(IsValid());

  if (!holder_) {
    return make_error_perror("File::Seek", EINVAL);
  }

  return holder_->Seek(offset, whence);
}

ErrnoError File::Truncate(off_t pos) {
  DCHECK(IsValid());

  if (!holder_) {
    return make_error_perror("File::Truncate", EINVAL);
  }

  return holder_->Truncate(pos);
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

#if defined(OS_WIN)  // binary can be only on windows
  if (flags & FLAG_OPEN_BINARY) {
    open_flags |= O_BINARY;
  }
#endif

  if (!open_flags && !(flags & FLAG_OPEN)) {
    DNOTREACHED();
    errno = EOPNOTSUPP;
    return make_error_perror("write_to_descriptor", EOPNOTSUPP);
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
  const std::string path_str = file_path.GetPath();
  ErrnoError err = open_descriptor(path_str, open_flags, &fd);
  if (err) {
    return err;
  }

  if (flags & FLAG_DELETE_ON_CLOSE) {
    err = unlink(path_str);
    DCHECK(!err) << err->GetDescription();
  }

  holder_ = new DescriptorHolder(fd);
  file_path_ = file_path;
  return ErrnoError();
}

ANSIFile::ANSIFile() : path_(), file_(nullptr) {}

ANSIFile::~ANSIFile() {}

ErrnoError ANSIFile::Open(const path_type::value_type& file_path, const char* mode) {
  return Open(path_type(file_path), mode);
}

ErrnoError ANSIFile::Open(const path_type& file_path, const char* mode) {
  DCHECK(!IsValid());

  std::string spath = file_path.GetPath();
  const char* path = spath.c_str();
  file_ = fopen(path, mode);
  if (file_) {
    path_ = file_path;
    return ErrnoError();
  }

  return make_error_perror("ANSIFile::Open", errno);
}

ErrnoError ANSIFile::Lock() {
  DCHECK(IsValid());

  if (!file_) {
    return make_error_perror("ANSIFile::Unlock", EINVAL);
  }

  int fd = fileno(file_);
  return lock_descriptor(fd);
}

ErrnoError ANSIFile::Unlock() {
  DCHECK(IsValid());

  if (!file_) {
    return make_error_perror("ANSIFile::Unlock", EINVAL);
  }

  int fd = fileno(file_);
  return unlock_descriptor(fd);
}

ErrnoError ANSIFile::Seek(off_t offset, int whence) {
  DCHECK(IsValid());

  if (!file_) {
    return make_error_perror("ANSIFile::Unlock", EINVAL);
  }

  int res = fseek(file_, offset, whence);
  if (res == ERROR_RESULT_VALUE) {
    return make_error_perror("fseek", errno);
  }

  return ErrnoError();
}

ErrnoError ANSIFile::GetSize(size_t* size) {
  DCHECK(IsValid());
  if (!size) {
    return make_errno_error_inval();
  }

  if (!file_) {
    return make_error_perror("ANSIFile::GetSize", EINVAL);
  }

  off_t old_pos = ftell(file_);
  ErrnoError errn = Seek(0, SEEK_END);
  if (errn) {
    return errn;
  }

  size_t fsize = ftell(file_);
  errn = Seek(old_pos, SEEK_SET);
  if (errn) {
    return errn;
  }

  *size = fsize;
  return ErrnoError();
}

bool ANSIFile::Read(byte_t* out, size_t len, size_t* nread_out) {
  DCHECK(IsValid());

  if (!file_ || !out || !nread_out) {
    return false;
  }

  size_t res = fread(out, sizeof(byte_t), len, file_);
  if (res > 0) {
    *nread_out = res;
    return true;
  } else if (feof(file_)) {
    return false;
  }

  return false;
}

bool ANSIFile::Read(buffer_t* out_data, size_t max_size) {
  DCHECK(IsValid());

  if (!file_ || !out_data) {
    return false;
  }

  byte_t* data = static_cast<byte_t*>(malloc(max_size));
  if (!data) {
    return false;
  }

  size_t res = fread(data, sizeof(byte_t), max_size, file_);
  if (res > 0) {
    *out_data = buffer_t(data, data + res);
  } else if (feof(file_)) {
    *out_data = buffer_t();
    free(data);
    return false;
  }

  free(data);
  return true;
}

bool ANSIFile::Read(std::string* out_data, size_t max_size) {
  DCHECK(IsValid());

  if (!file_ || !out_data) {
    return false;
  }

  char* data = static_cast<char*>(malloc(max_size));
  if (!data) {
    return false;
  }

  size_t res = fread(data, sizeof(char), max_size, file_);
  if (res > 0) {
    *out_data = std::string(data, res);
  } else if (feof(file_)) {
    *out_data = std::string();
    free(data);
    return false;
  }

  free(data);
  return true;
}

bool ANSIFile::ReadLine(buffer_t* out_data) {
  DCHECK(IsValid());

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
  DCHECK(IsValid());

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
  DCHECK(IsValid());

  if (!file_) {
    return true;
  }

  return feof(file_) != 0;
}

bool ANSIFile::Write(const buffer_t& data) {
  DCHECK(IsValid());

  if (!file_ || data.empty()) {
    DNOTREACHED();
    return false;
  }

  size_t res = fwrite(data.data(), sizeof(byte_t), data.size(), file_);
  return res == data.size();
}

bool ANSIFile::Write(const char_buffer_t& data) {
  DCHECK(IsValid());

  if (!file_ || data.empty()) {
    DNOTREACHED();
    return false;
  }

  size_t res = fwrite(data.data(), sizeof(byte_t), data.size(), file_);
  return res == data.size();
}

bool ANSIFile::Write(const std::string& data) {
  DCHECK(IsValid());

  if (!file_ || data.empty()) {
    DNOTREACHED();
    return false;
  }

  size_t res = fwrite(data.c_str(), sizeof(byte_t), data.length(), file_);
  return res == data.size();
}

bool ANSIFile::Write(const string16& data) {
  DCHECK(IsValid());

  if (!file_ || data.empty()) {
    DNOTREACHED();
    return false;
  }

  size_t res = fwrite(data.c_str(), sizeof(string16::value_type), data.length(), file_);
  return res == data.size();
}

ErrnoError ANSIFile::Truncate(off_t pos) {
  DCHECK(IsValid());

  if (!file_) {
    return make_error_perror("ANSIFile::Truncate", EINVAL);
  }

  return Ftruncate(fileno(file_), pos);
}

ErrnoError ANSIFile::Flush() {
  DCHECK(IsValid());

  if (!file_) {
    return make_error_perror("ANSIFile::Flush", EINVAL);
  }

  fflush(file_);
  return ErrnoError();
}

bool ANSIFile::IsValid() const {
  return file_ != nullptr;
}

bool ANSIFile::IsOpen() const {
  return IsValid();
}

ErrnoError ANSIFile::Close() {
  DCHECK(IsValid());

  if (!file_) {
    return make_error_perror("ANSIFile::Close", EINVAL);
  }

  fclose(file_);
  file_ = nullptr;
  return ErrnoError();
}

ErrnoError create_node(const std::string& path) {
  if (path.empty()) {
    return make_error_perror("create_node", EINVAL);
  }

  File fl;
  ErrnoError err = fl.Open(path, File::FLAG_OPEN | File::FLAG_CREATE | File::FLAG_WRITE);
  if (err) {
    return err;
  }

  ignore_result(fl.Close());
  return ErrnoError();
}

bool read_file_to_string(const std::string& path, std::string* contents) {
  return read_file_to_string_with_max_size(path, contents, std::numeric_limits<off_t>::max());
}

bool read_file_to_string_with_max_size(const std::string& path, std::string* contents, off_t max_size) {
  if (path.empty() || !contents || max_size == 0) {
    return false;
  }

  FILE* file = fopen(path.c_str(), "rb");
  if (!file) {
    return false;
  }

  // Many files supplied in |path| have incorrect size (proc files etc).
  // Hence, the file is read sequentially as opposed to a one-shot read, using
  // file size as a hint for chunk size if available.
  constexpr off_t kDefaultChunkSize = 1 << 16;
  off_t chunk_size;
#if !defined(OS_NACL_NONSFI)
  common::ErrnoError err = get_file_size_by_path(path, &chunk_size);
  if (err) {
    chunk_size = kDefaultChunkSize - 1;
  }
  // We need to attempt to read at EOF for feof flag to be set so here we
  // use |chunk_size| + 1.
  chunk_size = std::min<off_t>(chunk_size, max_size) + 1;
#else
  chunk_size = kDefaultChunkSize;
#endif  // !defined(OS_NACL_NONSFI)
  size_t bytes_read_this_pass;
  size_t bytes_read_so_far = 0;
  bool read_status = true;
  std::string local_contents;
  local_contents.resize(chunk_size);

  while ((bytes_read_this_pass = fread(&local_contents[bytes_read_so_far], 1, chunk_size, file)) > 0) {
    if ((max_size - bytes_read_so_far) < bytes_read_this_pass) {
      // Read more than max_size bytes, bail out.
      bytes_read_so_far = max_size;
      read_status = false;
      break;
    }
    // In case EOF was not reached, iterate again but revert to the default
    // chunk size.
    if (bytes_read_so_far == 0) {
      chunk_size = kDefaultChunkSize;
    }

    bytes_read_so_far += bytes_read_this_pass;
    // Last fread syscall (after EOF) can be avoided via feof, which is just a
    // flag check.
    if (feof(file)) {
      break;
    }
    local_contents.resize(bytes_read_so_far + chunk_size);
  }

  read_status = read_status && !ferror(file);
  fclose(file);
  contents->swap(local_contents);
  contents->resize(bytes_read_so_far);

  return read_status;
}

}  // namespace file_system
}  // namespace common
