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

#include <common/threads/platform_thread.h>

#include <common/time.h>

namespace common {
namespace threads {
namespace {

struct ThreadParams {
  void* args;
  routine_signature* cl;
};

DWORD WINAPI ThreadFunc(LPVOID params) {
  ThreadParams* thread_params = static_cast<ThreadParams*>(params);
  void* result = thread_params->cl(thread_params->args);
  UNUSED(result);
  delete thread_params;
  return 0;
}

}  // namespace

const platform_handle_t invalid_handle = NULL;
const platform_thread_id_t invalid_tid = 0;

void InitProcessPolicy(lcpu_count_t lcpuCount) {
  DCHECK(lcpuCount);
  if (!lcpuCount) {
    return;
  }

  HANDLE process = GetCurrentProcess();
  DWORD_PTR processAffinityMask;
  DWORD_PTR systemAffinityMask;

  if (!GetProcessAffinityMask(process, &processAffinityMask, &systemAffinityMask)) {
    return;
  }

  DWORD_PTR mask = (0x1 << lcpuCount) - 1;

  BOOL res = SetProcessAffinityMask(process, mask);
  DCHECK(res);
}

void FreeProcessPolicy(lcpu_count_t lCpuCount) {
  UNUSED(lCpuCount);
}

bool PlatformThreadHandle::EqualsHandle(const PlatformThreadHandle& other) const {
  return handle_ == other.handle_;
}

// static
platform_thread_id_t PlatformThread::GetCurrentId() {
  return GetCurrentThreadId();
}

// static
platform_handle_t GetCurrentHandle() {
  return OpenThread(SYNCHRONIZE, FALSE, GetCurrentThreadId());
}

bool PlatformThread::Create(PlatformThreadHandle* thread_handle,
                            routine_signature routine,
                            void* arg,
                            ThreadPriority priority) {
  DCHECK(thread_handle);
  if (!thread_handle) {
    return false;
  }

  ThreadParams* params = new ThreadParams;
  params->cl = routine;
  params->args = arg;

  thread_handle->handle_ =
      CreateThread(NULL, 0, static_cast<LPTHREAD_START_ROUTINE>(&ThreadFunc), params, 0, &thread_handle->thread_id_);
  bool result = thread_handle->handle_ != invalid_handle;
  DCHECK(result);
  if (result) {
    if (priority == PRIORITY_NORMAL) {
      SetThreadPriority(thread_handle->handle_, THREAD_PRIORITY_NORMAL);
    } else if (priority == PRIORITY_HIGH) {
      SetThreadPriority(thread_handle->handle_, THREAD_PRIORITY_HIGHEST);
    } else if (priority == PRIORITY_ABOVE_NORMAL) {
      SetThreadPriority(thread_handle->handle_, THREAD_PRIORITY_ABOVE_NORMAL);
    } else if (priority == PRIORITY_IDLE) {
      SetThreadPriority(thread_handle->handle_, THREAD_PRIORITY_IDLE);
    }
  }
  return result;
}

bool PlatformThread::Join(PlatformThreadHandle* thread_handle, void** thread_return) {
  UNUSED(thread_return);
  if (!thread_handle) {
    DNOTREACHED();
    return false;
  }

  if (thread_handle->handle_ == invalid_handle) {
    thread_handle->thread_id_ = invalid_tid;
    return false;
  }

  DWORD result = WaitForSingleObject(thread_handle->handle_, INFINITE);
  if (result != WAIT_OBJECT_0) {
    thread_handle->handle_ = invalid_handle;
    thread_handle->thread_id_ = invalid_tid;
    return false;
  }
  CloseHandle(thread_handle->handle_);
  thread_handle->handle_ = invalid_handle;
  thread_handle->thread_id_ = invalid_tid;
  return true;
}

void PlatformThread::SetAffinity(PlatformThreadHandle* thread_handle, lcpu_count_t lcpuCount) {
  DCHECK(thread_handle);
  if (!thread_handle) {
    return;
  }

  DWORD_PTR mask = 0x1 << lcpuCount;

  DWORD_PTR res = SetThreadAffinityMask(thread_handle->handle_, mask);
  DCHECK(res);
}

bool PlatformThread::InitTlsKey(platform_tls_t* key) {
  DCHECK(key);
  if (!key) {
    return false;
  }

  (*key) = TlsAlloc();
  return true;
}

bool PlatformThread::ReleaseTlsKey(platform_tls_t key) {
  TlsFree(key);
  return true;
}

void* PlatformThread::GetTlsDataByKey(platform_tls_t key) {
  return TlsGetValue(key);
}

bool PlatformThread::SetTlsDataByKey(platform_tls_t key, void* data) {
  return TlsSetValue(key, data) == 1;
}

void PlatformThread::Sleep(time64_t milliseconds) {
  // When measured with a high resolution clock, Sleep() sometimes returns much
  // too early. We may need to call it repeatedly to get the desired duration.
  time64_t end = current_mstime() + milliseconds;
  for (time64_t now = current_mstime(); now < end; now = current_mstime()) {
    ::Sleep(static_cast<DWORD>(end - now));
  }
}

}  // namespace threads
}  // namespace common
