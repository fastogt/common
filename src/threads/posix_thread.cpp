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

#include <common/threads/platform_thread.h>

#include <sched.h>  // for sched_get_priority_max, etc

#if defined(OS_LINUX) || defined(OS_ANDROID)
#if defined(OS_ANDROID)
#include <sys/syscall.h>
#else
#include <syscall.h>  // for __NR_gettid
#endif
#include <unistd.h>  // for syscall
#elif defined(OS_FREEBSD)
#include <pthread_np.h>
typedef cpuset_t cpu_set_t;
#elif defined(OS_MACOSX)
#include <mach/mach.h>
#include <mach/thread_policy.h>
#include <sys/sysctl.h>

#define SYSCTL_CORE_COUNT "machdep.cpu.core_count"

typedef struct cpu_set {
  uint32_t count;
} cpu_set_t;

#define CPU_ZERO(cpusetp) (cpusetp)->count = 0
#define CPU_SET(num, cpusetp) (cpusetp)->count |= (1 << num)
#define CPU_ISSET(num, cpusetp) (cpusetp)->count&(1 << num)

int pthread_setaffinity_np(pthread_t thread, size_t cpu_size, cpu_set_t* cpu_set) {
  thread_port_t mach_thread;
  size_t core = 0;

  for (core = 0; core < 8 * cpu_size; core++) {
    if (CPU_ISSET(core, cpu_set)) {
      break;
    }
  }

  thread_affinity_policy_data_t policy = {static_cast<integer_t>(core)};
  mach_thread = pthread_mach_thread_np(thread);
  thread_policy_set(mach_thread, THREAD_AFFINITY_POLICY, (thread_policy_t)&policy, 1);
  return 0;
}

int sched_getaffinity(pid_t pid, size_t cpu_size, cpu_set_t* cpu_set) {
  UNUSED(pid);
  UNUSED(cpu_size);

  int32_t core_count = 0;
  size_t len = sizeof(core_count);
  int ret = sysctlbyname(SYSCTL_CORE_COUNT, &core_count, &len, 0, 0);
  if (ret) {
    return -1;
  }
  cpu_set->count = 0;
  for (int32_t i = 0; i < core_count; i++) {
    cpu_set->count |= (1 << i);
  }

  return 0;
}
#endif

#include <common/macros.h>             // for DCHECK, DNOTREACHED, UNUSED, etc
#include <common/system_info/types.h>  // for lcpu_count_t
#include <common/time.h>

namespace common {
namespace threads {
namespace {

struct ThreadParams {
  void* args;
  routine_signature* cl;
  platform_thread_id_t* tid_;
};

void* threadFunc(void* params) {
  ThreadParams* thread_params = static_cast<ThreadParams*>(params);

  platform_thread_id_t tid = PlatformThread::GetCurrentId();
  *(thread_params->tid_) = tid;
  void* result = thread_params->cl(thread_params->args);
  delete thread_params;
#if defined(OS_WIN)
  return result;
#else
  pthread_exit(result);
  return nullptr;
#endif
}

}  // namespace

const platform_handle_t invalid_handle = PTHREAD_INVALID_TID;
const platform_thread_id_t invalid_tid = 0;

bool PlatformThreadHandle::EqualsHandle(const PlatformThreadHandle& other) const {
  return pthread_equal(handle_, other.handle_) != 0;
}

platform_handle_t PlatformThread::GetCurrentHandle() {
  return pthread_self();
}

// static
platform_thread_id_t PlatformThread::GetCurrentId() {
// Pthreads doesn't have the concept of a thread ID, so we have to reach down
// into the kernel.
#if defined(OS_MACOSX)
  return pthread_mach_thread_np(pthread_self());
#elif defined(OS_WIN)
  return reinterpret_cast<uintptr_t>(pthread_self());
#elif defined(OS_LINUX)
  return syscall(__NR_gettid);
#elif defined(OS_FREEBSD)
  return pthread_getthreadid_np();
#elif defined(OS_ANDROID)
  return gettid();
#elif defined(OS_SOLARIS) || defined(OS_QNX)
  return pthread_self();
#elif defined(OS_NACL) && defined(__GLIBC__)
  return pthread_self();
#elif defined(OS_NACL) && !defined(__GLIBC__)
  // Pointers are 32-bits in NaCl.
  return reinterpret_cast<int32>(pthread_self());
#elif defined(OS_POSIX)
  return reinterpret_cast<int64>(pthread_self());
#endif
}

bool PlatformThread::Create(PlatformThreadHandle* thread_handle,
                            routine_signature routine,
                            void* arg,
                            ThreadPriority priority) {
  if (!thread_handle) {
    return false;
  }

  ThreadParams* params = new ThreadParams;
  params->cl = routine;
  params->args = arg;
  params->tid_ = &thread_handle->thread_id_;

  pthread_attr_t custom_sched_attr;
  pthread_attr_t* pcustom_sched_attr = nullptr;

  if (priority != PRIORITY_NORMAL) {
    pcustom_sched_attr = &custom_sched_attr;
    int policy = 0;
    struct sched_param param;

    pthread_attr_init(&custom_sched_attr);
    pthread_attr_getschedpolicy(&custom_sched_attr, &policy);

    if (priority == PRIORITY_HIGH) {
      param.sched_priority = sched_get_priority_max(policy);
    } else if (priority == PRIORITY_IDLE) {
      param.sched_priority = sched_get_priority_min(policy);
    } else {
      int pmax = sched_get_priority_max(policy);
      int pmin = sched_get_priority_min(policy);
      param.sched_priority = (pmax - pmin) / 2;
    }

    pthread_attr_setschedparam(&custom_sched_attr, &param);
  }

  int res = pthread_create(&thread_handle->handle_, pcustom_sched_attr, &threadFunc, params);
  if (pcustom_sched_attr) {
    pthread_attr_destroy(pcustom_sched_attr);
  }

  bool result = res == 0;
  DCHECK(result);
  return result;
}

bool PlatformThread::Join(PlatformThreadHandle* thread_handle, void** thread_return) {
  if (!thread_handle) {
    return false;
  }

  if (thread_handle->GetPlatformHandle() == invalid_handle) {
    thread_handle->thread_id_ = invalid_tid;
    return false;
  }

  int res = pthread_join(thread_handle->handle_, thread_return);
  DCHECK_EQ(res, 0);

  thread_handle->handle_ = invalid_handle;
  thread_handle->thread_id_ = invalid_tid;
  return true;
}

void PlatformThread::SetAffinity(PlatformThreadHandle* thread_handle, lcpu_count_t lCpuCount) {
  UNUSED(lCpuCount);

  if (!thread_handle || thread_handle->GetPlatformHandle() == invalid_handle) {
    return;
  }

#if defined(HAVE_PTHREAD) && !defined(OS_WIN) && !defined(OS_ANDROID)
  cpu_set_t cpu_set;
  CPU_ZERO(&cpu_set);
  CPU_SET(lCpuCount, &cpu_set);
  int res = pthread_setaffinity_np(thread_handle->handle_, sizeof(cpu_set_t), &cpu_set);
  DCHECK_EQ(res, 0);
#endif
}

bool PlatformThread::InitTlsKey(platform_tls_t* key) {
  if (!key) {
    DNOTREACHED();
    return false;
  }

  int res = pthread_key_create(key, nullptr);
  bool result = res == 0;
  DCHECK(result);
  return result;
}

bool PlatformThread::ReleaseTlsKey(platform_tls_t key) {
  int res = pthread_key_delete(key);
  bool result = res == 0;
  DCHECK(result);
  return result;
}

void* PlatformThread::GetTlsDataByKey(platform_tls_t key) {
  return pthread_getspecific(key);
}

bool PlatformThread::SetTlsDataByKey(platform_tls_t key, void* data) {
  int res = pthread_setspecific(key, data);
  bool result = res == 0;
  DCHECK(result);
  return result;
}

void PlatformThread::Sleep(time64_t milliseconds) {
  struct timespec sleep_time = time::mstime2timespec(milliseconds), remaining;
  while (::nanosleep(&sleep_time, &remaining) == -1 && errno == EINTR) {
    sleep_time = remaining;
  }
}

}  // namespace threads
}  // namespace common
