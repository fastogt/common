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

#include <common/qt/gui/regexp_input_dialog.h>

#include <QDialogButtonBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRegExpValidator>

#include <common/macros.h>

namespace common {
namespace qt {
namespace gui {

RegExpInputDialog::RegExpInputDialog(QWidget* parent, Qt::WindowFlags flags) : QDialog(parent) {
  if (flags != 0) {
    setWindowFlags(flags);
  }

  QVBoxLayout* l = new QVBoxLayout;

  label_ = new QLabel;

  QRegExp regExp("*");
  validator_ = new QRegExpValidator(regExp);

  text_ = new QLineEdit;
  text_->setValidator(validator_);
  VERIFY(connect(text_, SIGNAL(textChanged(QString)), this, SLOT(checkValid(QString))));

  buttonBox_ = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, Qt::Horizontal);
  VERIFY(connect(buttonBox_, &QDialogButtonBox::accepted, this, &RegExpInputDialog::accept));
  VERIFY(connect(buttonBox_, &QDialogButtonBox::rejected, this, &RegExpInputDialog::reject));

  l->addWidget(label_);
  l->addWidget(text_);
  l->addWidget(buttonBox_);
  l->setSizeConstraint(QLayout::SetFixedSize);
  setLayout(l);
}

void RegExpInputDialog::setLabelText(const QString& label) {
  label_->setText(label);
}

void RegExpInputDialog::setText(const QString& text) {
  text_->setText(text);
}

void RegExpInputDialog::setRegExp(const QRegExp& regExp) {
  validator_->setRegExp(regExp);
  checkValid(text_->text());
}

QString RegExpInputDialog::text() {
  return text_->text();
}

void RegExpInputDialog::checkValid(const QString& text) {
  QString _text = QString(text);
  int pos = 0;
  bool valid = validator_->validate(_text, pos) == QRegExpValidator::Acceptable;
  buttonBox_->button(QDialogButtonBox::Ok)->setEnabled(valid);
}

}  // namespace gui
}  // namespace qt
}  // namespace common
