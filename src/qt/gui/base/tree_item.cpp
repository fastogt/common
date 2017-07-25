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

#include <common/qt/gui/base/tree_item.h>

#include <algorithm>

#include <common/macros.h>

namespace common {
namespace qt {
namespace gui {

TreeItem::TreeItem(TreeItem* parent, void* user_data) : parent_(parent), user_data_(user_data) {}

TreeItem::~TreeItem() {
  clear();
}

void TreeItem::addChildren(TreeItem* child) {
  if (child) {
    DCHECK(child->parent_ == this);
    childrens_.push_back(child);
  }
}

void TreeItem::removeChildren(TreeItem* child) {
  childrens_.erase(std::remove(childrens_.begin(), childrens_.end(), child));
}

size_t TreeItem::childrenCount() const {
  return childrens_.size();
}

TreeItem* TreeItem::child(size_t pos) const {
  if (pos < childrens_.size()) {
    return childrens_[pos];
  }

  return NULL;
}

int TreeItem::indexOf(TreeItem* item) const {
  for (size_t i = 0; i < childrens_.size(); ++i) {
    if (item == childrens_[i]) {
      return i;
    }
  }
  return -1;
}

TreeItem* TreeItem::parent() const {
  return parent_;
}

void* TreeItem::userData() const {
  return user_data_;
}

void TreeItem::clear() {
  for (size_t i = 0; i < childrens_.size(); ++i) {
    delete childrens_[i];
  }

  childrens_.clear();
}

}  // namespace gui
}  // namespace qt
}  // namespace common
