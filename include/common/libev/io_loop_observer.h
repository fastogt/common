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

#include <common/libev/types.h>

namespace common {
namespace libev {

class IoLoop;
class IoClient;
class IoChild;

class IoLoopObserver {
 public:
  virtual void PreLooped(IoLoop* server) = 0;

  virtual void Accepted(IoClient* client) = 0;
  virtual void Moved(IoLoop* server, IoClient* client) = 0;  // owner server, now client is orphan
  virtual void Closed(IoClient* client) = 0;
  virtual void TimerEmited(IoLoop* server, timer_id_t id) = 0;
  virtual void Accepted(IoChild* child) = 0;
  virtual void Moved(IoLoop* server, IoChild* child) = 0;  // owner server, now child is orphan
  virtual void ChildStatusChanged(IoChild* child, int status, int signal) = 0;

  virtual void DataReceived(IoClient* client) = 0;
  virtual void DataReadyToWrite(IoClient* client) = 0;

  virtual void PostLooped(IoLoop* server) = 0;

  virtual ~IoLoopObserver();
};

}  // namespace libev
}  // namespace common
