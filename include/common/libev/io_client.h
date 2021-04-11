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

#include <common/error.h>
#include <common/libev/io_base.h>
#include <common/libev/types.h>

namespace common {
namespace libev {

class IoLoop;

class IoClient : public IoBase<IoClient> {
 public:
  friend class IoLoop;
  typedef IoBase<IoClient> base_class;

  explicit IoClient(IoLoop* server, flags_t flags = EV_READ);
  ~IoClient() override;

  ErrnoError Close() WARN_UNUSED_RESULT;

  IoLoop* GetServer() const;

  flags_t GetFlags() const;

  size_t GetWroteBytes() const;
  size_t GetReadBytes() const;

  const char* ClassName() const override;

  ErrnoError Write(const void* data, size_t size, size_t* nwrite_out) WARN_UNUSED_RESULT;
  ErrnoError Read(void* out_data, size_t max_size, size_t* nread_out) WARN_UNUSED_RESULT;

  ErrnoError SingleWrite(const void* data, size_t size, size_t* nwrite_out) WARN_UNUSED_RESULT;
  ErrnoError SingleRead(void* out_data, size_t max_size, size_t* nread_out) WARN_UNUSED_RESULT;

 protected:  // executed IoLoop
  virtual descriptor_t GetFd() const = 0;

 private:
  virtual ErrnoError DoSingleWrite(const void* data, size_t size, size_t* nwrite_out) WARN_UNUSED_RESULT = 0;
  virtual ErrnoError DoSingleRead(void* out_data, size_t max_size, size_t* nread_out) WARN_UNUSED_RESULT = 0;
  virtual ErrnoError DoClose() WARN_UNUSED_RESULT = 0;

  IoLoop* server_;
  LibevIO* read_write_io_;
  flags_t flags_;
  size_t wrote_bytes_;
  size_t read_bytes_;
  DISALLOW_COPY_AND_ASSIGN(IoClient);
};

}  // namespace libev
}  // namespace common
