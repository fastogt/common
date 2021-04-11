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

#include <sstream>
#include <string>

#include <common/log_levels.h>

namespace common {
namespace logging {
// This class is used to explicitly ignore values in the conditional
// logging macros.  This avoids compiler warnings like "value computed
// is not used" and "statement has no effect".
class LogMessageVoidify {
 public:
  LogMessageVoidify() {}
  // This has to be an operator with a precedence lower than << but
  // higher than ?:
  void operator&(std::ostream&)const {}
};

bool LOG_IS_ON(LOG_LEVEL level);
LOG_LEVEL CURRENT_LOG_LEVEL();
void SET_CURRENT_LOG_LEVEL(LOG_LEVEL level);

void SET_LOGGER_PROJECT_NAME(const std::string& project_name);
std::string LOGGER_PROJECT_NAME();

class LogMessage {
 public:
  explicit LogMessage(LOG_LEVEL level, bool new_line = true);
  LogMessage(const char* file, int line, LOG_LEVEL level, bool new_line = true);
  ~LogMessage();

  std::ostream& Stream();

 private:
  LogMessage(const LogMessage&);
  void operator=(const LogMessage&);

  const char* file_;
  const int line_;
  const LOG_LEVEL level_;
  const bool new_line_;
  std::ostringstream stream_;
};

void INIT_LOGGER(const std::string& project_name, LOG_LEVEL level);  // to console
void INIT_LOGGER(const std::string& project_name,
                 const std::string& file_path,
                 LOG_LEVEL level,
                 ssize_t max_size = -1);  // to file, max_size = -1 => unlimited file

void SET_LOGER_STREAM(std::ostream* logger);

}  // namespace logging
}  // namespace common

// A few definitions of macros that don't generate much code. These are used
// by LOG() and LOG_IF, etc. Since these are used all over our code, it's
// better to have compact code for these operations.
#define COMPACT_LOG_EX_CRIT(ClassName) common::logging::ClassName(common::logging::LOG_LEVEL_CRIT)
#define COMPACT_LOG_EX_ERR(ClassName) common::logging::ClassName(common::logging::LOG_LEVEL_ERR)
#define COMPACT_LOG_EX_WARNING(ClassName) common::logging::ClassName(common::logging::LOG_LEVEL_WARNING)
#define COMPACT_LOG_EX_NOTICE(ClassName) common::logging::ClassName(common::logging::LOG_LEVEL_NOTICE)
#define COMPACT_LOG_EX_INFO(ClassName) common::logging::ClassName(common::logging::LOG_LEVEL_INFO)
#define COMPACT_LOG_EX_DEBUG(ClassName) common::logging::ClassName(common::logging::LOG_LEVEL_DEBUG)

#define COMPACT_LOG_FILE_EX_CRIT(ClassName) \
  common::logging::ClassName(__FILE__, __LINE__, common::logging::LOG_LEVEL_CRIT)
#define COMPACT_LOG_FILE_EX_ERR(ClassName) \
  common::logging::ClassName(__FILE__, __LINE__, common::logging::LOG_LEVEL_ERR)
#define COMPACT_LOG_FILE_EX_WARNING(ClassName) \
  common::logging::ClassName(__FILE__, __LINE__, common::logging::L_WARNING)
#define COMPACT_LOG_FILE_EX_NOTICE(ClassName) \
  common::logging::ClassName(__FILE__, __LINE__, common::logging::LOG_LEVEL_NOTICE)
#define COMPACT_LOG_FILE_EX_INFO(ClassName) \
  common::logging::ClassName(__FILE__, __LINE__, common::logging::LOG_LEVEL_INFO)
#define COMPACT_LOG_FILE_EX_DEBUG(ClassName) \
  common::logging::ClassName(__FILE__, __LINE__, common::logging::LOG_LEVEL_DEBUG)

#define COMPACT_LOG_CRIT COMPACT_LOG_EX_CRIT(LogMessage)
#define COMPACT_LOG_ERR COMPACT_LOG_EX_ERR(LogMessage)
#define COMPACT_LOG_WARNING COMPACT_LOG_EX_WARNING(LogMessage)
#define COMPACT_LOG_NOTICE COMPACT_LOG_EX_NOTICE(LogMessage)
#define COMPACT_LOG_INFO COMPACT_LOG_EX_INFO(LogMessage)
#define COMPACT_LOG_DEBUG COMPACT_LOG_EX_DEBUG(LogMessage)

#define COMPACT_LOG_FILE_CRIT COMPACT_LOG_FILE_EX_CRIT(LogMessage)
#define COMPACT_LOG_FILE_ERR COMPACT_LOG_FILE_EX_ERR(LogMessage)
#define COMPACT_LOG_FILE_WARNING COMPACT_LOG_FILE_EX_WARNING(LogMessage)
#define COMPACT_LOG_FILE_NOTICE COMPACT_LOG_FILE_EX_NOTICE(LogMessage)
#define COMPACT_LOG_FILE_INFO COMPACT_LOG_FILE_EX_INFO(LogMessage)
#define COMPACT_LOG_FILE_DEBUG COMPACT_LOG_FILE_EX_DEBUG(LogMessage)

#define LOG_FILE_LINE_STREAM(LEVEL) COMPACT_LOG_FILE_##LEVEL.Stream()
#define LOG_STREAM(LEVEL) COMPACT_LOG_##LEVEL.Stream()

#define LAZY_STREAM(stream, condition) !(condition) ? (void)0 : common::logging::LogMessageVoidify() & (stream)

#define LOG(LEVEL) LAZY_STREAM(LOG_STREAM(LEVEL), common::logging::LOG_IS_ON(common::logging::LOG_LEVEL_##LEVEL))

#define CRITICAL_LOG() LOG(CRIT)
#define ERROR_LOG() LOG(ERR)
#define WARNING_LOG() LOG(WARNING)
#define NOTICE_LOG() LOG(NOTICE)
#define INFO_LOG() LOG(INFO)
#define DEBUG_LOG() LOG(DEBUG)

#define RUNTIME_LOG(LEVEL) LAZY_STREAM(common::logging::LogMessage(LEVEL).Stream(), common::logging::LOG_IS_ON(LEVEL))
