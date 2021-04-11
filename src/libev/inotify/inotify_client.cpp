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

#include <common/libev/inotify/inotify_client.h>

#include <sys/inotify.h>

#include <common/libev/inotify/inotify_client_observer.h>

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUF_SIZE (1024 * (EVENT_SIZE + 16))

namespace common {
namespace libev {
namespace inotify {

IoInotifyClient::IoInotifyClient(IoLoop* server, IoInotifyClientObserver* client, flags_t flags)
    : base_class(server, inotify_init(), flags), client_(client) {}

IoInotifyClient::~IoInotifyClient() {}

common::ErrnoError IoInotifyClient::WatchDirectory(const file_system::ascii_directory_string_path& direcotry,
                                                   uint32_t mask) {
  if (!direcotry.IsValid()) {
    return common::make_errno_error_inval();
  }

  descriptor_t inode = GetFd();
  if (inode == INVALID_DESCRIPTOR) {
    return common::make_errno_error("Invalid inode", EINVAL);
  }

  const std::string dir_str = direcotry.GetPath();
  descriptor_t watcher_fd = inotify_add_watch(inode, dir_str.c_str(), mask);
  if (watcher_fd == ERROR_RESULT_VALUE) {
    return common::make_errno_error(errno);
  }

  watched_directories_.push_back({watcher_fd, direcotry});
  return common::ErrnoError();
}

void IoInotifyClient::ProcessRead() {
  char data[BUF_SIZE] = {0};
  size_t nread = 0;
  common::ErrnoError errn = SingleRead(data, BUF_SIZE, &nread);
  if ((errn && errn->GetErrorCode() != EAGAIN) || nread == 0) {
    return;
  }

  size_t i = 0;
  while (i < nread) {
    struct inotify_event* event = reinterpret_cast<struct inotify_event*>(data + i);
    if (event->len) {
      const auto inode = FindInotifyNodeByDescriptor(event->wd);
      if (inode) {
        if (client_) {
          client_->HandleChanges(this, inode->directory, event->name, event->mask & IN_ISDIR, event->mask);
        }
      } else {
        DNOTREACHED();
      }
    }
    i += EVENT_SIZE + event->len;
  }
}

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

  for (size_t i = 0; i < watched_directories_.size(); ++i) {
    inotify_rm_watch(inode, watched_directories_[i].fd);
  }
  watched_directories_.clear();
  return base_class::DoClose();
}

}  // namespace inotify
}  // namespace libev
}  // namespace common
