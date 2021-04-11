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

#include <common/libev/io_loop.h>

#include <algorithm>
#include <mutex>

#include <common/sprintf.h>

#include <common/libev/event_io.h>
#include <common/libev/io_client.h>
#include <common/libev/io_loop_observer.h>

#include <common/libev/event_child.h>
#include <common/libev/io_child.h>

namespace {

typedef std::unique_lock<std::mutex> lock_t;
std::mutex g_exists_loops_mutex;
std::vector<common::libev::IoLoop*> g_exists_loops;

}  // namespace

namespace common {
namespace libev {

IoLoop::IoLoop(LibEvLoop* loop, IoLoopObserver* observer) : loop_(loop), observer_(observer), clients_(), id_() {
  loop_->SetObserver(this);
}

IoLoop::~IoLoop() {
  delete loop_;
}

bool IoLoop::IsRunning() const {
  return loop_->IsRunning();
}

int IoLoop::Exec() {
  int res = loop_->Exec();
  return res;
}

void IoLoop::Stop() {
  loop_->Stop();
}

void IoLoop::UnRegisterClient(IoClient* client) {
  if (!client) {
    DNOTREACHED();
    return;
  }

  CHECK(IsLoopThread()) << "Must be called in loop thread!";
  CHECK(client->GetServer() == this) << "Must have same server!";
  const std::string formated_name = client->GetFormatedName();

  LibevIO* client_ev = client->read_write_io_;
  client_ev->Stop();
  client->server_ = nullptr;

  if (observer_) {
    observer_->Moved(this, client);
  }

  clients_.erase(std::remove(clients_.begin(), clients_.end(), client), clients_.end());
  INFO_LOG() << "Successfully unregister client[" << formated_name << "], from server[" << GetFormatedName() << "], "
             << clients_.size() << " client(s) connected.";
}

bool IoLoop::RegisterClient(IoClient* client) {
  if (!client) {
    DNOTREACHED();
    return false;
  }

  CHECK(IsLoopThread()) << "Must be called in loop thread!";
  if (!IsCanBeRegistered(client)) {
    return false;
  }
  const std::string formated_name = client->GetFormatedName();

  if (client->GetServer()) {
    CHECK(client->GetServer() == this) << "Must have same server!";
  } else {
    client->server_ = this;
  }

  // Initialize and start watcher to read client requests
  LibevIO* client_ev = client->read_write_io_;
  bool is_inited = client_ev->Init(loop_, read_write_cb, client->GetFd(), client->GetFlags());
  if (!is_inited) {
    DNOTREACHED();
    return false;
  }
  client_ev->Start();

  if (observer_) {
    observer_->Accepted(client);
  }

  clients_.push_back(client);
  INFO_LOG() << "Successfully connected with client[" << formated_name << "], from server[" << GetFormatedName()
             << "], " << clients_.size() << " client(s) connected.";
  return true;
}

void IoLoop::CloseClient(IoClient* client) {
  if (!client) {
    DNOTREACHED();
    return;
  }

  CHECK(IsLoopThread()) << "Must be called in loop thread!";
  CHECK(client->GetServer() == this) << "Must have same server!";
  const std::string formated_name = client->GetFormatedName();

  LibevIO* client_ev = client->read_write_io_;
  client_ev->Stop();

  if (observer_) {
    observer_->Closed(client);
  }
  clients_.erase(std::remove(clients_.begin(), clients_.end(), client), clients_.end());
  INFO_LOG() << "Successfully disconnected client[" << formated_name << "], from server[" << GetFormatedName() << "], "
             << clients_.size() << " client(s) connected.";
}

timer_id_t IoLoop::CreateTimer(double sec, bool repeat) {
  return loop_->CreateTimer(sec, repeat);
}

void IoLoop::RemoveTimer(timer_id_t id) {
  loop_->RemoveTimer(id);
}

IoChild* IoLoop::RegisterChild(process_handle_t pid) {
  IoChild* client = CreateChild();
  RegisterChild(client, pid);
  return client;
}

void IoLoop::RegisterChild(IoChild* child, process_handle_t pid) {
  CHECK(IsLoopThread()) << "Must be called in loop thread!";
  CHECK(child);
  const std::string formated_name = child->GetFormatedName();

  if (child->GetServer()) {
    CHECK(child->GetServer() == this);
  } else {
    child->server_ = this;
  }

  // Initialize and start watcher to read client requests
  LibevChild* child_ev = child->child_;
  bool is_inited = child_ev->Init(loop_, child_cb, pid);
  if (!is_inited) {
    DNOTREACHED();
    return;
  }
  child_ev->Start();

  if (observer_) {
    observer_->Accepted(child);
  }
  childs_.push_back(child);
  INFO_LOG() << "Successfully connected with client[" << formated_name << "], from server[" << GetFormatedName()
             << "], " << childs_.size() << " childs(s) connected.";
}

void IoLoop::UnRegisterChild(IoChild* child) {
  CHECK(IsLoopThread()) << "Must be called in loop thread!";
  if (!child) {
    NOTREACHED();
    return;
  }

  CHECK(child->GetServer() == this);
  const std::string formated_name = child->GetFormatedName();

  LibevChild* child_ev = child->child_;
  child_ev->Stop();
  child->server_ = nullptr;

  if (observer_) {
    observer_->Moved(this, child);
  }
  childs_.erase(std::remove(childs_.begin(), childs_.end(), child), childs_.end());
  INFO_LOG() << "Successfully unregister client[" << formated_name << "], from server[" << GetFormatedName() << "], "
             << childs_.size() << " client(s) connected.";
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

  lock_t loc(g_exists_loops_mutex);
  for (size_t i = 0; i < g_exists_loops.size(); ++i) {
    IoLoop* loop = g_exists_loops[i];
    if (loop && pred(loop)) {
      return loop;
    }
  }

  return nullptr;
}

std::vector<IoClient*> IoLoop::GetClients() const {
  CHECK(IsLoopThread()) << "Must be called in loop thread!";

  return clients_;
}

std::vector<IoChild*> IoLoop::GetChilds() const {
  CHECK(IsLoopThread()) << "Must be called in loop thread!";

  return childs_;
}

void IoLoop::read_write_cb(LibEvLoop* loop, LibevIO* io, flags_t revents) {
  IoClient* pclient = reinterpret_cast<IoClient*>(io->GetUserData());
  IoLoop* pserver = pclient->GetServer();
  pserver->ReadWrite(loop, pclient, revents);
}

void IoLoop::ReadWrite(LibEvLoop* loop, IoClient* client, flags_t revents) {
  CHECK(IsLoopThread()) << "Must be called in loop thread!";
  CHECK(loop_ == loop);
  CHECK(client && client->GetServer() == this);

  if (EV_ERROR & revents) {
    DNOTREACHED();
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

void IoLoop::child_cb(LibEvLoop* loop, LibevChild* child, int status, int signal, flags_t revents) {
  IoChild* pchild = reinterpret_cast<IoChild*>(child->GetUserData());
  IoLoop* pserver = pchild->GetServer();
  pserver->ChildStatus(loop, pchild, status, signal, revents);
}

void IoLoop::ChildStatus(LibEvLoop* loop, IoChild* child, int status, int signal, flags_t revents) {
  CHECK(IsLoopThread()) << "Must be called in loop thread!";
  CHECK(loop_ == loop);
  CHECK(child && child->GetServer() == this);

  if (EV_ERROR & revents) {
    DNOTREACHED();
    return;
  }

  if (revents & EV_CHILD) {
    if (observer_) {
      observer_->ChildStatusChanged(child, status, signal);
    }
  }
}

void IoLoop::PreLooped(LibEvLoop* loop) {
  UNUSED(loop);
  {
    lock_t loc(g_exists_loops_mutex);
    g_exists_loops.push_back(this);
  }

  if (observer_) {
    observer_->PreLooped(this);
  }
}

void IoLoop::Started(LibEvLoop* loop) {
  UNUSED(loop);
}

void IoLoop::Stopped(LibEvLoop* loop) {
  UNUSED(loop);
  CHECK(IsLoopThread()) << "Must be called in loop thread!";

  const std::vector<IoClient*> cl = GetClients();

  for (size_t i = 0; i < cl.size(); ++i) {
    IoClient* client = cl[i];
    ErrnoError err = client->Close();
    DCHECK(!err) << err->GetDescription();
    delete client;
  }

  const std::vector<IoChild*> childs = GetChilds();
  for (size_t i = 0; i < childs.size(); ++i) {
    IoChild* child = childs[i];
    delete child;
  }
}

void IoLoop::PostLooped(LibEvLoop* loop) {
  UNUSED(loop);
  {
    lock_t loc(g_exists_loops_mutex);
    g_exists_loops.erase(std::remove(g_exists_loops.begin(), g_exists_loops.end(), this), g_exists_loops.end());
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
