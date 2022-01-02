/*  Copyright (C) 2014-2022 FastoGT. All right reserved.

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

#if defined(HAVE_PTHREAD)
#include <pthread.h>
#include <sys/types.h>
#elif defined(OS_WIN)
#include <windows.h>
#endif

#include <common/macros.h>
#include <common/system_info/types.h>
#include <common/types.h>

namespace common {
namespace threads {

typedef void (*closure_type)();
typedef void*(routine_signature)(void*);

#if defined(HAVE_PTHREAD)
typedef pid_t platform_thread_id_t;
typedef pthread_key_t platform_tls_t;
typedef pthread_t platform_handle_t;
#else
typedef DWORD platform_thread_id_t;
typedef DWORD platform_tls_t;
typedef HANDLE platform_handle_t;
#endif

extern const platform_handle_t invalid_handle;
extern const platform_thread_id_t invalid_tid;

// Used to operate on threads.
class PlatformThreadHandle {
 public:
  PlatformThreadHandle() : handle_(invalid_handle), thread_id_(invalid_tid) {}

  PlatformThreadHandle(platform_handle_t handle, platform_thread_id_t id) : handle_(handle), thread_id_(id) {}

  bool EqualsHandle(const PlatformThreadHandle& other) const;

  platform_thread_id_t GetTid() const { return thread_id_; }
  platform_handle_t GetPlatformHandle() const { return handle_; }

  bool Equals(const PlatformThreadHandle& handle) const {
    return EqualsHandle(handle) && thread_id_ == handle.thread_id_;
  }

 private:
  friend class PlatformThread;

  platform_handle_t handle_;
  platform_thread_id_t thread_id_;
};

PlatformThreadHandle invalid_thread_handle();
PlatformThreadHandle current_thread_handle();

inline bool operator==(const PlatformThreadHandle& left, const PlatformThreadHandle& right) {
  return left.Equals(right);
}

inline bool operator!=(const PlatformThreadHandle& left, const PlatformThreadHandle& right) {
  return !(left == right);
}

enum ThreadPriority {
  PRIORITY_IDLE = -1,
  PRIORITY_NORMAL = 0,
  PRIORITY_ABOVE_NORMAL = 1,
  PRIORITY_HIGH = 2,
};

class PlatformThread {
 public:
  static bool Create(PlatformThreadHandle* thread_handle,
                     routine_signature routine,
                     void* arg,
                     ThreadPriority priority);
  static bool Join(PlatformThreadHandle* thread_handle, void** thread_return);
  static void SetAffinity(PlatformThreadHandle* thread_handle, lcpu_count_t lCpuCount);

  static platform_handle_t GetCurrentHandle();
  static platform_thread_id_t GetCurrentId();

  static bool InitTlsKey(platform_tls_t* key);
  static bool ReleaseTlsKey(platform_tls_t key);
  static void* GetTlsDataByKey(platform_tls_t key);
  static bool SetTlsDataByKey(platform_tls_t key, void* data);

  static void Sleep(time64_t milliseconds);
};

}  // namespace threads
}  // namespace common
