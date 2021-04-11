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

#include <common/qt/gui/app_style.h>

#include <QApplication>
#include <QStyleFactory>

namespace common {
namespace qt {
namespace gui {

const QString defStyle = "Native";

void applyStyle(const QString& styleName) {
  if (styleName == defStyle) {
    QApplication::setStyle(new AppStyle);
  } else {
    QApplication::setStyle(QStyleFactory::create(styleName));
  }
}

QStringList supportedStyles() {
  static QStringList result = QStringList() << defStyle << QStyleFactory::keys();
  return result;
}

void applyFont(const QFont& font) {
  QApplication::setFont(font);
}

void AppStyle::drawControl(ControlElement element,
                           const QStyleOption* option,
                           QPainter* painter,
                           const QWidget* widget) const {
  QProxyStyle::drawControl(element, option, painter, widget);
}

void AppStyle::drawPrimitive(PrimitiveElement element,
                             const QStyleOption* option,
                             QPainter* painter,
                             const QWidget* widget) const {
#ifdef OS_WIN
  if (element == QStyle::PE_FrameFocusRect) {
    return;
  }
#endif  // OS_WIN

  return QProxyStyle::drawPrimitive(element, option, painter, widget);
}

QRect AppStyle::subElementRect(SubElement element, const QStyleOption* option, const QWidget* widget /*= 0 */) const {
  return QProxyStyle::subElementRect(element, option, widget);
}

}  // namespace gui
}  // namespace qt
}  // namespace common
