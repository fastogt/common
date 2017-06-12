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

#include <common/qt/gui/icon_label.h>

#include <QHBoxLayout>
#include <QLabel>
#include <QIcon>

namespace common {
namespace qt {
namespace gui {
IconLabel::IconLabel(const QIcon& icon, const QString& text, const QSize& size, QWidget* parent)
    : QWidget(parent), icon_(NULL), text_(NULL), el_mode_(Qt::ElideNone) {
  QHBoxLayout* mainL = new QHBoxLayout;
  icon_ = new QLabel;
  text_ = new QLabel;
  mainL->addWidget(icon_);
  mainL->addWidget(text_);

  setText(text);
  setIcon(icon, size);
  setLayout(mainL);
}

void IconLabel::setWordWrap(bool on) {
  text_->setWordWrap(on);
}

QString IconLabel::text() const {
  return text_->text();
}

void IconLabel::setText(const QString& text) {
  if (el_mode_ == Qt::ElideNone) {
    text_->setText(text);
    return;
  }

  QFontMetrics metrics = text_->fontMetrics();
  QString elidedText = metrics.elidedText(text, el_mode_, text_->width());
  text_->setText(elidedText);
}

void IconLabel::setIcon(const QIcon& icon, const QSize& size) {
  const QPixmap pm = icon.pixmap(size);
  icon_->setPixmap(pm);
}

Qt::TextElideMode IconLabel::elideMode() const {
  return el_mode_;
}

void IconLabel::setElideMode(Qt::TextElideMode mod) {
  el_mode_ = mod;
}
}  // namespace gui
}  // namespace qt
}  // namespace common
