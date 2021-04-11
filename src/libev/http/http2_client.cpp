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

#include <common/libev/http/http2_client.h>

#include <errno.h>
#include <inttypes.h>

#include <string>

#include <common/convert2string.h>
#include <common/file_system/file_system.h>
#include <common/logger.h>
#include <common/net/net.h>
#include <common/sprintf.h>

#define RFC1123FMT "%a, %d %b %Y %H:%M:%S GMT"

namespace common {
namespace {

const char* HTML_PATTERN_ISISSSS7 =
    R"(<!DOCTYPE html>
  <html>
  <head>
  <meta http-equiv="Content-type" content="text/html;charset=UTF-8">
  <title>%d %s</title>
  </head>
  <body bgcolor="#cc9999">
  <h4>%d %s</h4>%s<hr>
  <address><a href="%s">%s</a></address>
  </body>
  </html>)";

struct SendDataHelper {
  libev::http::StreamSPtr header_stream;
  uint32_t all_size;
};

ErrnoError send_data_frame(const char* buff, size_t buff_len, void* user_data, size_t* processed) {
  SendDataHelper* helper = reinterpret_cast<SendDataHelper*>(user_data);
  libev::http::StreamSPtr header_stream = helper->header_stream;

  uint8_t flags = 0;
  if (helper->all_size - buff_len == 0) {
    flags = http2::HTTP2_FLAG_END_STREAM;
  }

  http2::frame_hdr hdr = http2::frame_data::create_frame_header(flags, header_stream->GetSid(), buff_len);
  http2::frame_data fdata(hdr, buff);
  ErrnoError err = header_stream->SendFrame(fdata);
  if (err) {
    DEBUG_MSG_ERROR(err, logging::LOG_LEVEL_ERR);
    return err;
  }

  *processed = buff_len;
  helper->all_size -= buff_len;

  return ErrnoError();
}

}  // namespace
namespace libev {
namespace http {

Http2Client::Http2Client(libev::IoLoop* server, const net::socket_info& info) : HttpClient(server, info), streams_() {}

ErrnoError Http2Client::Get(const uri::GURL& url, bool is_keep_alive) {
  return SendRequest(common::http::HM_GET, url, common::http::HP_2_0, {}, is_keep_alive);
}

ErrnoError Http2Client::Head(const uri::GURL& url, bool is_keep_alive) {
  return SendRequest(common::http::HM_HEAD, url, common::http::HP_2_0, {}, is_keep_alive);
}

const char* Http2Client::ClassName() const {
  return "Http2Client";
}

bool Http2Client::IsHttp2() const {
  StreamSPtr main_stream = FindStreamByStreamID(0);
  return main_stream.get();
}

ErrnoError Http2Client::SendError(common::http::http_protocol protocol,
                                  common::http::http_status status,
                                  const common::http::headers_t& extra_headers,
                                  const char* text,
                                  bool is_keep_alive,
                                  const HttpServerInfo& info) {
  if (IsHttp2() && protocol == common::http::HP_2_0) {
    const std::string title = ConvertToString(status);
    char err_data[1024] = {0};
    off_t err_len = SNPrintf(err_data, sizeof(err_data), HTML_PATTERN_ISISSSS7, status, title, status, title, text,
                             info.server_url, info.server_name);
    ErrnoError err = SendHeaders(protocol, status, extra_headers, "text/html", &err_len, nullptr, is_keep_alive, info);
    if (err) {
      DEBUG_MSG_ERROR(err, logging::LOG_LEVEL_ERR);
      return err;
    }

    StreamSPtr header_stream = FindStreamByType(http2::HTTP2_HEADERS);
    if (!header_stream) {
      ErrnoError errr = DEBUG_MSG_PERROR("FindStreamByType", EAGAIN, logging::LOG_LEVEL_ERR);
      return errr;
    }

    http2::frame_hdr hdr =
        http2::frame_data::create_frame_header(http2::HTTP2_FLAG_END_STREAM, header_stream->GetSid(), err_len);
    http2::frame_data fdata(hdr, err_data);
    return header_stream->SendFrame(fdata);
  }

  return HttpClient::SendError(protocol, status, extra_headers, text, is_keep_alive, info);
}

ErrnoError Http2Client::SendFileByFd(common::http::http_protocol protocol, int fdesc, off_t size) {
  if (IsHttp2() && protocol == common::http::HP_2_0) {
    StreamSPtr header_stream = FindStreamByType(http2::HTTP2_HEADERS);
    if (!header_stream) {
      return DEBUG_MSG_PERROR("FindStreamByType", EAGAIN, logging::LOG_LEVEL_ERR);
    }

    SendDataHelper help;
    help.header_stream = header_stream;
    help.all_size = size;

    ErrnoError err = file_system::read_file_cb(fdesc, nullptr, size, &send_data_frame, &help);
    if (err) {
      DEBUG_MSG_ERROR(err, logging::LOG_LEVEL_ERR);
      return err;
    }

    return ErrnoError();
  }

  return HttpClient::SendFileByFd(protocol, fdesc, size);
}

ErrnoError Http2Client::SendHeaders(common::http::http_protocol protocol,
                                    common::http::http_status status,
                                    const common::http::headers_t& extra_headers,
                                    const char* mime_type,
                                    off_t* length,
                                    time_t* mod,
                                    bool is_keep_alive,
                                    const HttpServerInfo& info) {
  if (IsHttp2() && protocol == common::http::HP_2_0) {
    StreamSPtr header_stream = FindStreamByType(http2::HTTP2_HEADERS);
    if (!header_stream) {
      ErrnoError err = DEBUG_MSG_PERROR("FindStreamByType", EAGAIN, logging::LOG_LEVEL_ERR);
      return err;
    }

    http2::http2_nvs_t nvs;

    http2::http2_nv nvstatus;
    nvstatus.name = MAKE_BUFFER(":status");
    nvstatus.value = ConvertToBytes((uint32_t)status);
    nvs.push_back(nvstatus);

    char timebuf[100];
    time_t now = time(nullptr);
    strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&now));
    http2::http2_nv nvdate;
    nvdate.name = MAKE_BUFFER("date");
    nvdate.value = ConvertToBytes(std::string(timebuf));
    nvs.push_back(nvdate);

    http2::http2_nv nvserver;
    nvserver.name = MAKE_BUFFER("server");
    nvserver.value = ConvertToBytes(info.server_name);
    nvs.push_back(nvserver);

    /*http2::http2_nv nvenc;
    nvenc.name = MAKE_buffer_t("content-encoding");
    nvenc.value = MAKE_buffer_t("deflate");
    nvs.push_back(nvenc);*/

    if (mime_type) {
      http2::http2_nv nvmime;
      nvmime.name = MAKE_BUFFER("content-type");
      nvmime.value = ConvertToBytes(mime_type);
      nvs.push_back(nvmime);
    }
    if (length) {
      http2::http2_nv nvlen;
      nvlen.name = MAKE_BUFFER("content-length");
      nvlen.value = ConvertToBytes(*length);
      nvs.push_back(nvlen);
    }

    if (mod) {
      strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(mod));
      http2::http2_nv nvmod;
      nvmod.name = MAKE_BUFFER("last-modified");
      nvmod.value = ConvertToBytes(std::string(timebuf));
      nvs.push_back(nvmod);
    }

    http2::http2_deflater hd;
    buffer_t buff;
    hd.http2_deflate_hd_bufs(buff, nvs);

    http2::frame_hdr hdr =
        http2::frame_headers::create_frame_header(http2::HTTP2_FLAG_END_HEADERS, header_stream->GetSid(), buff.size());
    http2::frame_headers fhdr(hdr, buff);

    return header_stream->SendFrame(fhdr);
  }

  return HttpClient::SendHeaders(protocol, status, extra_headers, mime_type, length, mod, is_keep_alive, info);
}

ErrnoError Http2Client::SendRequest(common::http::http_method method,
                                    const uri::GURL& url,
                                    common::http::http_protocol protocol,
                                    const common::http::headers_t& extra_headers,
                                    bool is_keep_alive) {
  if (IsHttp2() && protocol == common::http::HP_2_0) {
    StreamSPtr header_stream = FindStreamByType(http2::HTTP2_HEADERS);
    if (!header_stream) {
      ErrnoError err = DEBUG_MSG_PERROR("FindStreamByType", EAGAIN, logging::LOG_LEVEL_ERR);
      return err;
    }

    http2::http2_nvs_t nvs;

    const std::string method_str = ConvertToString(method);

    http2::http2_nv nvstatus;
    nvstatus.name = MAKE_BUFFER(":method");
    nvstatus.value = ConvertToBytes(method_str);
    nvs.push_back(nvstatus);

    const std::string path = url.PathForRequest();
    http2::http2_nv nvpath;
    nvpath.name = MAKE_BUFFER(":path");
    nvpath.value = ConvertToBytes(path);
    nvs.push_back(nvpath);

    const std::string host = url.HostNoBrackets();
    http2::http2_nv nvhost;
    nvhost.name = MAKE_BUFFER(":host");
    nvhost.value = ConvertToBytes(host);
    nvs.push_back(nvhost);

    char timebuf[100];
    time_t now = time(nullptr);
    strftime(timebuf, sizeof(timebuf), RFC1123FMT, gmtime(&now));
    http2::http2_nv nvdate;
    nvdate.name = MAKE_BUFFER("date");
    nvdate.value = ConvertToBytes(std::string(timebuf));
    nvs.push_back(nvdate);

    http2::http2_deflater hd;
    buffer_t buff;
    hd.http2_deflate_hd_bufs(buff, nvs);

    http2::frame_hdr hdr =
        http2::frame_headers::create_frame_header(http2::HTTP2_FLAG_END_HEADERS, header_stream->GetSid(), buff.size());
    http2::frame_headers fhdr(hdr, buff);

    return header_stream->SendFrame(fhdr);
  }

  return HttpClient::SendRequest(method, url, protocol, extra_headers, is_keep_alive);
}

StreamSPtr Http2Client::FindStreamByStreamID(IStream::stream_id_t stream_id) const {
  for (size_t i = 0; i < streams_.size(); ++i) {
    StreamSPtr stream = streams_[i];
    if (stream->GetSid() == stream_id) {
      return stream;
    }
  }

  return StreamSPtr();
}

StreamSPtr Http2Client::FindStreamByType(http2::frame_t type) const {
  for (size_t i = 0; i < streams_.size(); ++i) {
    StreamSPtr stream = streams_[i];
    if (stream->GetType() == type) {
      return stream;
    }
  }

  return StreamSPtr();
}

bool Http2Client::IsSettingNegotiated() const {
  StreamSPtr settings = FindStreamByStreamID(0);
  if (!settings || settings->GetType() != http2::HTTP2_SETTINGS) {
    return false;
  }

  HTTP2SettingsStreamSPtr rsettings = std::static_pointer_cast<HTTP2SettingsStream>(settings);
  if (rsettings) {
    return rsettings->IsNegotiated();
  }

  return false;
}

void Http2Client::ProcessFrames(const http2::frames_t& frames) {
  const net::socket_descr_t fd = GetFd();
  for (size_t i = 0; i < frames.size(); ++i) {
    http2::frame_base frame = frames[i];
    StreamSPtr stream = FindStreamByStreamID(frame.stream_id());
    if (!stream) {
      IStream* nstream = IStream::CreateStream(fd, frame);
      stream = StreamSPtr(nstream);
      streams_.push_back(stream);
    }

    stream->ProcessFrame(frame);
  }
}

}  // namespace http
}  // namespace libev
}  // namespace common
