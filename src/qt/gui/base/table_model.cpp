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

#include <common/qt/gui/base/table_model.h>

#include <stddef.h>  // for size_t

#include <common/macros.h>  // for UNUSED

#include <common/qt/gui/base/table_item.h>  // for TableItem

namespace common {
namespace qt {
namespace gui {

TableModel::TableModel(QObject* parent) : QAbstractTableModel(parent) {}

TableModel::~TableModel() {}

int TableModel::rowCount(const QModelIndex& parent) const {
  UNUSED(parent);

  return static_cast<int>(data_.size());
}

QModelIndex TableModel::index(int row, int column, const QModelIndex& parent) const {
  if (!hasIndex(row, column, parent)) {
    return QModelIndex();
  }

  TableItem* childItem = data_[row];
  if (childItem) {
    return createIndex(row, column, childItem);
  }
  return QModelIndex();
}

void TableModel::insertItem(TableItem* child) {
  if (!child) {
    return;
  }

  beginInsertRows(QModelIndex(), data_.size(), data_.size());
  data_.push_back(child);
  endInsertRows();
}

void TableModel::removeItem(TableItem* child) {
  if (!child) {
    return;
  }

  size_t child_count = data_.size();
  int index = -1;
  for (size_t i = 0; i < child_count; ++i) {
    if (data_[i] == child) {
      index = static_cast<int>(i);
      break;
    }
  }

  if (index == -1) {
    return;
  }

  beginRemoveRows(QModelIndex(), index, index);
  data_.erase(data_.begin() + index);
  delete child;
  endRemoveRows();
}

void TableModel::updateItem(const QModelIndex& topLeft, const QModelIndex& bottomRight) {
  emit dataChanged(topLeft, bottomRight);
}

}  // namespace gui
}  // namespace qt
}  // namespace common
