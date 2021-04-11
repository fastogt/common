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

#include <stddef.h>

#include <vector>

namespace common {
namespace qt {
namespace gui {

class TreeItem {
 public:
  typedef std::vector<TreeItem*> child_container_t;
  explicit TreeItem(TreeItem* parent, void* user_data);
  virtual ~TreeItem();

  void addChildren(TreeItem* child);
  void removeChildren(TreeItem* child);

  size_t childrenCount() const;
  TreeItem* child(size_t pos) const;
  int indexOf(TreeItem* item) const;
  TreeItem* parent() const;

  void* userData() const;
  void clear();

 protected:
  TreeItem* const parent_;

 private:
  child_container_t childrens_;
  void* const user_data_;
};

template <typename UnaryPredicate>
TreeItem* findItemRecursive(const TreeItem* parent, UnaryPredicate p) {
  if (!parent) {
    return nullptr;
  }

  size_t sz = parent->childrenCount();
  for (size_t i = 0; i < sz; ++i) {
    TreeItem* item = parent->child(i);
    if (!item) {
      continue;
    }

    if (p(item)) {
      return item;
    }

    TreeItem* rt = findItemRecursive(item, p);
    if (rt) {
      return rt;
    }
  }

  return nullptr;
}

template <typename UnaryPredicate>
TreeItem* findItemRecursive(TreeItem* parent, UnaryPredicate p) {
  if (!parent) {
    return nullptr;
  }

  size_t sz = parent->childrenCount();
  for (size_t i = 0; i < sz; ++i) {
    TreeItem* item = parent->child(i);
    if (!item) {
      continue;
    }

    if (p(item)) {
      return item;
    }

    TreeItem* rt = findItemRecursive(item, p);
    if (rt) {
      return rt;
    }
  }

  return nullptr;
}

template <typename UnaryPredicate>
UnaryPredicate forEachRecursive(const TreeItem* parent, UnaryPredicate p) {
  if (!parent) {
    return p;
  }

  size_t sz = parent->childrenCount();
  for (size_t i = 0; i < sz; ++i) {
    TreeItem* item = parent->child(i);
    if (!item) {
      continue;
    }

    forEachRecursive(item, p);
  }

  p(parent);
  return p;
}

template <typename UnaryPredicate>
UnaryPredicate forEachRecursive(TreeItem* parent, UnaryPredicate p) {
  if (!parent) {
    return p;
  }

  size_t sz = parent->childrenCount();
  for (size_t i = 0; i < sz; ++i) {
    TreeItem* item = parent->child(i);
    if (!item) {
      continue;
    }

    forEachRecursive(item, p);
  }

  p(parent);
  return p;
}

}  // namespace gui
}  // namespace qt
}  // namespace common
