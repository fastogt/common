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

#include <common/qt/gui/shortcuts.h>

#include <QKeyEvent>

namespace common {
namespace qt {
namespace gui {

FastoQKeySequence::FastoQKeySequence(QKeySequence::StandardKey skey)
    : skey_(skey), mod_(Qt::NoModifier), key_(Qt::Key_unknown) {}

FastoQKeySequence::FastoQKeySequence(Qt::KeyboardModifiers mod, int key)
    : skey_(QKeySequence::UnknownKey), mod_(mod), key_(key) {}

FastoQKeySequence::operator QKeySequence() {
  if (mod_ == Qt::NoModifier && key_ == Qt::Key_unknown) {
    return QKeySequence(skey_);
  } else {
    return QKeySequence(mod_ + key_);
  }
}

FastoQKeySequence::operator QKeySequence() const {
  if (mod_ == Qt::NoModifier && key_ == Qt::Key_unknown) {
    return QKeySequence(skey_);
  } else {
    return QKeySequence(mod_ + key_);
  }
}

bool FastoQKeySequence::operator==(QKeyEvent* ev) const {
  if (!ev) {
    return false;
  }

  if (mod_ == Qt::NoModifier && key_ == Qt::Key_unknown) {
    return ev->matches(skey_);
  } else {
    return ev->modifiers() == mod_ && ev->key() == key_;
  }
}

bool isAutoCompleteShortcut(QKeyEvent* keyEvent) {
  return (keyEvent->modifiers() & Qt::ControlModifier) && (keyEvent->key() == Qt::Key_Space);
}

bool isHideAutoCompleteShortcut(QKeyEvent* keyEvent) {
  return (keyEvent->key() == Qt::Key_Escape);
}

}  // namespace gui
}  // namespace qt
}  // namespace common
