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

#include <common/qt/utils_qt.h>

#include <QApplication>
#include <QFile>
#include <QFileInfo>
#include <QTextStream>

#include <common/qt/convert2string.h>  // for ConvertToString

namespace common {
namespace qt {

WaitCursorHolder::WaitCursorHolder() {
  QApplication::setOverrideCursor(Qt::WaitCursor);
}

WaitCursorHolder::~WaitCursorHolder() {
  QApplication::restoreOverrideCursor();
}

QString ApplicationDirPath() {
  return QApplication::applicationDirPath();
}

Error LoadFromFileText(const QString& filePath, QString* outText) {
  if (!outText) {
    return make_inval_error_value(ERROR_TYPE);
  }

  QFile file(filePath);
  if (file.open(QFile::ReadOnly | QFile::Text)) {
    QTextStream in(&file);
    WaitCursorHolder h;
    UNUSED(h);
    *outText = in.readAll();
    return Error();
  }

  return make_error_value(ConvertToString(file.errorString()), ERROR_TYPE);
}

Error SaveToFileText(QString filePath, const QString& text) {
  if (filePath.isEmpty()) {
    return make_inval_error_value(ERROR_TYPE);
  }

#ifdef OS_LINUX
  if (QFileInfo(filePath).suffix().isEmpty()) {
    filePath += ".txt";
  }
#endif
  QFile file(filePath);
  if (file.open(QFile::WriteOnly | QFile::Text)) {
    QTextStream out(&file);
    WaitCursorHolder h;
    UNUSED(h);
    out << text;
    return Error();
  }

  return make_error_value(ConvertToString(file.errorString()), ERROR_TYPE);
}

}  // namespace qt
}  // namespace common
