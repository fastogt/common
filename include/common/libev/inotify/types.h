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

#if defined(OS_LINUX)

namespace common {
namespace libev {
namespace inotify {

enum {
  EV_IN_ACCESS = 0x00000001,                               /* File was accessed.  */
  EV_IN_MODIFY = 0x00000002,                               /* File was modified.  */
  EV_IN_ATTRIB = 0x00000004,                               /* Metadata changed.  */
  EV_IN_CLOSE_WRITE = 0x00000008,                          /* Writtable file was closed.  */
  EV_IN_CLOSE_NOWRITE = 0x00000010,                        /* Unwrittable file closed.  */
  EV_IN_CLOSE = (EV_IN_CLOSE_WRITE | EV_IN_CLOSE_NOWRITE), /* Close.  */
  EV_IN_OPEN = 0x00000020,                                 /* File was opened.  */
  EV_IN_MOVED_FROM = 0x00000040,                           /* File was moved from X.  */
  EV_IN_MOVED_TO = 0x00000080,                             /* File was moved to Y.  */
  EV_IN_MOVE = (EV_IN_MOVED_FROM | EV_IN_MOVED_TO),        /* Moves.  */
  EV_IN_CREATE = 0x00000100,                               /* Subfile was created.  */
  EV_IN_DELETE = 0x00000200,                               /* Subfile was deleted.  */
  EV_IN_DELETE_SELF = 0x00000400,                          /* Self was deleted.  */
  EV_IN_MOVE_SELF = 0x00000800                             /* Self was moved.  */
};

}  // namespace inotify
}  // namespace libev
}  // namespace common

#endif
