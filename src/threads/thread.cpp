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

#include <common/threads/thread.h>

#include <common/threads/thread_manager.h>

namespace common {
namespace threads {

template <typename RT>
void WrapThread(Thread<RT>* thread) {
  THREAD_MANAGER()->WrapThread(thread);
  CHECK(IsCurrentThread(thread));
}

template <typename RT>
void UnWrapThread(Thread<RT>* thread) {
  THREAD_MANAGER()->UnWrapThread(thread);
  CHECK(IsCurrentThread(thread));
}

template <typename RT>
bool IsCurrentThread(Thread<RT>* thread) {
  return THREAD_MANAGER()->IsCurrentThread(thread);
}

PlatformThreadHandle invalid_thread_handle() {
  return PlatformThreadHandle(invalid_handle, invalid_tid);
}

PlatformThreadHandle current_thread_handle() {
  return PlatformThreadHandle(PlatformThread::GetCurrentHandle(), PlatformThread::GetCurrentId());
}

template void WrapThread(Thread<void>* thread);
template void UnWrapThread(Thread<void>* thread);
template bool IsCurrentThread(Thread<void>* thread);

template void WrapThread(Thread<int>* thread);
template void UnWrapThread(Thread<int>* thread);
template bool IsCurrentThread(Thread<int>* thread);

}  // namespace threads
}  // namespace common
