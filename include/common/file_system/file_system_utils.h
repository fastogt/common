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

#include <functional>
#include <string>
#include <vector>

#include <common/file_system/path.h>

namespace common {
namespace file_system {

template <typename CharT, typename Traits>
std::vector<FileStringPath<CharT, Traits>> ScanFolder(
    const DirectoryStringPath<CharT, Traits>& folder,
    const std::basic_string<CharT, Traits>& pattern,
    bool recursive,
    std::function<bool(const FileStringPath<CharT, Traits>&)> filter_predicate =
        std::function<bool(const FileStringPath<CharT, Traits>&)>());

template <typename CharT, typename Traits>
std::vector<FileStringPath<CharT, Traits>> ScanFolder(
    const DirectoryStringPath<CharT, Traits>& folder,
    const CharT* pattern,
    bool recursive,
    std::function<bool(const FileStringPath<CharT, Traits>&)> filter_predicate =
        std::function<bool(const FileStringPath<CharT, Traits>&)>()) {
  return ScanFolder(folder, std::basic_string<CharT, Traits>(pattern), recursive, filter_predicate);
}

}  // namespace file_system
}  // namespace common
