/*  Copyright (C) 2014-2017 FastoGT. All right reserved.

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

#include <common/string_util.h>

namespace {
const char* g_level_names[] = {"EMERG", "ALLERT", "CRITICAL", "ERROR", "WARNING", "NOTICE", "INFO", "DEBUG"};
}

namespace common {
namespace logging {

const char* log_level_to_text(LOG_LEVEL level) {
  if (level >= LOG_LEVEL_COUNT) {
    DNOTREACHED() << "Ivalid input log level: " << level;
    return NULL;
  }

  return g_level_names[level];
}

bool text_to_log_level(const char* level_text, LOG_LEVEL* level) {
  if (!level_text || !level) {
    DNOTREACHED() << "Ivalid input level_text: " << level_text << ", level out:" << level;
    return false;
  }

  for (size_t i = 0; i < SIZEOFMASS(g_level_names); ++i) {
    const char* name = g_level_names[i];
    if (strcasecmp(level_text, name) == 0) {
      *level = static_cast<LOG_LEVEL>(i);
      return true;
    }
  }

  return false;
}

}  // namespace logging
}  // namespace common
