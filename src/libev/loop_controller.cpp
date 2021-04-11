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

#include <common/libev/loop_controller.h>

#include <stdlib.h>

#include <common/libev/io_loop.h>  // for IoLoop
#include <common/libev/io_loop_observer.h>

namespace common {
namespace libev {

ILoopController::ILoopController() : loop_(nullptr), handler_(nullptr) {}

int ILoopController::Exec() {
  CHECK(!handler_);
  CHECK(!loop_);

  handler_ = CreateHandler();
  if (!handler_) {
    return EXIT_FAILURE;
  }

  loop_ = CreateServer(handler_);
  if (!loop_) {
    destroy(&handler_);
    return EXIT_FAILURE;
  }

  return loop_->Exec();
}

void ILoopController::Start() {
  HandleStarted();
}

void ILoopController::Stop() {
  if (loop_) {
    loop_->Stop();
  }

  HandleStopped();
}

void ILoopController::ExecInLoopThread(custom_loop_exec_function_t func) const {
  if (loop_) {
    loop_->ExecInLoopThread(func);
  }
}

ILoopController::~ILoopController() {
  destroy(&loop_);
  destroy(&handler_);
}

}  // namespace libev
}  // namespace common
