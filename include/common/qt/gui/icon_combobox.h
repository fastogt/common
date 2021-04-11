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

#include <QVariant>
#include <QWidget>

class QLabel;
class QComboBox;

namespace common {
namespace qt {
namespace gui {

class IconComboBox : public QWidget {
  Q_OBJECT
 public:
  explicit IconComboBox(QWidget* parent = Q_NULLPTR);
  IconComboBox(const QIcon& icon, const QSize& icon_size, QWidget* parent = Q_NULLPTR);

  void setIcon(const QIcon& icon, const QSize& size);

  QIcon comboItemIcon(int index) const;
  QString comboItemText(int index) const;
  int comboCurrentIndex() const;
  QString comboCurrentText() const;
  void addComboItem(const QString& text, const QVariant& userData = QVariant());
  void addComboItem(const QIcon& icon, const QString& text, const QVariant& userData = QVariant());
  void addComboItems(const QStringList& texts);

  void clearCombo();

 private:
  void init(const QIcon& icon, const QSize& icon_size);

  QLabel* icon_;
  QComboBox* combo_;
};

}  // namespace gui
}  // namespace qt
}  // namespace common
