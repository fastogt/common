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

#include <common/libev/io_client.h>

#include <inttypes.h>

#include <string>

#include <common/logger.h>

#include <common/libev/tcp/tcp_server.h>

namespace common {
namespace libev {

IoClient::IoClient(IoLoop* server, flags_t flags)
    : server_(server), read_write_io_(new LibevIO), flags_(flags), name_(), id_() {
  read_write_io_->SetUserData(this);
}

IoClient::~IoClient() {
  destroy(&read_write_io_);
}

void IoClient::Close() {
  if (server_) {
    server_->CloseClient(this);
  }
  CloseImpl();
}

IoLoop* IoClient::Server() const {
  return server_;
}

void IoClient::SetName(const std::string& name) {
  name_ = name;
}

std::string IoClient::Name() const {
  return name_;
}

flags_t IoClient::Flags() const {
  return flags_;
}

void IoClient::SetFlags(flags_t flags) {
  if (flags_ != flags) {
    flags_ = flags;
    read_write_io_->Stop();
    read_write_io_->SetEvents(flags);
    read_write_io_->Start();
  }
}

common::patterns::id_counter<IoClient>::type_t IoClient::Id() const {
  return id_.id();
}

const char* IoClient::ClassName() const {
  return "IoClient";
}

std::string IoClient::FormatedName() const {
  return common::MemSPrintf("[%s][%s(%" PRIuMAX ")]", Name(), ClassName(), Id());
}

}  // namespace libev
}  // namespace common
