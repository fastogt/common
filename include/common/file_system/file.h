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

#include <common/error.h>  // for ErrnoError, Error
#include <common/file_system/path.h>
#include <common/sprintf.h>

namespace common {
namespace file_system {

class DescriptorHolder;

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

  File();  // IsValid fasle
  ~File();

  ErrnoError Open(const path_type::value_type& file_path, uint32_t flags) WARN_UNUSED_RESULT;
  ErrnoError Open(const path_type& file_path, uint32_t flags) WARN_UNUSED_RESULT;
  path_type GetPath() const;
  bool IsValid() const;
  bool IsOpen() const;

  descriptor_t GetFd() const;

  ErrnoError Write(const void* data, size_t size, size_t* nwrite_out) WARN_UNUSED_RESULT;
  ErrnoError Write(const char* data, size_t size, size_t* nwrite_out) WARN_UNUSED_RESULT;
  ErrnoError WriteBuffer(const std::string& data, size_t* nwrite_out) WARN_UNUSED_RESULT;
  ErrnoError WriteBuffer(const char_buffer_t& data, size_t* nwrite_out) WARN_UNUSED_RESULT;
  ErrnoError Read(void* out, size_t len, size_t* nread_out) WARN_UNUSED_RESULT;

  ErrnoError Close();

  ErrnoError Lock() WARN_UNUSED_RESULT;
  ErrnoError Unlock() WARN_UNUSED_RESULT;

  ErrnoError Seek(off_t offset, int whence) WARN_UNUSED_RESULT;

  ErrnoError Truncate(off_t pos) WARN_UNUSED_RESULT;

 private:
  DescriptorHolder* holder_;
  path_type file_path_;

  DISALLOW_COPY_AND_ASSIGN(File);
};

class ANSIFile {
 public:
  typedef ascii_string_path path_type;

  ANSIFile();
  ~ANSIFile();

  ErrnoError Open(const path_type::value_type& file_path, const char* mode) WARN_UNUSED_RESULT;
  ErrnoError Open(const path_type& file_path, const char* mode) WARN_UNUSED_RESULT;
  bool IsValid() const;
  bool IsOpen() const;

  ErrnoError Lock() WARN_UNUSED_RESULT;
  ErrnoError Unlock() WARN_UNUSED_RESULT;

  ErrnoError Seek(off_t offset, int whence) WARN_UNUSED_RESULT;
  ErrnoError GetSize(size_t* size) WARN_UNUSED_RESULT;

  bool Read(byte_t* out, size_t len, size_t* nread_out);
  bool Read(buffer_t* out_data, size_t max_size);
  bool Read(std::string* out_data, size_t max_size);
  bool ReadLine(buffer_t* out_data);
  bool ReadLine(std::string* out_data);

  bool Write(const buffer_t& data);
  bool Write(const char_buffer_t& data);
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

  ErrnoError Truncate(off_t pos) WARN_UNUSED_RESULT;

  ErrnoError Flush() WARN_UNUSED_RESULT;
  ErrnoError Close();

  bool IsEOF() const;

 private:
  path_type path_;
  FILE* file_;

  DISALLOW_COPY_AND_ASSIGN(ANSIFile);
};

template <typename File>
class FileGuard : public File {
 public:
  typedef File base_class;

  template <typename... Args>
  explicit FileGuard(Args... args) : base_class(args...) {}

  ~FileGuard() {
    ErrnoError err = base_class::Close();
    DCHECK(!err) << "Close client error: " << err->GetDescription();
  }

 private:
  using base_class::Close;
  DISALLOW_COPY_AND_ASSIGN(FileGuard);
};

}  // namespace file_system
}  // namespace common
