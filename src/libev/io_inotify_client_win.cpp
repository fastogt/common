/*  Copyright (C) 2014-2018 FastoGT. All right reserved.

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

#include <common/libev/io_inotify_client.h>

namespace common {
namespace libev {

IoInotifyClient::IoInotifyClient(IoLoop* server, IoInotifyClientObserver* client, flags_t flags)
    : base_class(server, INVALID_DESCRIPTOR, flags), client_(client) {}

IoInotifyClient::~IoInotifyClient() {}

common::ErrnoError IoInotifyClient::WatchDirectory(const file_system::ascii_directory_string_path& direcotry,
                                                   uint32_t mask) {
  UNUSED(direcotry);
  UNUSED(mask);
  return common::ErrnoError();
}

void IoInotifyClient::ProcessRead() {}

Optional<InotifyNode> IoInotifyClient::FindInotifyNodeByDescriptor(descriptor_t fd) const {
  if (fd == INVALID_DESCRIPTOR) {
    return Optional<InotifyNode>();
  }

  for (size_t i = 0; i < watched_directories_.size(); ++i) {
    if (watched_directories_[i].fd == fd) {
      return watched_directories_[i];
    }
  }

  return Optional<InotifyNode>();
}

ErrnoError IoInotifyClient::DoClose() {
  descriptor_t inode = GetFd();
  if (inode == INVALID_DESCRIPTOR) {
    watched_directories_.clear();
    return ErrnoError();
  }

  watched_directories_.clear();
  return base_class::DoClose();
}

}  // namespace libev
}  // namespace common
