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

#pragma once

#include <string>

#include <common/net/socket_tcp.h>
#include <common/patterns/crtp_pattern.h>

#include <common/libev/event_io.h>
#include <common/libev/event_loop.h>

namespace common {
namespace libev {

class IoLoop;

class IoClient : common::IMetaClassInfo {
 public:
  friend class IoLoop;
  IoClient(IoLoop* server, flags_t flags = EV_READ);
  virtual ~IoClient();

  void Close();

  IoLoop* Server() const;

  void SetName(const std::string& name);
  std::string Name() const;

  flags_t Flags() const;
  void SetFlags(flags_t flags);

  common::patterns::id_counter<IoClient>::type_t Id() const;
  virtual const char* ClassName() const override;
  std::string FormatedName() const;

  virtual common::Error Write(const char* data, size_t size, size_t* nwrite) WARN_UNUSED_RESULT = 0;
  virtual common::Error Read(char* out, size_t max_size, size_t* nread) WARN_UNUSED_RESULT = 0;

 protected:  // executed IoLoop
  virtual descriptor_t GetFd() const = 0;
  virtual void CloseImpl() = 0;

 private:
  IoLoop* server_;
  LibevIO* read_write_io_;
  flags_t flags_;

  std::string name_;
  const common::patterns::id_counter<IoClient> id_;
};

}  // namespace libev
}  // namespace common
