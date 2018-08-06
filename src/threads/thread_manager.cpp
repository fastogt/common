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

#include <common/threads/thread_manager.h>

#include <common/system_info/cpu_info.h>  // for CpuInfo, CurrentCpuInfo

namespace common {
namespace threads {

lcpu_count_t ThreadManager::LogicalCpusCount() const {
  return info_.GetLogicalCpusCount();
}

lcpu_count_t ThreadManager::ThreadsOnCore() const {
  return info_.GetThreadsOnCore();
}

ThreadManager::ThreadManager() : info_(system_info::CurrentCpuInfo()), key_(0), main_thread_(new Thread<int>) {
  InitProcessPolicy(LogicalCpusCount());
  PlatformThread::InitTlsKey(&key_);

  main_thread_->handle_ = PlatformThreadHandle(PlatformThread::GetCurrentHandle(), PlatformThread::GetCurrentId());
  main_thread_->event_.Set();
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
