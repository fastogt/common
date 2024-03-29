/*  Copyright (C) 2014-2022 FastoGT. All right reserved.

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

#include <common/uri/url_constants.h>

namespace common {
namespace uri {

const char kDataScheme[] = "data";
const char kFileScheme[] = "file";
const char kDevScheme[] = "dev";
const char kUnknownScheme[] = "unknown";
const char kUdpScheme[] = "udp";
const char kRtpScheme[] = "rtp";
const char kSrtScheme[] = "srt";
const char kTcpScheme[] = "tcp";

const char kWebRTCScheme[] = "webrtc";
const char kWebRTCsScheme[] = "webrtcs";

const char kRtmpScheme[] = "rtmp";
const char kRtmpsScheme[] = "rtmps";
const char kRtmptScheme[] = "rtmpt";
const char kRtmpeScheme[] = "rtmpe";
const char kRtmfpScheme[] = "rtmfp";
const char kRtspScheme[] = "rtsp";
const char kFtpScheme[] = "ftp";
const char kGsScheme[] = "gs";
const char kS3Scheme[] = "s3";
const char kHttpScheme[] = "http";
const char kHttpsScheme[] = "https";
const char kTelScheme[] = "tel";
const char kWsScheme[] = "ws";
const char kWssScheme[] = "wss";

const char kStandardSchemeSeparator[] = "://";

const size_t kMaxURLChars = 2 * 1024 * 1024;

}  // namespace uri
}  // namespace common
