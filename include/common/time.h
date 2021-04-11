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

#ifndef COMPILER_MSVC
#include <sys/time.h>  // for timeval

#include <common/types.h>  // for time64_t

namespace common {
namespace time {

time64_t current_utc_mstime();  // millisecond
struct timespec current_timespec();
struct timeval current_timeval();

struct timespec timeval2timespec(const struct timeval* tv);
struct timeval timespec2timeval(const struct timespec* ts);

time64_t timeval2mstime(const struct timeval* tv);    // millisecond
time64_t timespec2mstime(const struct timespec* ts);  // millisecond

utctime_t tm2utctime(struct tm* timestruct, bool is_local);
struct tm utctime2tm(utctime_t time_sec, bool is_local);

struct timeval mstime2timeval(time64_t mst);
struct timespec mstime2timespec(time64_t mst);

static constexpr int64_t kHoursPerDay = 24;
static constexpr int64_t kSecondsPerMinute = 60;
static constexpr int64_t kSecondsPerHour = 60 * kSecondsPerMinute;
static constexpr int64_t kMillisecondsPerSecond = 1000;
static constexpr int64_t kMillisecondsPerDay = kMillisecondsPerSecond * 60 * 60 * kHoursPerDay;
static constexpr int64_t kMicrosecondsPerMillisecond = 1000;
static constexpr int64_t kMicrosecondsPerSecond = kMicrosecondsPerMillisecond * kMillisecondsPerSecond;
static constexpr int64_t kMicrosecondsPerMinute = kMicrosecondsPerSecond * 60;
static constexpr int64_t kMicrosecondsPerHour = kMicrosecondsPerMinute * 60;
static constexpr int64_t kMicrosecondsPerDay = kMicrosecondsPerHour * kHoursPerDay;
static constexpr int64_t kMicrosecondsPerWeek = kMicrosecondsPerDay * 7;
static constexpr int64_t kNanosecondsPerMicrosecond = 1000;
static constexpr int64_t kNanosecondsPerSecond = kNanosecondsPerMicrosecond * kMicrosecondsPerSecond;

}  // namespace time
}  // namespace common

#endif
