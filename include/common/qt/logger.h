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

#include <QObject>
#include <QString>

#include <common/error.h>                       // for Error
#include <common/patterns/singleton_pattern.h>  // for LazySingleton

namespace common {
namespace qt {

class Logger : public QObject, public patterns::LazySingleton<Logger> {
  friend class patterns::LazySingleton<Logger>;
  Q_OBJECT
 public:
  void print(const char* mess, logging::LOG_LEVEL level, bool notify);
  void print(const QString& mess, logging::LOG_LEVEL level, bool notify);
  void print(const std::string& mess, logging::LOG_LEVEL level, bool notify);

 Q_SIGNALS:
  void printed(const QString& mess, logging::LOG_LEVEL level);

 private:
  Logger();
};

}  // namespace qt
}  // namespace common

template <typename T>
inline void LOG_MSG(T mess, common::logging::LOG_LEVEL level, bool notify) {
  return common::qt::Logger::GetInstance().print(mess, level, notify);
}

inline void LOG_ERROR(common::Error err, common::logging::LOG_LEVEL level, bool notify) {
  return common::qt::Logger::GetInstance().print(err->GetDescription(), level, notify);
}
