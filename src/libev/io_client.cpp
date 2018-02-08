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

#include <common/libev/io_client.h>

#include <inttypes.h>

#include <common/sprintf.h>

#include <common/libev/event_io.h>
#include <common/libev/io_loop.h>

namespace common {
namespace libev {

IoClient::IoClient(IoLoop* server, flags_t flags)
    : server_(server), read_write_io_(new LibevIO), flags_(flags), name_(), id_() {
  read_write_io_->SetUserData(this);
}

IoClient::~IoClient() {
  destroy(&read_write_io_);
}

Error IoClient::Close() {
  if (server_) {
    server_->CloseClient(this);
  }
  return DoClose();
}

IoLoop* IoClient::GetServer() const {
  return server_;
}

void IoClient::SetName(const std::string& name) {
  name_ = name;
}

std::string IoClient::GetName() const {
  return name_;
}

flags_t IoClient::GetFlags() const {
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

patterns::id_counter<IoClient>::type_t IoClient::GetId() const {
  return id_.get_id();
}

const char* IoClient::ClassName() const {
  return "IoClient";
}

Error IoClient::Write(const buffer_t& data, size_t* nwrite_out) {
  return Write(data.data(), data.size(), nwrite_out);
}

Error IoClient::Write(const std::string& data, size_t* nwrite_out) {
  return Write(data.data(), data.size(), nwrite_out);
}

Error IoClient::Read(buffer_t* out_data, size_t max_size, size_t* nread_out) {
  return Read(out_data->data(), max_size, nread_out);
}

Error IoClient::Read(std::string* out_data, size_t max_size, size_t* nread_out) {
  char* buff = new char[max_size];
  Error err = Read(buff, max_size, nread_out);
  if (err) {
    delete[] buff;
    return err;
  }

  *out_data = std::string(buff, *nread_out);
  delete[] buff;
  return err;
}

std::string IoClient::GetFormatedName() const {
  return MemSPrintf("[%s][%s(%" PRIuMAX ")]", GetName(), ClassName(), GetId());
}

}  // namespace libev
}  // namespace common
