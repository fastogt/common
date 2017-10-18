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

#include <common/file_system/file.h>

#include <errno.h>
#include <fcntl.h>
#include <string.h>

#include <common/file_system/descriptor_holder.h>
#include <common/file_system/file_system.h>

namespace common {
namespace file_system {

File::File() : holder_(nullptr), file_path_() {}

File::~File() {
  delete holder_;
}

File::path_type File::GetPath() const {
  return file_path_;
}

bool File::IsValid() const {
  return holder_ && holder_->IsValid();
}

ErrnoError File::Write(const buffer_t& data, size_t* nwrite_out) {
  DCHECK(IsValid());
  return holder_->Write(data, nwrite_out);
}

descriptor_t File::GetFd() const {
  DCHECK(IsValid());
  return holder_->GetFd();
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

ErrnoError File::Seek(off_t offset, int whence) {
  DCHECK(IsValid());
  return holder_->Seek(offset, whence);
}

ErrnoError File::Truncate(off_t pos) {
  DCHECK(IsValid());
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

#ifdef OS_WIN  // binary can be only on windows
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
    unlink(path_str);
  }

  holder_ = new DescriptorHolder(fd);
  file_path_ = file_path;
  return ErrnoError();
}

ANSIFile::ANSIFile(const path_type& file_path) : path_(file_path), file_(NULL) {}

ANSIFile::~ANSIFile() {}

ErrnoError ANSIFile::Open(const char* mode) {
  if (!file_) {
    std::string spath = path_.GetPath();
    const char* path = spath.c_str();
    file_ = fopen(path, mode);
    if (file_) {
      return ErrnoError();
    }
    return make_error_perror("ANSIFile::Open", errno);
  }

  return ErrnoError();
}

ErrnoError ANSIFile::Lock() {
  if (!file_) {
    return make_error_perror("ANSIFile::Unlock", EINVAL);
  }

  int fd = fileno(file_);
  return lock_descriptor(fd);
}

ErrnoError ANSIFile::Unlock() {
  if (!file_) {
    return make_error_perror("ANSIFile::Unlock", EINVAL);
  }

  int fd = fileno(file_);
  return unlock_descriptor(fd);
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
  if (!file_ || data.empty()) {
    DNOTREACHED();
    return false;
  }

  size_t res = fwrite(data.data(), sizeof(byte_t), data.size(), file_);
  return res == data.size();
}

bool ANSIFile::Write(const std::string& data) {
  if (!file_ || data.empty()) {
    DNOTREACHED();
    return false;
  }

  size_t res = fwrite(data.c_str(), sizeof(byte_t), data.length(), file_);
  return res == data.length();
}

bool ANSIFile::Write(const string16& data) {
  if (!file_ || data.empty()) {
    DNOTREACHED();
    return false;
  }

  size_t res = fwrite(data.c_str(), sizeof(string16::value_type), data.length(), file_);
  return res == data.length();
}

ErrnoError ANSIFile::Truncate(off_t pos) {
  if (!file_) {
    return make_error_perror("ANSIFile::Truncate", EINVAL);
  }

  return ftruncate(fileno(file_), pos);
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

ErrnoError create_node(const std::string& path) {
  if (path.empty()) {
    return make_error_perror("create_node", EINVAL);
  }

  File fl;
  return fl.Open(path, File::FLAG_OPEN | File::FLAG_CREATE | File::FLAG_WRITE);
}

}  // namespace file_system
}  // namespace common
