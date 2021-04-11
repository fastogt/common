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

#include <utility>
#include <vector>

#include <QWidget>

namespace common {
namespace qt {
namespace gui {

struct plot_settings {
  plot_settings(qreal min_x, qreal max_x, qreal min_y, qreal max_y, unsigned num_x_ticks = 0, unsigned num_y_ticks = 0);

  void scroll(int dx, int dy);
  void adjust();
  qreal span_x() const;
  qreal span_y() const;
  plot_settings create_child(qreal dx, qreal dy, QRect rect) const;

  qreal min_x;
  qreal max_x;
  qreal min_y;
  qreal max_y;

  unsigned num_x_ticks;
  unsigned num_y_ticks;
};

class GraphWidget : public QWidget {
  Q_OBJECT
 public:
  enum { min_width = 640, min_height = 480 };

  typedef std::vector<std::pair<qreal, qreal>> nodes_container_type;
  explicit GraphWidget(QWidget* parent = Q_NULLPTR);

  void setNodes(const nodes_container_type& nodes);

 public Q_SLOTS:
  void zoom_in();
  void zoom_out();

 protected:
  void paintEvent(QPaintEvent* event) override;
  void mousePressEvent(QMouseEvent* event) override;
  void mouseMoveEvent(QMouseEvent* event) override;
  void mouseReleaseEvent(QMouseEvent* event) override;
  void keyPressEvent(QKeyEvent* event) override;
  void wheelEvent(QWheelEvent* event) override;

 private:
  void updateRubberBandRegion();
  void drawGrid(QPainter* painter);
  void drawCurves(QPainter* painter);
  QRect paintRect() const;

  enum { margin = 100 };

  static const Qt::GlobalColor grid_color = Qt::black, rubber_color = Qt::blue, line_color = Qt::red;

  std::vector<plot_settings> zoom_stack_;
  unsigned cur_zoom_;
  bool rubber_band_is_shown_;
  QRect rubber_band_rect_;
  nodes_container_type nodes_;
};

}  // namespace gui
}  // namespace qt
}  // namespace common
