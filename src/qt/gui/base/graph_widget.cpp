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

#include <common/qt/gui/base/graph_widget.h>

#include <math.h>    // for floor, ceil, log10, pow
#include <stddef.h>  // for size_t
#include <string>    // for string

#include <QDateTime>
#include <QStylePainter>
#include <QWheelEvent>

#include <common/convert2string.h>
#include <common/macros.h>             // for UNUSED
#include <common/qt/convert2string.h>  // for ConvertFromString, etc

namespace common {
namespace qt {
namespace gui {
namespace {

bool is_valid_setting(const plot_settings& settings) {
  return settings.min_x >= 0 && settings.min_y >= 0;
}

void adjust_axis(qreal* min, qreal* max, unsigned* numTicks) {
  static const unsigned MinTicks = 4;
  qreal grossStep = (*max - *min) / MinTicks;
  qreal step = pow(10.0, floor(log10(grossStep)));

  if (5 * step < grossStep) {
    step *= 5;
  } else if (2 * step < grossStep) {
    step *= 2;
  }

  *numTicks = int(ceil(*max / step) - floor(*min / step));
  if (*numTicks < MinTicks) {
    *numTicks = MinTicks;
  }
  *min = floor(*min / step) * step;
  *max = ceil(*max / step) * step;
}

}  // namespace

plot_settings::plot_settings(qreal min_x,
                             qreal max_x,
                             qreal min_y,
                             qreal max_y,
                             unsigned num_x_ticks,
                             unsigned num_y_ticks)
    : min_x(min_x), max_x(max_x), min_y(min_y), max_y(max_y), num_x_ticks(num_x_ticks), num_y_ticks(num_y_ticks) {}

void plot_settings::scroll(int dx, int dy) {
  qreal stepX = span_x() / num_x_ticks;
  qreal stepY = span_y() / num_y_ticks;
  plot_settings tmp(min_x + dx * stepX, max_x + dx * stepX, min_y + dy * stepY, max_y + dy * stepY);
  if (is_valid_setting(tmp)) {
    min_x += dx * stepX;
    max_x += dx * stepX;
    min_y += dy * stepY;
    max_y += dy * stepY;
  }
}

void plot_settings::adjust() {
  adjust_axis(&min_x, &max_x, &num_x_ticks);
  adjust_axis(&min_y, &max_y, &num_y_ticks);
}

qreal plot_settings::span_x() const {
  return max_x - min_x;
}

qreal plot_settings::span_y() const {
  return max_y - min_y;
}

plot_settings plot_settings::create_child(qreal dx, qreal dy, QRect rect) const {
  plot_settings settings(*this);
  settings.min_x = min_x + dx * rect.left();
  settings.max_x = min_x + dx * rect.right();
  settings.min_y = max_y - dy * rect.bottom();
  settings.max_y = max_y - dy * rect.top();
  settings.adjust();

  return settings;
}

GraphWidget::GraphWidget(QWidget* parent) : QWidget(parent), cur_zoom_(0), rubber_band_is_shown_(false), nodes_() {
  setBackgroundRole(QPalette::Light);
  setAutoFillBackground(true);
  setFocusPolicy(Qt::StrongFocus);
  setMinimumSize(QSize(min_width, min_height));
}

void GraphWidget::setNodes(const nodes_container_type& nodes) {
  nodes_ = nodes;
  zoom_stack_.clear();
  if (!nodes_.empty()) {
    qreal min_x = nodes_[0].first;
    qreal max_x = nodes_[0].first;
    qreal min_y = nodes_[0].second;
    qreal max_y = nodes_[0].second;

    for (size_t i = 0; i < nodes_.size(); ++i) {
      qreal curX = nodes_[i].first;
      qreal curY = nodes_[i].second;
      if (min_x > curX) {
        min_x = curX;
      }

      if (max_x < curX) {
        max_x = curX;
      }

      if (min_y > curY) {
        min_y = curY;
      }

      if (max_y < curY) {
        max_y = curY;
      }
    }

    zoom_stack_.push_back(plot_settings(min_x, max_x, min_y, max_y, 5, 5));
  }
  update();
}

void GraphWidget::zoom_out() {
  if (cur_zoom_ > 0) {
    --cur_zoom_;
    update();
  }
}

void GraphWidget::zoom_in() {
  if (cur_zoom_ < zoom_stack_.size() - 1) {
    ++cur_zoom_;
    update();
  }
}

void GraphWidget::drawGrid(QPainter* painter) {
  if (zoom_stack_.empty()) {
    return;
  }

  QRect rect = paintRect();
  if (!rect.isValid()) {
    return;
  }

  QFont font = painter->font();
  font.setPointSize(7);
  painter->setFont(font);

  plot_settings settings = zoom_stack_[cur_zoom_];
  painter->setPen(QColor(grid_color));
  for (unsigned i = 0; i <= settings.num_x_ticks; ++i) {
    int x = rect.left() + (i * (rect.width() - 1) / settings.num_x_ticks);
    qreal label = settings.min_x + (i * settings.span_x() / settings.num_x_ticks);
    painter->drawLine(x, rect.top(), x, rect.bottom());
    painter->drawLine(x, rect.bottom(), x, rect.bottom() + 5);
    QDateTime t = QDateTime::fromMSecsSinceEpoch(static_cast<qint64>(label));
    painter->drawText(x - 50, rect.bottom() + 5, 120, 20, Qt::AlignHCenter | Qt::AlignTop, t.toString(Qt::ISODate));
  }

  for (unsigned j = 0; j <= settings.num_y_ticks; ++j) {
    int y = rect.bottom() - (j * (rect.height() - 1) / settings.num_y_ticks);
    qreal label = settings.min_y + (j * settings.span_y() / settings.num_y_ticks);
    painter->drawLine(rect.left(), y, rect.right(), y);
    painter->drawLine(rect.left() - 5, y, rect.left(), y);

    std::string slabel = common::ConvertToString(label);
    QString numb;
    bool res = common::ConvertFromString(slabel, &numb);
    UNUSED(res);
    painter->drawText(rect.left() - margin, y - 10, margin - 5, 20, Qt::AlignRight | Qt::AlignVCenter, numb);
  }

  painter->drawRect(rect.adjusted(0, 0, -1, -1));
}

void GraphWidget::drawCurves(QPainter* painter) {
  if (zoom_stack_.empty()) {
    return;
  }

  QRect rect = paintRect();

  if (!rect.isValid()) {
    return;
  }

  painter->setClipRect(rect.adjusted(+1, +1, -1, -1));

  QPolygonF polyline;
  plot_settings settings = zoom_stack_[cur_zoom_];
  for (unsigned int i = 0; i < nodes_.size(); ++i) {
    qreal dx = nodes_[i].first - settings.min_x;
    qreal dy = nodes_[i].second - settings.min_y;
    qreal x = rect.left() + (dx * (rect.width() - 1) / settings.span_x());
    qreal y = rect.bottom() - (dy * (rect.height() - 1) / settings.span_y());

    polyline.push_back(QPointF(x, y));
  }
  painter->setPen(QColor(line_color));
  painter->drawPolyline(polyline);
}

void GraphWidget::updateRubberBandRegion() {
  QRect rect = rubber_band_rect_.normalized();
  update(rect.left(), rect.top(), rect.width(), 1);
  update(rect.left(), rect.top(), 1, rect.height());
  update(rect.left(), rect.bottom(), rect.width(), 1);
  update(rect.right(), rect.top(), 1, rect.height());
}

void GraphWidget::paintEvent(QPaintEvent* event) {
  UNUSED(event);

  if (rubber_band_is_shown_) {
    QStylePainter spainter(this);
    spainter.setPen(QColor(rubber_color));
    spainter.drawRect(rubber_band_rect_.normalized().adjusted(0, 0, -1, -1));
  }

  QPainter painter(this);
  drawGrid(&painter);
  drawCurves(&painter);
}

void GraphWidget::mousePressEvent(QMouseEvent* event) {
  if (zoom_stack_.empty()) {
    return;
  }

  if (event->button() == Qt::LeftButton) {
    if (paintRect().contains(event->pos())) {
      rubber_band_is_shown_ = true;
      rubber_band_rect_.setTopLeft(event->pos());
      rubber_band_rect_.setBottomRight(event->pos());
      updateRubberBandRegion();
      setCursor(Qt::CrossCursor);
    }
  }
}

void GraphWidget::mouseMoveEvent(QMouseEvent* event) {
  if (rubber_band_is_shown_) {
    updateRubberBandRegion();
    rubber_band_rect_.setBottomRight(event->pos());
    updateRubberBandRegion();
  }

  // QWidget::mouseMoveEvent(event);
}

void GraphWidget::mouseReleaseEvent(QMouseEvent* event) {
  if (zoom_stack_.empty()) {
    return;
  }

  if ((event->button() == Qt::LeftButton) && rubber_band_is_shown_) {
    rubber_band_is_shown_ = false;
    updateRubberBandRegion();
    unsetCursor();

    QRect rect = rubber_band_rect_.normalized();
    if (rect.width() > 4 && rect.height() > 4) {
      rect.translate(-margin, -margin);

      plot_settings prevSettings = zoom_stack_[cur_zoom_];
      qreal dx = prevSettings.span_x() / qreal(width() - 2 * margin);
      qreal dy = prevSettings.span_y() / qreal(height() - 2 * margin);
      plot_settings settings = prevSettings.create_child(dx, dy, rect);
      if (is_valid_setting(settings)) {
        zoom_stack_.push_back(settings);
        zoom_in();
        update();
      }
    }
  }
}

void GraphWidget::keyPressEvent(QKeyEvent* event) {
  switch (event->key()) {
    case Qt::Key_Plus:
      zoom_in();
      break;
    case Qt::Key_Minus:
      zoom_out();
      break;
    case Qt::Key_Left:
      zoom_stack_[cur_zoom_].scroll(-1, 0);
      update();
      break;
    case Qt::Key_Right:
      zoom_stack_[cur_zoom_].scroll(+1, 0);
      update();
      break;
    case Qt::Key_Down:
      zoom_stack_[cur_zoom_].scroll(0, -1);
      update();
      break;
    case Qt::Key_Up:
      zoom_stack_[cur_zoom_].scroll(0, +1);
      update();
      break;
    default:
      break;
  }

  QWidget::keyPressEvent(event);
}

void GraphWidget::wheelEvent(QWheelEvent* event) {
  int numDegrees = event->delta() / 8;
  int numTicks = numDegrees / 15;
  if (event->orientation() == Qt::Horizontal) {
    zoom_stack_[cur_zoom_].scroll(numTicks, 0);
  } else {
    zoom_stack_[cur_zoom_].scroll(0, numTicks);
  }
  update();
}

QRect GraphWidget::paintRect() const {
  return QRect(margin, margin, width() - 2 * margin, height() - 2 * margin);
}

}  // namespace gui
}  // namespace qt
}  // namespace common
