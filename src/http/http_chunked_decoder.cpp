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

#include <common/http/http_chunked_decoder.h>

#include <string.h>

#include <algorithm>

#include <common/string_number_conversions.h>

namespace common {
namespace http {

// Absurdly long size to avoid imposing a constraint on chunked encoding
// extensions.
const size_t HttpChunkedDecoder::kMaxLineBufLen = 16384;

HttpChunkedDecoder::HttpChunkedDecoder()
    : chunk_remaining_(0),
      chunk_terminator_remaining_(false),
      reached_last_chunk_(false),
      reached_eof_(false),
      bytes_after_eof_(0) {}

Error HttpChunkedDecoder::FilterBuf(char* buf, int buf_len, int* out) {
  if (!out) {
    return common::make_error_inval();
  }

  int result = 0;

  while (buf_len > 0) {
    if (chunk_remaining_ > 0) {
      // Since |chunk_remaining_| is positive and |buf_len| an int, the minimum
      // of the two must be an int.
      int num = static_cast<int>(std::min(chunk_remaining_, static_cast<int64_t>(buf_len)));

      buf_len -= num;
      chunk_remaining_ -= num;

      result += num;
      buf += num;

      // After each chunk's data there should be a CRLF.
      if (chunk_remaining_ == 0)
        chunk_terminator_remaining_ = true;
      continue;
    } else if (reached_eof_) {
      bytes_after_eof_ += buf_len;
      break;  // Done!
    }

    int bytes_consumed;
    Error err = ScanForChunkRemaining(buf, buf_len, &bytes_consumed);
    if (err)
      return err;  // Error

    buf_len -= bytes_consumed;
    if (buf_len > 0)
      memmove(buf, buf + bytes_consumed, buf_len);
  }

  *out = result;
  return common::Error();
}

Error HttpChunkedDecoder::ScanForChunkRemaining(const char* buf, int buf_len, int* out) {
  DCHECK_EQ(0, chunk_remaining_);
  DCHECK_GT(buf_len, 0);

  int bytes_consumed = 0;

  size_t index_of_lf = StringPiece(buf, buf_len).find('\n');
  if (index_of_lf != StringPiece::npos) {
    buf_len = static_cast<int>(index_of_lf);
    if (buf_len && buf[buf_len - 1] == '\r')  // Eliminate a preceding CR.
      buf_len--;
    bytes_consumed = static_cast<int>(index_of_lf) + 1;

    // Make buf point to the full line buffer to parse.
    if (!line_buf_.empty()) {
      line_buf_.append(buf, buf_len);
      buf = line_buf_.data();
      buf_len = static_cast<int>(line_buf_.size());
    }

    if (reached_last_chunk_) {
      if (buf_len > 0) {
      } else {
        reached_eof_ = true;
      }
    } else if (chunk_terminator_remaining_) {
      if (buf_len > 0) {
        return common::make_error_inval();
      }
      chunk_terminator_remaining_ = false;
    } else if (buf_len > 0) {
      // Ignore any chunk-extensions.
      size_t index_of_semicolon = StringPiece(buf, buf_len).find(';');
      if (index_of_semicolon != StringPiece::npos)
        buf_len = static_cast<int>(index_of_semicolon);

      if (!ParseChunkSize(buf, buf_len, &chunk_remaining_)) {
        return common::make_error_inval();
      }

      if (chunk_remaining_ == 0)
        reached_last_chunk_ = true;
    } else {
      return common::make_error_inval();
    }
    line_buf_.clear();
  } else {
    // Save the partial line; wait for more data.
    bytes_consumed = buf_len;

    // Ignore a trailing CR
    if (buf[buf_len - 1] == '\r')
      buf_len--;

    if (line_buf_.length() + buf_len > kMaxLineBufLen) {
      return common::make_error_inval();
    }

    line_buf_.append(buf, buf_len);
  }

  *out = bytes_consumed;
  return common::Error();
}

// While the HTTP 1.1 specification defines chunk-size as 1*HEX
// some sites rely on more lenient parsing.
// http://www.yahoo.com/, for example, pads chunk-size with trailing spaces
// (0x20) to be 7 characters long, such as "819b   ".
//
// A comparison of browsers running on WindowsXP shows that
// they will parse the following inputs (egrep syntax):
//
// Let \X be the character class for a hex digit: [0-9a-fA-F]
//
//   RFC 7230: ^\X+$
//        IE7: ^\X+[^\X]*$
// Safari 3.1: ^[\t\r ]*\X+[\t ]*$
//  Firefox 3: ^[\t\f\v\r ]*[+]?(0x)?\X+[^\X]*$
// Opera 9.51: ^[\t\f\v ]*[+]?(0x)?\X+[^\X]*$
//
// Our strategy is to be as strict as possible, while not breaking
// known sites.
//
//         Us: ^\X+[ ]*$
bool HttpChunkedDecoder::ParseChunkSize(const char* start, int len, int64_t* out) {
  DCHECK_GE(len, 0);

  // Strip trailing spaces
  while (len > 0 && start[len - 1] == ' ')
    len--;

  // Be more restrictive than HexStringToInt64;
  // don't allow inputs with leading "-", "+", "0x", "0X"
  StringPiece chunk_size(start, len);
  if (chunk_size.find_first_not_of("0123456789abcdefABCDEF") != StringPiece::npos) {
    return false;
  }

  int64_t parsed_number;
  bool ok = HexStringToInt64(chunk_size, &parsed_number);
  if (ok && parsed_number >= 0) {
    *out = parsed_number;
    return true;
  }
  return false;
}

}  // namespace http
}  // namespace common
