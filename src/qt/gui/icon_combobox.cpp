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

#include <common/qt/gui/icon_combobox.h>

#include <QComboBox>
#include <QHBoxLayout>
#include <QIcon>
#include <QLabel>

namespace common {
namespace qt {
namespace gui {

IconComboBox::IconComboBox(QWidget* parent) : QWidget(parent), icon_(NULL), combo_(NULL) {
  init(QIcon(), QSize());
}

IconComboBox::IconComboBox(const QIcon& icon, const QSize& icon_size, QWidget* parent)
    : QWidget(parent), icon_(NULL), combo_(NULL) {
  init(icon, icon_size);
}

void IconComboBox::init(const QIcon& icon, const QSize& icon_size) {
  QHBoxLayout* mainL = new QHBoxLayout;
  icon_ = new QLabel;
  combo_ = new QComboBox;
  mainL->addWidget(icon_);
  mainL->addWidget(combo_);

  setIcon(icon, icon_size);
  setLayout(mainL);
}

void IconComboBox::setIcon(const QIcon& icon, const QSize& size) {
  const QPixmap pm = icon.pixmap(size);
  icon_->setPixmap(pm);
  icon_->setFixedSize(size);
}

QIcon IconComboBox::comboItemIcon(int index) const {
  return combo_->itemIcon(index);
}

QString IconComboBox::comboItemText(int index) const {
  return combo_->itemText(index);
}

int IconComboBox::comboCurrentIndex() const {
  return combo_->currentIndex();
}

QString IconComboBox::comboCurrentText() const {
  return combo_->currentText();
}

void IconComboBox::addComboItem(const QString& text, const QVariant& userData) {
  combo_->addItem(text, userData);
}

void IconComboBox::addComboItem(const QIcon& icon, const QString& text, const QVariant& userData) {
  combo_->addItem(icon, text, userData);
}

void IconComboBox::addComboItems(const QStringList& texts) {
  combo_->addItems(texts);
}

void IconComboBox::clearCombo() {
  combo_->clear();
}

}  // namespace gui
}  // namespace qt
}  // namespace common
