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

#include <common/qt/translations/translations.h>

#include <QApplication>
#include <QDir>
#include <QTranslator>

#include <common/qt/utils_qt.h>

namespace {

QString kProject;
QTranslator kTr;

QString trPath() {
  const QString app_dir = common::qt::ApplicationDirPath();
  return app_dir + "/translations";
}

QStringList qmLanguages() {
  const QString tr_path = trPath();
  static QDir trd(tr_path);
  return trd.entryList(QStringList(kProject + "_*.qm"));
}

QPair<QString, QLocale> convertToLocale(const QString& fileName) {
  QString langCode = fileName;
  langCode.remove(0, fileName.indexOf('_') + 1);
  langCode.chop(3);
  return QPair<QString, QLocale>(fileName, langCode);
}

QString pathToQm(const QString& l) {
  const QStringList languages = qmLanguages();
  for (int i = 0; i < languages.size(); ++i) {
    QPair<QString, QLocale> p = convertToLocale(languages[i]);
    QString lang = QLocale::languageToString(p.second.language());
    if (l == lang) {
      return p.first;
    }
  }

  return QString();
}

const QString builtInLanguage = "English"; /* "English" is built-in */
}  // namespace

namespace common {
namespace qt {
namespace translations {

const QString defLanguage = "System";

QString applyLanguage(const QString& lang) {
  QString langres = lang;

  if (langres == defLanguage) {
    langres = QLocale::languageToString(QLocale::system().language());
  }

  QString qmPath = pathToQm(langres);
  bool isLoad = kTr.load(qmPath, trPath());

  if (!isLoad && langres != builtInLanguage) {
    return builtInLanguage;
  }

  return langres;
}

QStringList supportedLanguages() {
  const QStringList languages = qmLanguages();

  QStringList result;
  result << defLanguage << builtInLanguage;
  for (int i = 0; i < languages.size(); ++i) {
    QPair<QString, QLocale> p = convertToLocale(languages[i]);
    QString lang = QLocale::languageToString(p.second.language());
    result.append(lang);
  }
  return result;
}

}  // namespace translations
}  // namespace qt
}  // namespace common

void INIT_TRANSLATION(const QString& project_name) {
  kProject = project_name;
  qApp->installTranslator(&kTr);
}
