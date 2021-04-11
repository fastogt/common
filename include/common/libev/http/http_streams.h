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

#include <memory>

#include <common/http/http2.h>
#include <common/net/isocket.h>
#include <common/net/socket_info.h>

namespace common {
namespace libev {
namespace http {

class IStream {
 public:
  typedef uint32_t stream_id_t;

  http2::frame_t GetType() const;
  stream_id_t GetSid() const;

  bool ProcessFrame(const http2::frame_base& frame);  // true if is handled

  ErrnoError SendFrame(const http2::frame_base& frame);
  ErrnoError SendCloseFrame();

  virtual ~IStream();

  static IStream* CreateStream(net::socket_descr_t fd, const http2::frame_base& frame);

 protected:
  IStream(net::socket_descr_t fd, const http2::frame_base& frame);

  virtual bool ProcessFrameImpl(const http2::frame_base& frame) = 0;

 private:
  ErrnoError SendData(const buffer_t& buff);
  net::ISocket* sock_;
  const http2::frame_base init_frame_;
};

typedef std::shared_ptr<IStream> StreamSPtr;

class HTTP2DataStream : public IStream {
 public:
  HTTP2DataStream(net::socket_descr_t fd, const http2::frame_base& frame);

 private:
  bool ProcessFrameImpl(const http2::frame_base& frame) override;
};

typedef std::shared_ptr<HTTP2DataStream> HTTP2DataStreamSPtr;

class HTTP2PriorityStream : public IStream {
 public:
  HTTP2PriorityStream(net::socket_descr_t fd, const http2::frame_base& frame);
  ~HTTP2PriorityStream();

 private:
  bool ProcessFrameImpl(const http2::frame_base& frame) override;
};

typedef std::shared_ptr<HTTP2PriorityStream> HTTP2PriorityStreamSPtr;

class HTTP2SettingsStream : public IStream {
 public:
  HTTP2SettingsStream(net::socket_descr_t fd, const http2::frame_base& frame);
  bool IsNegotiated() const;

 private:
  bool ProcessFrameImpl(const http2::frame_base& frame) override;
  bool negotiated_;
};

typedef std::shared_ptr<HTTP2SettingsStream> HTTP2SettingsStreamSPtr;

class HTTP2HeadersStream : public IStream {
 public:
  HTTP2HeadersStream(net::socket_descr_t fd, const http2::frame_base& frame);

 private:
  bool ProcessFrameImpl(const http2::frame_base& frame) override;
};

typedef std::shared_ptr<HTTP2HeadersStream> HTTP2HeadersStreamSPtr;

}  // namespace http
}  // namespace libev
}  // namespace common
