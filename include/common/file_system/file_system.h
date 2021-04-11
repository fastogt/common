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
#include <common/types.h>

namespace common {
namespace file_system {

ErrnoError get_file_size_by_descriptor(descriptor_t fd_desc, off_t* size);
ErrnoError get_file_size_by_path(const std::string& path, off_t* size) WARN_UNUSED_RESULT;

ErrnoError Ftruncate(descriptor_t fd_desc, off_t lenght) WARN_UNUSED_RESULT;
ErrnoError unlink(const std::string& path) WARN_UNUSED_RESULT;
ErrnoError clear_file_by_descriptor(descriptor_t fd_desc) WARN_UNUSED_RESULT;
ErrnoError close_descriptor(descriptor_t fd_desc) WARN_UNUSED_RESULT;
ErrnoError lock_descriptor(descriptor_t fd_desc) WARN_UNUSED_RESULT;
ErrnoError unlock_descriptor(descriptor_t fd_desc) WARN_UNUSED_RESULT;
ErrnoError seek_descriptor(descriptor_t fd_desc, off_t offset, int whence) WARN_UNUSED_RESULT;

ErrnoError open_descriptor(const std::string& path, int oflags, descriptor_t* out_desc) WARN_UNUSED_RESULT;
extern ErrnoError create_node(const std::string& path) WARN_UNUSED_RESULT;
ErrnoError set_blocking_descriptor(descriptor_t descr, bool blocking);
ErrnoError touch(const std::string& path) WARN_UNUSED_RESULT;
typedef ErrnoError (*read_cb)(const char* buff, size_t buff_len, void* user_data, size_t* processed);
ErrnoError read_file_cb(int in_fd, off_t* offset, size_t count, read_cb cb, void* user_data) WARN_UNUSED_RESULT;

ErrnoError copy_file(const std::string& path_from, const std::string& path_to) WARN_UNUSED_RESULT;
ErrnoError move_file(const std::string& path_from, const std::string& path_to) WARN_UNUSED_RESULT;
ErrnoError remove_file(const std::string& file_path) WARN_UNUSED_RESULT;
ErrnoError node_access(const std::string& node) WARN_UNUSED_RESULT;
ErrnoError create_directory(const std::string& path, bool is_recursive) WARN_UNUSED_RESULT;
ErrnoError remove_directory(const std::string& path, bool is_recursive) WARN_UNUSED_RESULT;
ErrnoError change_directory(const std::string& path) WARN_UNUSED_RESULT;

ErrnoError write_to_descriptor(descriptor_t fd_desc, const void* buf, size_t size, size_t* nwrite_out)
    WARN_UNUSED_RESULT;
ErrnoError read_from_descriptor(descriptor_t fd_desc, void* buf, size_t size, size_t* nread_out) WARN_UNUSED_RESULT;

ErrnoError get_file_time_last_modification(const std::string& file_path,
                                           utctime_t* mod_time_sec) WARN_UNUSED_RESULT;  // utc time

bool find_file_in_path(const std::string& file_name, std::string* out_path) WARN_UNUSED_RESULT;

bool read_file_to_string(const std::string& path, std::string* contents);
bool read_file_to_string_with_max_size(const std::string& path, std::string* contents, off_t max_size);

}  // namespace file_system
}  // namespace common
