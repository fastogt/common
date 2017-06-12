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

#include <common/qt/gui/glass_widget.h>

#include <QLabel>
#include <QMovie>
#include <QEvent>

#include <QGraphicsOpacityEffect>

namespace common {
namespace qt {
namespace gui {

GlassWidget::GlassWidget(const QString& path, const QString& text, qreal opacity, const QColor& color, QObject* parent)
    : QObject(parent), movie_(nullptr), text_(text) {
  wGlass_ = new QLabel;

  wGlass_->setStyleSheet(QString("background-color: %1;").arg(color.name()));
  QGraphicsOpacityEffect* tmpEffect = new QGraphicsOpacityEffect;
  tmpEffect->setOpacity(opacity);
  wGlass_->setGraphicsEffect(tmpEffect);

  wAnimationContainer_ = new QLabel;
  wInfoTextContaiter_ = new QLabel;

  movie_ = new QMovie(path, QByteArray(), this);
  wAnimationContainer_->setMovie(movie_);
}

GlassWidget::~GlassWidget() {
  wGlass_->deleteLater();
  wAnimationContainer_->deleteLater();
  wInfoTextContaiter_->deleteLater();
  movie_->deleteLater();
}

void GlassWidget::start() {
  QWidget* widget = qobject_cast<QWidget*>(parent());

  wGlass_->setParent(widget);
  widget->installEventFilter(this);
  wAnimationContainer_->setParent(widget);
  wInfoTextContaiter_->setParent(widget);

  showInfoTextBlock();
  showAnimationBlock();

  QEvent event(QEvent::Resize);
  eventFilter(0, &event);
}

void GlassWidget::stop() {
  QWidget* parent = wGlass_->parentWidget();
  if (parent) {
    parent->removeEventFilter(this);
  }

  wGlass_->hide();
  wGlass_->setParent(nullptr);
  wAnimationContainer_->hide();
  wInfoTextContaiter_->hide();
  wAnimationContainer_->setParent(nullptr);
  wInfoTextContaiter_->setParent(nullptr);
}

bool GlassWidget::eventFilter(QObject* /* object */, QEvent* event) {
  if ((event->type() == QEvent::Show) || (event->type() == QEvent::Resize)) {
    wGlass_->resize(wGlass_->parentWidget()->size());
    wGlass_->move(0, 0);
    infoBlockPositioning();
    return true;
  }

  wGlass_->setFocus();
  event->accept();
  return false;
}

void GlassWidget::showInfoTextBlock() {
  wInfoTextContaiter_->setText(text_);
  wInfoTextContaiter_->show();
  wInfoTextContaiter_->adjustSize();
}

void GlassWidget::showAnimationBlock() {
  movie_->jumpToFrame(0);
  wAnimationContainer_->resize(movie_->currentPixmap().size());
  movie_->start();

  wAnimationContainer_->show();
  wGlass_->show();
}

void GlassWidget::infoBlockPositioning() {
  if (wAnimationContainer_->isVisible() && wInfoTextContaiter_->isVisible()) {
    wAnimationContainer_->move((wGlass_->width() - wAnimationContainer_->width()) / 2,
                               wGlass_->height() / 2 - wAnimationContainer_->height());
    wInfoTextContaiter_->move((wGlass_->width() - wInfoTextContaiter_->width()) / 2,
                              wGlass_->height() / 2 + wInfoTextContaiter_->height());
  } else {
    if (wAnimationContainer_->isVisible()) {
      wAnimationContainer_->move((wGlass_->width() - wAnimationContainer_->width()) / 2,
                                 (wGlass_->height() - wAnimationContainer_->height()) / 2);
    }

    if (wInfoTextContaiter_->isVisible()) {
      wInfoTextContaiter_->move((wGlass_->width() - wInfoTextContaiter_->width()) / 2,
                                (wGlass_->height() - wInfoTextContaiter_->height()) / 2);
    }
  }
}

}  // namespace gui
}  // namespace qt
}  // namespace common
