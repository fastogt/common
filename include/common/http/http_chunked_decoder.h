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
#include <stdint.h>

#include <string>

#include <common/error.h>

namespace common {
namespace http {

// From RFC2617 section 3.6.1, the chunked transfer coding is defined as:
//
//   Chunked-Body    = *chunk
//                     last-chunk
//                     trailer
//                     CRLF
//   chunk           = chunk-size [ chunk-extension ] CRLF
//                     chunk-data CRLF
//   chunk-size      = 1*HEX
//   last-chunk      = 1*("0") [ chunk-extension ] CRLF
//
//   chunk-extension = *( ";" chunk-ext-name [ "=" chunk-ext-val ] )
//   chunk-ext-name  = token
//   chunk-ext-val   = token | quoted-string
//   chunk-data      = chunk-size(OCTET)
//   trailer         = *(entity-header CRLF)
//
// The chunk-size field is a string of hex digits indicating the size of the
// chunk.  The chunked encoding is ended by any chunk whose size is zero,
// followed by the trailer, which is terminated by an empty line.
//
// NOTE: This implementation does not bother to parse trailers since they are
// not used on the web.
//
class HttpChunkedDecoder {
 public:
  // The maximum length of |line_buf_| between calls to FilterBuff().
  // Exposed for tests.
  static const size_t kMaxLineBufLen;

  HttpChunkedDecoder();

  // Indicates that a previous call to FilterBuf encountered the final CRLF.
  bool reached_eof() const { return reached_eof_; }

  // Returns the number of bytes after the final CRLF.
  int bytes_after_eof() const { return bytes_after_eof_; }

  // Called to filter out the chunk markers from buf and to check for end-of-
  // file.  This method modifies |buf| inline if necessary to remove chunk
  // markers.  The return value indicates the final size of decoded data stored
  // in |buf|.  Call reached_eof() after this method to check if end-of-file
  // was encountered.
  Error FilterBuf(char* buf, int buf_len, int* out);

 private:
  // Scans |buf| for the next chunk delimiter.  This method returns the number
  // of bytes consumed from |buf|.  If found, |chunk_remaining_| holds the
  // value for the next chunk size.
  Error ScanForChunkRemaining(const char* buf, int buf_len, int* out);

  // Converts string |start| of length |len| to a numeric value.
  // |start| is a string of type "chunk-size" (hex string).
  // If the conversion succeeds, returns true and places the result in |out|.
  static bool ParseChunkSize(const char* start, int len, int64_t* out);

  // Indicates the number of bytes remaining for the current chunk.
  int64_t chunk_remaining_;

  // A small buffer used to store a partial chunk marker.
  std::string line_buf_;

  // True if waiting for the terminal CRLF of a chunk's data.
  bool chunk_terminator_remaining_;

  // Set to true when FilterBuf encounters the last-chunk.
  bool reached_last_chunk_;

  // Set to true when FilterBuf encounters the final CRLF.
  bool reached_eof_;

  // The number of extraneous unfiltered bytes after the final CRLF.
  int bytes_after_eof_;
};

}  // namespace http
}  // namespace common
