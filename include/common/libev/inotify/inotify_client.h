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

#if defined(OS_LINUX)

#include <vector>

#include <common/file_system/path.h>
#include <common/libev/descriptor_client.h>
#include <common/libev/inotify/types.h>

namespace common {
namespace libev {
namespace inotify {

struct InotifyNode {
  descriptor_t fd;
  file_system::ascii_directory_string_path directory;
};

class IoInotifyClientObserver;

class IoInotifyClient : public DescriptorClient {
 public:
  typedef DescriptorClient base_class;

  explicit IoInotifyClient(IoLoop* server, IoInotifyClientObserver* client, flags_t flags = EV_READ);
  ~IoInotifyClient() override;

  common::ErrnoError WatchDirectory(const file_system::ascii_directory_string_path& direcotry,
                                    uint32_t mask) WARN_UNUSED_RESULT;

  void ProcessRead();

 private:
  Optional<InotifyNode> FindInotifyNodeByDescriptor(descriptor_t fd) const;

  using base_class::Read;
  using base_class::SingleRead;
  using base_class::SingleWrite;
  using base_class::Write;

  ErrnoError DoClose() override;

  IoInotifyClientObserver* client_;
  std::vector<InotifyNode> watched_directories_;
  DISALLOW_COPY_AND_ASSIGN(IoInotifyClient);
};

}  // namespace inotify
}  // namespace libev
}  // namespace common

#endif
