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

#include <common/libev/io_loop.h>

#include <string>

#include <common/logger.h>
#include <common/threads/types.h>

#include <common/libev/io_client.h>
#include <common/libev/io_loop_observer.h>

namespace {

typedef common::unique_lock<common::mutex> lock_t;
common::mutex g_exists_loops_mutex_;
std::vector<common::libev::IoLoop*> g_exists_loops_;

}  // namespace

namespace common {
namespace libev {

IoLoop::IoLoop(IoLoopObserver* observer) : loop_(new LibEvLoop), observer_(observer), clients_(), id_() {
  loop_->SetObserver(this);
}

IoLoop::~IoLoop() {
  delete loop_;
}

int IoLoop::Exec() {
  int res = loop_->Exec();
  return res;
}

void IoLoop::Stop() {
  loop_->Stop();
}

void IoLoop::RegisterClient(const common::net::socket_info& info) {
  IoClient* client = CreateClient(info);
  RegisterClient(client);
}

void IoLoop::UnRegisterClient(IoClient* client) {
  CHECK(IsLoopThread());
  CHECK(client && client->Server() == this);
  const std::string formated_name = client->FormatedName();

  LibevIO* client_ev = client->read_write_io_;
  client_ev->Stop();
  client->server_ = nullptr;

  if (observer_) {
    observer_->Moved(this, client);
  }

  clients_.erase(std::remove(clients_.begin(), clients_.end(), client), clients_.end());
  INFO_LOG() << "Successfully unregister client[" << formated_name << "], from server[" << FormatedName() << "], "
             << clients_.size() << " client(s) connected.";
}

void IoLoop::RegisterClient(IoClient* client) {
  CHECK(IsLoopThread());
  CHECK(client);
  const std::string formated_name = client->FormatedName();

  if (client->Server()) {
    CHECK(client->Server() == this);
  } else {
    client->server_ = this;
  }

  // Initialize and start watcher to read client requests
  LibevIO* client_ev = client->read_write_io_;
  client_ev->Init(loop_, read_write_cb, client->GetFd(), client->Flags());
  client_ev->Start();

  if (observer_) {
    observer_->Accepted(client);
  }

  clients_.push_back(client);
  INFO_LOG() << "Successfully connected with client[" << formated_name << "], from server[" << FormatedName() << "], "
             << clients_.size() << " client(s) connected.";
}

void IoLoop::CloseClient(IoClient* client) {
  CHECK(IsLoopThread());
  CHECK(client && client->Server() == this);
  const std::string formated_name = client->FormatedName();

  LibevIO* client_ev = client->read_write_io_;
  client_ev->Stop();

  if (observer_) {
    observer_->Closed(client);
  }
  clients_.erase(std::remove(clients_.begin(), clients_.end(), client), clients_.end());
  INFO_LOG() << "Successfully disconnected client[" << formated_name << "], from server[" << FormatedName() << "], "
             << clients_.size() << " client(s) connected.";
}

timer_id_t IoLoop::CreateTimer(double sec, double repeat) {
  return loop_->CreateTimer(sec, repeat);
}

void IoLoop::RemoveTimer(timer_id_t id) {
  loop_->RemoveTimer(id);
}

common::patterns::id_counter<IoLoop>::type_t IoLoop::GetId() const {
  return id_.id();
}

void IoLoop::ExecInLoopThread(custom_loop_exec_function_t func) {
  loop_->ExecInLoopThread(func);
}

bool IoLoop::IsLoopThread() const {
  return loop_->IsLoopThread();
}

IoLoop* IoLoop::FindExistLoopByPredicate(std::function<bool(IoLoop*)> pred) {
  if (!pred) {
    return nullptr;
  }

  lock_t loc(g_exists_loops_mutex_);
  for (size_t i = 0; i < g_exists_loops_.size(); ++i) {
    IoLoop* loop = g_exists_loops_[i];
    if (loop && pred(loop)) {
      return loop;
    }
  }

  return nullptr;
}

std::vector<IoClient*> IoLoop::Clients() const {
  CHECK(IsLoopThread());

  return clients_;
}

void IoLoop::SetName(const std::string& name) {
  name_ = name;
}

std::string IoLoop::Name() const {
  return name_;
}

std::string IoLoop::FormatedName() const {
  return common::MemSPrintf("[%s][%s(%llu)]", Name(), ClassName(), GetId());
}

void IoLoop::read_write_cb(LibEvLoop* loop, LibevIO* io, flags_t revents) {
  IoClient* pclient = reinterpret_cast<IoClient*>(io->UserData());
  IoLoop* pserver = pclient->Server();
  pserver->ReadWrite(loop, pclient, revents);
}

void IoLoop::ReadWrite(LibEvLoop* loop, IoClient* client, flags_t revents) {
  CHECK(IsLoopThread());
  CHECK(loop_ == loop);
  CHECK(client && client->Server() == this);

  if (EV_ERROR & revents) {
    NOTREACHED();
    return;
  }

  if (revents & EV_READ) {
    if (observer_) {
      observer_->DataReceived(client);
    }
  }

  if (revents & EV_WRITE) {
    if (observer_) {
      observer_->DataReadyToWrite(client);
    }
  }
}

void IoLoop::PreLooped(LibEvLoop* loop) {
  UNUSED(loop);
  {
    lock_t loc(g_exists_loops_mutex_);
    g_exists_loops_.push_back(this);
  }

  if (observer_) {
    observer_->PreLooped(this);
  }
}

void IoLoop::Stoped(LibEvLoop* loop) {
  UNUSED(loop);
  CHECK(IsLoopThread());

  const std::vector<IoClient*> cl = Clients();

  for (size_t i = 0; i < cl.size(); ++i) {
    IoClient* client = cl[i];
    client->Close();
    delete client;
  }
}

void IoLoop::PostLooped(LibEvLoop* loop) {
  UNUSED(loop);
  {
    lock_t loc(g_exists_loops_mutex_);
    g_exists_loops_.erase(std::remove(g_exists_loops_.begin(), g_exists_loops_.end(), this), g_exists_loops_.end());
  }

  if (observer_) {
    observer_->PostLooped(this);
  }
}

void IoLoop::TimerEmited(LibEvLoop* loop, timer_id_t id) {
  UNUSED(loop);
  if (observer_) {
    observer_->TimerEmited(this, id);
  }
}

}  // namespace libev
}  // namespace common
