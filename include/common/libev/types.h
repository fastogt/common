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

#include <functional>  // for function

#include <common/macros.h>  // for DISALLOW_COPY_AND_ASSIGN

#include <common/patterns/crtp_pattern.h>  // for id_counter

#define INVALID_TIMER_ID -1

#if defined(OS_POSIX)
#define LIBEV_CHILD_ENABLE 1
#else
#define LIBEV_CHILD_ENABLE 0
#endif

#if LIBEV_CHILD_ENABLE
struct ev_child;
typedef ev_child fasto_ev_child;
#else
struct fasto_ev_child;
#endif

namespace common {
namespace libev {

#if defined(OS_WIN)
typedef void* process_handle_t;
#else
typedef pid_t process_handle_t;
#endif

class LibEvLoop;
class LibevAsync;
class LibevIO;
class LibevTimer;
class LibevChild;

/* eventmask, revents, events... */
enum {
  EV_UNDEF = static_cast<int>(0xFFFFFFFF), /* guaranteed to be invalid */
  EV_NONE = 0x00,                          /* no events */
  EV_READ = 0x01,                          /* ev_io detected read will not block */
  EV_WRITE = 0x02,                         /* ev_io detected write will not block */
  EV__IOFDSET = 0x80,                      /* internal use only */
  EV_IO = EV_READ,                         /* alias for type-detection */
  EV_TIMER = 0x00000100,                   /* timer timed out */
  EV_PERIODIC = 0x00000200,                /* periodic timer timed out */
  EV_SIGNAL = 0x00000400,                  /* signal was received */
  EV_CHILD = 0x00000800,                   /* child/pid had status change */
  EV_STAT = 0x00001000,                    /* stat data changed */
  EV_IDLE = 0x00002000,                    /* event loop is idling */
  EV_PREPARE = 0x00004000,                 /* event loop about to poll */
  EV_CHECK = 0x00008000,                   /* event loop finished poll */
  EV_EMBED = 0x00010000,                   /* embedded event loop needs sweep */
  EV_FORK = 0x00020000,                    /* event loop resumed in child */
  EV_CLEANUP = 0x00040000,                 /* event loop resumed in child */
  EV_ASYNC = 0x00080000,                   /* async intra-loop signal */
  EV_CUSTOM = 0x01000000,                  /* for use by user code */
  EV_ERROR = static_cast<int>(0x80000000)  /* sent when an error occurs */
};

typedef int flags_t;

typedef intmax_t timer_id_t;
typedef uintmax_t io_id_t;
typedef uintmax_t async_id_t;
typedef uintmax_t child_id_t;

template <typename handle_t, typename id_t>
class LibevBase : public patterns::id_counter<LibevBase<handle_t, id_t>, id_t> {
 public:
  typedef void* user_data_t;
  LibevBase() : handle_(static_cast<handle_t*>(calloc(1, sizeof(handle_t)))), user_data_(nullptr) {
    if (handle_) {
      handle_->data = this;
    }
  }
  ~LibevBase() {
    if (handle_) {
      free(handle_);
      handle_ = nullptr;
    }
  }
  handle_t* GetHandle() const { return handle_; }
  void SetUserData(user_data_t user_data) { user_data_ = user_data; }

  user_data_t GetUserData() const { return user_data_; }

 private:
  handle_t* handle_;
  user_data_t user_data_;
  DISALLOW_COPY_AND_ASSIGN(LibevBase);
};

typedef std::function<void()> custom_loop_exec_function_t;

typedef std::function<void(LibEvLoop* loop, LibevAsync* async, flags_t revents)> async_loop_exec_function_t;
typedef std::function<void(LibEvLoop* loop, LibevIO* io, flags_t revents)> io_loop_exec_function_t;
typedef std::function<void(LibEvLoop* loop, LibevTimer* timer, flags_t revents)> timer_loop_exec_function_t;
typedef std::function<void(LibEvLoop* loop, LibevChild* child, int status, int signal, flags_t revents)>
    child_loop_exec_function_t;

}  // namespace libev
}  // namespace common
