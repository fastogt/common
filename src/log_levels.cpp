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

#include <common/log_levels.h>

#include <string.h>

#include <common/macros.h>

namespace common {
namespace logging {

namespace {
const char* log_levels_names[LOG_NUM_LEVELS] = {"EMERG",   "ALLERT", "CRITICAL", "ERROR",
                                                "WARNING", "NOTICE", "INFO",     "DEBUG"};

const char* log_levels_name(int level) {
  if (level >= 0 && level < LOG_NUM_LEVELS) {
    return log_levels_names[level];
  }

  DNOTREACHED();
  return "UNKNOWN";
}
}  // namespace

const char* log_level_to_text(LOG_LEVEL level) {
  if (level < 0 || level >= LOG_NUM_LEVELS) {
    DNOTREACHED() << "Invalid input log level: " << level;
    return nullptr;
  }

  return log_levels_name(level);
}

bool text_to_log_level(const char* level_text, LOG_LEVEL* level) {
  if (!level_text || !level) {
    DNOTREACHED() << "Invalid input level_text: " << level_text << ", level out:" << level;
    return false;
  }

  for (int i = 0; i < LOG_NUM_LEVELS; ++i) {
    const char* name = log_levels_name(i);
    if (strcmp(level_text, name) == 0) {
      *level = static_cast<LOG_LEVEL>(i);
      return true;
    }
  }

  return false;
}

}  // namespace logging
}  // namespace common
