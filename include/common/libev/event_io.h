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

#include <common/libev/types.h>

struct ev_io;
struct ev_loop;

namespace common {
namespace libev {

class LibevIO : public LibevBase<struct ev_io, io_id_t> {
 public:
  typedef LibevBase<struct ev_io, io_id_t> base_class;
  LibevIO();
  ~LibevIO();

  descriptor_t GetFd() const;

  bool Init(LibEvLoop* loop, io_loop_exec_function_t cb, descriptor_t fd, flags_t events) WARN_UNUSED_RESULT;
  void Start();
  void Stop();

  flags_t GetEvents() const;
  void SetEvents(int events);

 private:
  DISALLOW_COPY_AND_ASSIGN(LibevIO);
  static void io_callback(struct ev_loop* loop, struct ev_io* watcher, int revents);

  LibEvLoop* loop_;
  io_loop_exec_function_t func_;
  descriptor_t fd_;
  flags_t events_;
};

}  // namespace libev
}  // namespace common
