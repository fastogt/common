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

#include <common/libev/http/http_streams.h>

#include <common/portable_endian.h>

#include <common/net/socket_tcp.h>

namespace common {
namespace libev {
namespace http {

http2::frame_t IStream::GetType() const {
  return init_frame_.type();
}

IStream::stream_id_t IStream::GetSid() const {
  return init_frame_.stream_id();
}

IStream::IStream(net::socket_descr_t fd, const http2::frame_base& frame)
    : sock_(new net::TcpSocketHolder(fd)), init_frame_(frame) {}

bool IStream::ProcessFrame(const http2::frame_base& frame) {
  if (!frame.IsValid()) {
    NOTREACHED();
    return false;
  }

  return ProcessFrameImpl(frame);
}

ErrnoError IStream::SendData(const buffer_t& buff) {
  size_t nwrite = 0;
  return sock_->Write(buff.data(), buff.size(), &nwrite);
}

ErrnoError IStream::SendFrame(const http2::frame_base& frame) {
  CHECK(GetSid() == frame.stream_id());
  buffer_t raw = frame.raw_data();
  return SendData(raw);
}

ErrnoError IStream::SendCloseFrame() {
  http2::frame_hdr hdr = http2::frame_rst::create_frame_header(0, GetSid());
  uint32_t er = be32toh(http2::HTTP2_STREAM_CLOSED);
  http2::frame_rst rst(hdr, &er);
  return SendFrame(rst);
}

IStream* IStream::CreateStream(net::socket_descr_t fd, const http2::frame_base& frame) {
  if (!frame.IsValid()) {
    NOTREACHED();
    return nullptr;
  }

  http2::frame_t type = frame.type();

  switch (type) {
    case http2::HTTP2_DATA:
      return new HTTP2DataStream(fd, frame);
    case http2::HTTP2_HEADERS:
      return new HTTP2HeadersStream(fd, frame);
    case http2::HTTP2_PRIORITY: {
      IStream* res = new HTTP2PriorityStream(fd, frame);
      return res;
    }
    case http2::HTTP2_RST_STREAM:
      NOTREACHED();
      return nullptr;
    case http2::HTTP2_SETTINGS:
      return new HTTP2SettingsStream(fd, frame);
    case http2::HTTP2_PUSH_PROMISE:
      NOTREACHED();
      return nullptr;
    case http2::HTTP2_PING:
      NOTREACHED();
      return nullptr;
    case http2::HTTP2_GOAWAY:
      NOTREACHED();
      return nullptr;
    case http2::HTTP2_WINDOW_UPDATE:
      NOTREACHED();
      return nullptr;
    case http2::HTTP2_CONTINUATION:
      NOTREACHED();
      return nullptr;

    default:
      NOTREACHED();
      return nullptr;
  }
}

IStream::~IStream() {
  destroy(&sock_);
}

HTTP2DataStream::HTTP2DataStream(net::socket_descr_t fd, const http2::frame_base& frame) : IStream(fd, frame) {
  CHECK(http2::HTTP2_DATA == frame.type());
}

bool HTTP2DataStream::ProcessFrameImpl(const http2::frame_base& frame) {
  UNUSED(frame);
  return true;
}

HTTP2PriorityStream::HTTP2PriorityStream(net::socket_descr_t fd, const http2::frame_base& frame) : IStream(fd, frame) {
  CHECK(http2::HTTP2_PRIORITY == frame.type());
}

HTTP2PriorityStream::~HTTP2PriorityStream() {}

bool HTTP2PriorityStream::ProcessFrameImpl(const http2::frame_base& frame) {
  UNUSED(frame);
  SendCloseFrame();
  return true;
}

HTTP2SettingsStream::HTTP2SettingsStream(net::socket_descr_t fd, const http2::frame_base& frame)
    : IStream(fd, frame), negotiated_(false) {
  CHECK(http2::HTTP2_SETTINGS == frame.type());
}

bool HTTP2SettingsStream::IsNegotiated() const {
  return negotiated_;
}

bool HTTP2SettingsStream::ProcessFrameImpl(const http2::frame_base& frame) {
  if (frame.type() == http2::HTTP2_SETTINGS) {
    SendFrame(frame);
    if (frame.flags() & http2::HTTP2_FLAG_ACK) {
      negotiated_ = true;
    }
  } else if (frame.type() != http2::HTTP2_GOAWAY) {
    SendCloseFrame();
  }
  return true;
}

HTTP2HeadersStream::HTTP2HeadersStream(net::socket_descr_t fd, const http2::frame_base& frame) : IStream(fd, frame) {
  CHECK(http2::HTTP2_HEADERS == frame.type());
}

bool HTTP2HeadersStream::ProcessFrameImpl(const common::http2::frame_base& frame) {
  UNUSED(frame);
  return false;
}

}  // namespace http
}  // namespace libev
}  // namespace common
