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

#include <common/qt/gui/base/tree_model.h>

#include <stddef.h>  // for size_t

#include <common/macros.h>  // for destroy

#include <common/qt/gui/base/tree_item.h>  // for TreeItem

namespace common {
namespace qt {
namespace gui {
namespace {

bool findChildInModel(const QModelIndex& parent,
                      void* user_data,
                      QModelIndex* index,
                      TreeItem* root,
                      QAbstractItemModel* model) {
  if (!root || !model || !index || !user_data) {
    return false;
  }

  TreeItem* par = nullptr;
  if (!parent.isValid()) {
    par = root;
  } else {
    par = static_cast<TreeItem*>(parent.internalPointer());
  }

  if (par->userData() == user_data) {
    *index = parent;
    return true;
  }

  for (size_t i = 0; i < par->childrenCount(); ++i) {
    TreeItem* child = par->child(i);
    QModelIndex ind = model->index(i, 0, parent);
    if (child->userData() == user_data) {
      *index = ind;
      return true;
    }

    bool res = findChildInModel(ind, user_data, index, root, model);
    if (res) {
      return true;
    }
  }

  return false;
}

}  // namespace

TreeModel::TreeModel(QObject* parent) : QAbstractItemModel(parent), root_(new TreeItem(nullptr, nullptr)) {}

TreeModel::~TreeModel() {
  destroy(&root_);
}

int TreeModel::rowCount(const QModelIndex& parent) const {
  const TreeItem* parent_item = nullptr;
  if (parent.isValid()) {
    parent_item = static_cast<TreeItem*>(parent.internalPointer());
  } else {
    parent_item = root_;
  }

  return parent_item ? static_cast<int>(parent_item->childrenCount()) : 0;
}

QModelIndex TreeModel::index(int row, int column, const QModelIndex& parent) const {
  if (hasIndex(row, column, parent)) {
    const TreeItem* parentItem = nullptr;
    if (!parent.isValid()) {
      parentItem = root_;
    } else {
      parentItem = static_cast<TreeItem*>(parent.internalPointer());
    }

    if (!parentItem) {
      return QModelIndex();
    }

    TreeItem* childItem = parentItem->child(static_cast<size_t>(row));
    if (childItem) {
      return createIndex(row, column, childItem);
    }
    return QModelIndex();
  }
  return QModelIndex();
}

QModelIndex TreeModel::parent(const QModelIndex& index) const {
  if (!index.isValid()) {
    return QModelIndex();
  }

  TreeItem* child_item = static_cast<TreeItem*>(index.internalPointer());
  if (!child_item) {
    return QModelIndex();
  }

  TreeItem* parent_item = child_item->parent();
  if (parent_item && parent_item != root_) {
    TreeItem* grand_parent = parent_item->parent();
    if (grand_parent) {
      int row = grand_parent->indexOf(parent_item);
      return createIndex(row, 0, parent_item);
    }
  }

  return QModelIndex();
}

TreeItem* TreeModel::root() const {
  return root_;
}

void TreeModel::setRoot(TreeItem* root) {
  beginResetModel();
  delete root_;
  root_ = root;
  endResetModel();
}

void TreeModel::insertItem(const QModelIndex& parent, TreeItem* child) {
  TreeItem* item = nullptr;
  if (!parent.isValid()) {
    item = root_;
  }

  if (!item) {
    item = static_cast<TreeItem*>(parent.internalPointer());
  }

  if (!item) {
    return;
  }

  int child_count = static_cast<int>(item->childrenCount());
  beginInsertRows(parent, child_count, child_count);
  item->addChildren(child);
  endInsertRows();
}

void TreeModel::removeItem(const QModelIndex& parent, TreeItem* child) {
  TreeItem* item = nullptr;
  if (!parent.isValid()) {
    item = root_;
  }

  if (!item) {
    item = static_cast<TreeItem*>(parent.internalPointer());
  }

  if (!item) {
    return;
  }

  // int child_count = item->childrenCount();
  int child_index = item->indexOf(child);
  beginRemoveRows(parent, child_index, child_index);
  item->removeChildren(child);
  delete child;
  endRemoveRows();
}

void TreeModel::removeAllItems(const QModelIndex& parent) {
  TreeItem* item = nullptr;
  if (!parent.isValid()) {
    item = root_;
  }

  if (!item) {
    if (!parent.internalPointer()) {
      return;
    }
    item = static_cast<TreeItem*>(parent.internalPointer());
  }

  int child_count = static_cast<int>(item->childrenCount());
  beginRemoveRows(parent, 0, child_count);
  item->clear();
  endRemoveRows();
}

void TreeModel::updateItem(const QModelIndex& top_left, const QModelIndex& bottom_right) {
  DCHECK(top_left.isValid());
  DCHECK(bottom_right.isValid());
  emit dataChanged(top_left, bottom_right);
}

bool TreeModel::findItem(void* user_data, QModelIndex* index) {
  return findChildInModel(QModelIndex(), user_data, index, root_, this);
}

}  // namespace gui
}  // namespace qt
}  // namespace common
