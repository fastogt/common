/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

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

#include <common/time.h>

#ifndef COMPILER_MSVC

namespace common {
namespace time {

struct timespec current_timespec() {
  struct timespec timeToWait;
  struct timeval now;
  gettimeofday(&now, NULL);
  timeToWait.tv_sec = now.tv_sec;
  timeToWait.tv_nsec = now.tv_usec * 1000;
  return timeToWait;
}

time64_t current_utc_mstime() {
  timeval cur_time = current_timeval();
  time_t now = ::time(NULL);
  struct tm utc_tm;
#ifdef OS_POSIX
  gmtime_r(&now, &utc_tm);
#else
  gmtime_s(&utc_tm, &now);
#endif
  struct tm local_tm;
#ifdef OS_POSIX
  localtime_r(&now, &local_tm);
#else
  localtime_s(&local_tm, &now);
#endif
  int offset_hours = local_tm.tm_hour - utc_tm.tm_hour;
  utc_tm.tm_hour += offset_hours;
  return mktime(&utc_tm) * 1000 + cur_time.tv_usec / 1000;
}

struct timeval current_timeval() {
  struct timeval tv;
  gettimeofday(&tv, NULL);
  return tv;
}

time64_t current_mstime() {
  struct timeval tv = current_timeval();
  return timeval2mstime(&tv);
}

struct timespec timeval2timespec(struct timeval* tv) {
  if (!tv) {
    return timespec();
  }

  struct timespec ts;
  ts.tv_sec = tv->tv_sec;
  ts.tv_nsec = tv->tv_usec * 1000;
  return ts;
}

struct timeval timespec2timeval(struct timespec* ts) {
  if (!ts) {
    return timeval();
  }

  struct timeval tv;
  tv.tv_sec = ts->tv_sec;
  tv.tv_usec = (ts->tv_nsec / 1000);
  return tv;
}

time64_t timeval2mstime(struct timeval* tv) {
  if (!tv) {
    return 0;
  }

  time64_t mst = static_cast<time64_t>(tv->tv_sec) * 1000;
  mst += tv->tv_usec / 1000;
  return mst;
}

time64_t timespec2mstime(struct timespec* ts) {
  if (!ts) {
    return 0;
  }

  time64_t mst = static_cast<time64_t>(ts->tv_sec) * 1000;
  mst += ts->tv_nsec / 1000000;
  return mst;
}

struct timeval mstime2timeval(time64_t mst) {
  if (mst < 0) {
    return timeval();
  }

  timeval tv;
  tv.tv_sec = mst / 1000;
  tv.tv_usec = (mst % 1000) * 1000;
  return tv;
}

struct timespec mstime2timespec(time64_t mst) {
  if (mst < 0) {
    return timespec();
  }

  timespec ts;
  ts.tv_sec = mst / 1000;
  ts.tv_nsec = (mst % 1000) * 1000000;
  return ts;
}

}  // namespace time
}  // namespace common
#endif