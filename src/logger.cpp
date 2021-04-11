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

#include <common/logger.h>

#include <errno.h>

#include <fstream>
#include <iostream>
#include <memory>

#include <common/file_system/file_system.h>
#include <common/file_system/types.h>
#include <common/patterns/singleton_pattern.h>
#include <common/sprintf.h>

#if defined(OS_MACOSX)
#include <mach/clock.h>
#include <mach/mach.h>
#endif

namespace common {
namespace logging {

namespace {

class LoggerInternal {
  friend class patterns::LazySingleton<LoggerInternal>;
  LoggerInternal() : project_name_("Unknown"), log_level_(LOG_LEVEL_NOTICE) {}

 public:
  LOG_LEVEL LogLevel() const { return log_level_; }
  void SetLogLevel(LOG_LEVEL log_level) { log_level_ = log_level; }

  std::string ProjectName() const { return project_name_; }
  void SetProjectName(const std::string& project_name) { project_name_ = project_name; }

  bool IsLogOn(LOG_LEVEL level) {
    if (level > log_level_) {
      return false;
    }

    return true;
  }

  std::string PrepareHeader(const char* file, int line, LOG_LEVEL level) {
    // We use fprintf() instead of cerr because we want this to work at static
    // initialization time.

    char buf[80];
    struct timespec spec;
#if defined(OS_MACOSX)
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
#if defined(OS_WIN)
    localtime_s(&info, &spec.tv_sec);
#else
    localtime_r(&spec.tv_sec, &info);
#endif
    strftime(buf, sizeof(buf), "%H:%M:%S", &info);

    if (file) {
      return MemSPrintf("%s:%d %s.%03ld %s [%s] ", file, line, buf, ms, project_name_, log_level_to_text(level));
    }

    return MemSPrintf("%s.%03ld %s [%s] ", buf, ms, project_name_, log_level_to_text(level));
  }

  static LoggerInternal* GetInstance() { return &patterns::LazySingleton<LoggerInternal>::GetInstance(); }

 private:
  std::string project_name_;
  LOG_LEVEL log_level_;
};

std::unique_ptr<std::ofstream> kLoggerFileHelper = std::unique_ptr<std::ofstream>(new std::ofstream);
std::ostream* kLogger = &std::cout;
}  // namespace

void INIT_LOGGER(const std::string& project_name, LOG_LEVEL level) {
  SET_CURRENT_LOG_LEVEL(level);
  SET_LOGGER_PROJECT_NAME(project_name);
}

void INIT_LOGGER(const std::string& project_name, const std::string& file_path, LOG_LEVEL level, ssize_t max_size) {
  INIT_LOGGER(project_name, level);
  const std::string stabled_path = file_system::prepare_path(file_path);
  if (kLoggerFileHelper->is_open()) {
    kLoggerFileHelper->close();
  }

  if (max_size != -1) {
    off_t file_size = 0;
    common::ErrnoError err = file_system::get_file_size_by_path(stabled_path, &file_size);
    if (!err) {
      if (file_size > max_size) {
        err = file_system::remove_file(stabled_path);
        if (err) {
          WARNING_LOG() << "Can't remove file: " << stabled_path << ", error: " << err->GetDescription();
        }
      }
    }
  }

  kLoggerFileHelper->open(stabled_path, std::ofstream::out | std::ofstream::app);
  if (kLoggerFileHelper->is_open()) {
    SET_LOGER_STREAM(kLoggerFileHelper.get());
    return;
  }

  WARNING_LOG() << "Can't open file: " << stabled_path << ", error: " << strerror(errno);
}

void SET_LOGER_STREAM(std::ostream* logger) {
  if (!logger) {
    return;
  }

  kLogger = logger;
}

bool LOG_IS_ON(LOG_LEVEL level) {
  return LoggerInternal::GetInstance()->IsLogOn(level);
}

LOG_LEVEL CURRENT_LOG_LEVEL() {
  return LoggerInternal::GetInstance()->LogLevel();
}

void SET_CURRENT_LOG_LEVEL(LOG_LEVEL level) {
  LoggerInternal::GetInstance()->SetLogLevel(level);
}

void SET_LOGGER_PROJECT_NAME(const std::string& project_name) {
  LoggerInternal::GetInstance()->SetProjectName(project_name);
}

std::string LOGGER_PROJECT_NAME() {
  return LoggerInternal::GetInstance()->ProjectName();
}

LogMessage::LogMessage(LOG_LEVEL level, bool new_line) : file_(), line_(), level_(level), new_line_(new_line) {
  stream_ << LoggerInternal::GetInstance()->PrepareHeader(nullptr, 0, level);
}

LogMessage::LogMessage(const char* file, int line, LOG_LEVEL level, bool new_line)
    : file_(file), line_(line), level_(level), new_line_(new_line), stream_() {
  stream_ << LoggerInternal::GetInstance()->PrepareHeader(file, line, level);
}

LogMessage::~LogMessage() {
  if (new_line_) {
    stream_ << "\n";
  }

  *kLogger << stream_.str();
  kLogger->flush();
  if (level_ <= logging::LOG_LEVEL_CRIT) {
#if defined(NDEBUG)
    immediate_exit();
#else
    immediate_assert();
#endif
  }
}

std::ostream& LogMessage::Stream() {
  return stream_;
}

}  // namespace logging
}  // namespace common
