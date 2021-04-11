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

#include <QColor>
#include <QObject>

class QLabel;
class QMovie;

namespace common {
namespace qt {
namespace gui {

class GlassWidget : public QObject {
  Q_OBJECT
 public:
  GlassWidget(const QString& path,
              const QString& text,
              qreal opacity = 0.5,
              const QColor& color = QColor(111, 111, 100),
              QObject* parent = Q_NULLPTR);
  virtual ~GlassWidget() override;

  void start();
  void stop();

 protected:
  virtual bool eventFilter(QObject* object, QEvent* event) override;

 private:
  void showInfoTextBlock();
  void showAnimationBlock();
  void infoBlockPositioning();

  QLabel* wGlass_;
  QLabel* wAnimationContainer_;
  QLabel* wInfoTextContaiter_;
  QMovie* movie_;
  const QString text_;
};

}  // namespace gui
}  // namespace qt
}  // namespace common
