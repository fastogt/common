// Copyright 2017 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <common/uri/url_canon.h>

namespace common {
namespace uri {

template class CanonOutputT<char>;
template class CanonOutputT<char16>;

}  // namespace uri
}  // namespace common
