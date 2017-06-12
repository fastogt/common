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

#include <common/logger.h>

#include <stdio.h>     // for fflush, fprintf, FILE, etc
#include <sys/time.h>  // for CLOCK_REALTIME
#include <time.h>      // for timespec, clock_gettime, etc

#include <fstream>
#include <iostream>
#include <string>  // for allocator, string, etc

#include <common/macros.h>  // for UNUSED
#include <common/smart_ptr.h>
#include <common/sprintf.h>

#include <common/threads/types.h>

#ifdef OS_MACOSX
#include <mach/clock.h>
#include <mach/mach.h>
#endif

namespace common {
namespace logging {

namespace {

std::string g_project_name = "Unknown";
common::unique_ptr<std::ofstream> g_logger_file_helper = common::unique_ptr<std::ofstream>(new std::ofstream);
common::mutex g_mutex;
std::ostream* g_logger = &std::cout;
LEVEL_LOG g_level_log = L_NOTICE;

std::string PrepareHeader(const char* file, int line, LEVEL_LOG level) {
  // We use fprintf() instead of cerr because we want this to work at static
  // initialization time.

  char buf[80];
  struct timespec spec;
#ifdef OS_MACOSX
  clock_serv_t cclock;
  mach_timespec_t mts;
  host_get_clock_service(mach_host_self(), CALENDAR_CLOCK, &cclock);
  clock_get_time(cclock, &mts);
  mach_port_deallocate(mach_task_self(), cclock);
  spec.tv_sec = mts.tv_sec;
  spec.tv_nsec = mts.tv_nsec;
#else
  clock_gettime(CLOCK_REALTIME, &spec);
#endif
  long ms = spec.tv_nsec / 1.0e6;  // Convert nanoseconds to milliseconds
  struct tm info;
  localtime_r(&spec.tv_sec, &info);
  strftime(buf, sizeof(buf), "%H:%M:%S", &info);

  if (file) {
    return MemSPrintf("%s:%d %s.%03ld %s [%s] ", file, line, buf, ms, normalize(g_project_name),
                      log_level_to_text(level));
  }

  return MemSPrintf("%s.%03ld %s [%s] ", buf, ms, normalize(g_project_name), log_level_to_text(level));
}

}  // namespace

void INIT_LOGGER(const std::string& project_name, LEVEL_LOG level) {
  g_level_log = level;
  common::logging::g_project_name = project_name;
}

void INIT_LOGGER(const std::string& project_name, const std::string& file_path, LEVEL_LOG level) {
  INIT_LOGGER(project_name, level);
  g_logger_file_helper->open(file_path, std::ofstream::out | std::ofstream::app);
  if (g_logger_file_helper->is_open()) {
    SET_LOGER_STREAM(g_logger_file_helper.get());
  } else {
    const char* err_str = common_strerror(errno);
    WARNING_LOG() << "Can't open file: " << file_path << " , error: " << err_str;
  }
}

void SET_LOGER_STREAM(std::ostream* logger) {
  if (!logger) {
    return;
  }

  g_logger = logger;
}

bool LOG_IS_ON(LEVEL_LOG level) {
  if (level > g_level_log) {
    return false;
  }
  return true;
}

LEVEL_LOG CURRENT_LOG_LEVEL() {
  return g_level_log;
}

void SET_CURRENT_LOG_LEVEL(LEVEL_LOG level) {
  g_level_log = level;
}

LogMessage::LogMessage(LEVEL_LOG level, bool new_line) : file_(), line_(), level_(level), new_line_(new_line) {
  g_mutex.lock();
  Stream() << PrepareHeader(NULL, 0, level);
}

LogMessage::LogMessage(const char* file, int line, LEVEL_LOG level, bool new_line)
    : file_(file), line_(line), level_(level), new_line_(new_line) {
  g_mutex.lock();
  Stream() << PrepareHeader(file, line, level);
}

LogMessage::~LogMessage() {
  if (new_line_) {
    Stream() << std::endl;
  }
  g_mutex.unlock();

  if (level_ <= common::logging::L_CRIT) {
#ifdef NDEBUG
    immediate_exit();
#else
    immediate_assert();
#endif
  }
}

std::ostream& LogMessage::Stream() {
  return *g_logger;
}

}  // namespace logging
}  // namespace common
