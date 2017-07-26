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

#include <common/threads/thread_manager.h>

#include <sched.h>  // for __CPU_SETSIZE, __NCPUBITS

#include <common/macros.h>                // for WORDSIZE, SIZEOFMASS, etc
#include <common/system_info/cpu_info.h>  // for CpuInfo, CurrentCpuInfo

#define THREADS_PER_CORE_DEFAULT 4

#if defined(HAVE_PTHREAD) && !defined(OS_WIN) && !defined(OS_MACOSX) && !defined(OS_FREEBSD) && !defined(OS_ANDROID)
#define MAX_LCPU_SIZE (__CPU_SETSIZE / __NCPUBITS) * THREADS_PER_CORE_DEFAULT
#else
#define MAX_LCPU_SIZE 16 * THREADS_PER_CORE_DEFAULT
#endif

#define COUNT_ADDRESSES 4

namespace {
struct CoreCache {
  uintptr_t addr[COUNT_ADDRESSES];
};

CoreCache max_lcpu[MAX_LCPU_SIZE];
}  // namespace

namespace common {
namespace threads {

lcpu_count_t ThreadManager::AllocLCpuNumber(uintptr_t fc) const {
  const lcpu_count_t lc = LogicalCpusCount();
  const lcpu_count_t toncore = ThreadsOnCore();
  if (SIZEOFMASS(max_lcpu) < lc) {
    DNOTREACHED();
    return 0;
  }

  for (auto i = 0; i < COUNT_ADDRESSES; ++i) {
    for (lcpu_count_t j = 0; j < lc; j += toncore) {
      uintptr_t cur = max_lcpu[j].addr[i];
      if (!cur) {
        max_lcpu[j].addr[i] = fc;
        return j;
      }

      if (cur == fc) {
        return j;
      } else {
        if ((cur - WORDSIZE * WORDSIZE) < fc && fc < (cur + WORDSIZE * WORDSIZE)) {
          return j;
        }
      }
    }
  }

  return 0;
}

void ThreadManager::ReleaseLCpuNumber(lcpu_count_t index, uintptr_t fc) {
  if (index >= SIZEOFMASS(max_lcpu)) {
    DNOTREACHED();
    return;
  }

  for (auto i = 0; i < COUNT_ADDRESSES; ++i) {
    if (max_lcpu[index].addr[i] == fc) {
      max_lcpu[index].addr[i] = 0;
      return;
    }
  }
}

lcpu_count_t ThreadManager::LogicalCpusCount() const {
  return info_.LogicalCpusCount();
}

lcpu_count_t ThreadManager::ThreadsOnCore() const {
  return info_.ThreadsOnCore();
}

ThreadManager::ThreadManager() : info_(system_info::CurrentCpuInfo()), key_(0), main_thread_(nullptr) {
  InitProcessPolicy(LogicalCpusCount());
  PlatformThread::InitTlsKey(&key_);

  main_thread_ = new Thread<int>;
  WrapThread(main_thread_);
}

ThreadManager::~ThreadManager() {
  UnWrapThread(main_thread_);
  delete main_thread_;
  PlatformThread::ReleaseTlsKey(key_);
  FreeProcessPolicy(LogicalCpusCount());
}

}  // namespace threads
}  // namespace common
