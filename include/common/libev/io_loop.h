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

#pragma once

#include <common/libev/event_loop.h>

#include <common/libev/io_base.h>

namespace common {
namespace net {
class socket_info;
}
}  // namespace common

namespace common {
namespace libev {

class IoLoopObserver;
class IoClient;
#if LIBEV_CHILD_ENABLE
class IoChild;
#endif

class IoLoop : public EvLoopObserver, public IoBase<IoLoop> {
 public:
  explicit IoLoop(LibEvLoop* loop, IoLoopObserver* observer = nullptr);
  virtual ~IoLoop();

  bool IsRunning() const;
  int Exec() WARN_UNUSED_RESULT;
  virtual void Stop();

  IoClient* RegisterClient(const net::socket_info& info);
  void RegisterClient(IoClient* client);
  void UnRegisterClient(IoClient* client);
  virtual void CloseClient(IoClient* client);

  timer_id_t CreateTimer(double sec, bool repeat);
  void RemoveTimer(timer_id_t id);

#if LIBEV_CHILD_ENABLE
  IoChild* RegisterChild(pid_t pid);
  void RegisterChild(IoChild* child, pid_t pid);
  void UnRegisterChild(IoChild* child);
#endif

  virtual const char* ClassName() const override = 0;

  void ExecInLoopThread(custom_loop_exec_function_t func);

  bool IsLoopThread() const;

  std::vector<IoClient*> GetClients() const;
#if LIBEV_CHILD_ENABLE
  std::vector<IoChild*> GetChilds() const;
#endif

  static IoLoop* FindExistLoopByPredicate(std::function<bool(IoLoop*)> pred);

 protected:
  virtual IoClient* CreateClient(const net::socket_info& info) = 0;
#if LIBEV_CHILD_ENABLE
  virtual IoChild* CreateChild() = 0;
#endif

  virtual void PreLooped(LibEvLoop* loop) override;
  virtual void Started(LibEvLoop* loop) override;
  virtual void Stopped(LibEvLoop* loop) override;
  virtual void PostLooped(LibEvLoop* loop) override;
  virtual void TimerEmited(LibEvLoop* loop, timer_id_t id) override;

  LibEvLoop* const loop_;

 private:
  static void read_write_cb(LibEvLoop* loop, LibevIO* io, flags_t revents);
  void ReadWrite(LibEvLoop* loop, IoClient* client, flags_t revents);

#if LIBEV_CHILD_ENABLE
  static void child_cb(LibEvLoop* loop, LibevChild* child, int status, flags_t revents);
  void ChildStatus(LibEvLoop* loop, IoChild* child, int status, flags_t revents);
#endif

  IoLoopObserver* const observer_;

  std::vector<IoClient*> clients_;
#if LIBEV_CHILD_ENABLE
  std::vector<IoChild*> childs_;
#endif
  const patterns::id_counter<IoLoop> id_;

  std::string name_;
};

}  // namespace libev
}  // namespace common
