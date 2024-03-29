/*  Copyright (C) 2014-2020 FastoGT. All right reserved.

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

#include <common/string_split.h>
#include <common/string_util.h>
#include <common/uri/gurl.h>
#include <gtest/gtest.h>

#define HTTP_PATH "/home/index.html"
#define FILE_PATH "/home/sasha/1.mp4"
#define DEV_VIDEO_PATH "/dev/video3"
#define SCREEN_PATH "screen"
#define WEBRTC_LINK "webrtc://239.0.3.3:3003/12345"
#define WEBRTCS_LINK                   \
  "webrtcs://live-api-s.facebook.com/" \
  "1696076320540361?s_bl=1&s_psm=1&s_sc=1696076367207023&s_sw=0&s_vt=api-s&a=AbxU0Q-pRKZw0-0r"
#define UDP_LINK "udp://239.0.3.3:3003"
#define UDP_LINK_QUERY "udp://224.96.9.196:2777?localaddr=192.168.40.10"
#define RTP_LINK "rtp://0.0.0.0:5555"
#define SRT_LINK "srt://239.0.3.3:3003"
#define SRT_LINK2 "srt://:7001"
#define TCP_LINK "tcp://google.com:2121"
#define RTMP_LINK "rtmp://192.168.1.105:5423/live"
#define RTMP_LINK_DEFAULT "rtmp://a.rtmp.youtube.com/live2"
#define RTMPS_LINK                            \
  "rtmps://live-api-s.facebook.com:443/rtmp/" \
  "1696076320540361?s_bl=1&s_psm=1&s_sc=1696076367207023&s_sw=0&s_vt=api-s&a=AbxU0Q-pRKZw0-0r"
#define RTMPT_LINK "rtmpt://192.168.1.105:5423/live"
#define RTMPE_LINK "rtmpe://192.168.1.105:5423/live"
#define RTMFP_LINK "rtmfp://192.168.1.105:5423/live"
#define RTSP_LINK "rtsp://192.168.1.210:555/Streaming/Channels/101"
#define RTSP_LINK_USER "rtsp://root:password@192.168.1.111/axis-media/media.amp?videocodec=h264&audiocodec=aac"

#define HTTP_COMMON_LINK "http://www.permadi.com/index.html"
#define HTTP_PORT_LINK "http://111.119.160.90:81/fastocloud/hls/2/5ebeba5ba2ffe6cd5d4488d3/0/master.m3u8"
#define HTTP_LONG_LINK "http://www.permadi.com/tutorial/urlEncoding/index.html"
#define HTTPS_LINK "https://github.com/fastogt/fastotvlite_mobile"
#define HTTP_QUERY_LINK "http://www.permadi.com/tutorial/urlEncoding/example.html?var=This+is+a+simple+%26+short+test"

#define SRT_WITH_QUERY "srt://196.202.177.146:9710?streamid=broadcast/golive/push"
#define SRT_TEMPLATE "srt://161.97.174.127:9998?streamid=#!::r=fastocloud"

#define AUDIO_QUERY "audio=default"
#define DEV_VIDEO_AUDIO_PATH DEV_VIDEO_PATH "?" AUDIO_QUERY

#define UNKNOWN_VIDEO_AUDIO_PATH SCREEN_PATH "?" AUDIO_QUERY

#define MDP_URL "http://41.76.110.249:200/index.mpd?id=1"

#define GS_URL "gs://some-test-41728.appspot.com/reqc/00oobdpgSivUgfFo4EPG-1616700089.webp"
#define S3_URL "s3://us-west-1/example-bucket/my/file.ogv?version=my-optional-version"

TEST(Url, IsValid) {
  common::uri::GURL s3(S3_URL);
  ASSERT_TRUE(s3.is_valid());
  ASSERT_TRUE(s3.SchemeIsS3());
  ASSERT_EQ(s3.path(), "/example-bucket/my/file.ogv");
  ASSERT_EQ(s3.host(), "us-west-1");
  ASSERT_EQ(s3.spec(), S3_URL);
  ASSERT_EQ(s3.query(), "version=my-optional-version");
  ASSERT_EQ(s3.ExtractFileName(), "file.ogv");

  common::uri::GURL gs(GS_URL);
  ASSERT_TRUE(gs.is_valid());
  ASSERT_TRUE(gs.SchemeIsGs());
  ASSERT_EQ(gs.path(), "/reqc/00oobdpgSivUgfFo4EPG-1616700089.webp");
  ASSERT_EQ(gs.host(), "some-test-41728.appspot.com");
  ASSERT_EQ(gs.spec(), GS_URL);
  ASSERT_EQ(gs.ExtractFileName(), "00oobdpgSivUgfFo4EPG-1616700089.webp");

  common::uri::GURL mdp(MDP_URL);
  auto req = mdp.path();
  ASSERT_TRUE(common::EndsWith(req, "mpd", false));

  common::uri::GURL invalid;
  ASSERT_FALSE(invalid.is_valid());

  common::uri::GURL http_common(HTTP_COMMON_LINK);
  ASSERT_TRUE(http_common.is_valid());
  ASSERT_TRUE(http_common.SchemeIsHTTPOrHTTPS());
  ASSERT_EQ(http_common.EffectiveIntPort(), 80);
  ASSERT_EQ(http_common.IntPort(), common::uri::PORT_UNSPECIFIED);
  ASSERT_EQ(http_common.spec(), HTTP_COMMON_LINK);

  common::uri::GURL http_port(HTTP_PORT_LINK);
  ASSERT_TRUE(http_port.is_valid());
  ASSERT_TRUE(http_port.SchemeIsHTTPOrHTTPS());
  ASSERT_EQ(http_port.port(), "81");
  ASSERT_EQ(http_port.IntPort(), 81);
  ASSERT_EQ(http_port.EffectiveIntPort(), 81);
  ASSERT_EQ(http_port.spec(), HTTP_PORT_LINK);

  common::uri::GURL http_long(HTTP_LONG_LINK);
  ASSERT_TRUE(http_long.is_valid());
  ASSERT_TRUE(http_long.SchemeIsHTTPOrHTTPS());
  ASSERT_EQ(http_long.spec(), HTTP_LONG_LINK);

  common::uri::GURL https(HTTPS_LINK);
  ASSERT_TRUE(https.is_valid());
  ASSERT_TRUE(https.SchemeIsHTTPOrHTTPS());
  ASSERT_EQ(https.spec(), HTTPS_LINK);

  common::uri::GURL http_query(HTTP_QUERY_LINK);
  ASSERT_TRUE(http_query.is_valid());
  ASSERT_TRUE(http_query.SchemeIsHTTPOrHTTPS());
  ASSERT_EQ(http_query.spec(), HTTP_QUERY_LINK);
  ASSERT_EQ(http_query.ExtractFileName(), "example.html");
  ASSERT_EQ(http_query.path(), "/tutorial/urlEncoding/example.html");

  const std::string originFile = "file://" + std::string(FILE_PATH);
  common::uri::GURL path5(originFile);
  ASSERT_TRUE(path5.is_valid());
  ASSERT_TRUE(path5.SchemeIsFile());
  ASSERT_EQ(FILE_PATH, path5.path());
  ASSERT_EQ(originFile, path5.spec());

  const std::string templateFile = "file:///home/sasha/object_%m_%d_%Y_%H:%M:%S.mp4";
  time_t now = ::time(nullptr);
  char timebuf[1024];
  strftime(timebuf, sizeof(timebuf), templateFile.c_str(), gmtime(&now));
  common::uri::GURL path51(templateFile);
  ASSERT_TRUE(path51.is_valid());
  ASSERT_TRUE(path51.SchemeIsFile());
  ASSERT_EQ(originFile, path5.spec());

  const std::string originDev = "dev://" + std::string(DEV_VIDEO_PATH);
  common::uri::GURL path6(originDev);
  ASSERT_TRUE(path6.is_valid());
  ASSERT_TRUE(path6.SchemeIsDev());
  ASSERT_EQ(DEV_VIDEO_PATH, path6.path());
  ASSERT_EQ(originDev, path6.spec());

  const std::string originDevAudio = "dev://" + std::string(DEV_VIDEO_AUDIO_PATH);
  common::uri::GURL pathdev(originDevAudio);
  ASSERT_TRUE(pathdev.is_valid());
  ASSERT_TRUE(pathdev.SchemeIsDev());
  ASSERT_EQ(DEV_VIDEO_PATH, pathdev.path());
  ASSERT_EQ(pathdev.query(), AUDIO_QUERY);
  ASSERT_EQ(originDevAudio, pathdev.spec());

  const std::string unknownScreen = "unknown://" + std::string(SCREEN_PATH);
  common::uri::GURL path7(unknownScreen);
  ASSERT_TRUE(path7.is_valid());
  ASSERT_TRUE(path7.SchemeIsUnknown());
  ASSERT_EQ(SCREEN_PATH, path7.path());
  ASSERT_EQ(unknownScreen, path7.spec());

  const std::string unknownAudio = "unknown://" + std::string(UNKNOWN_VIDEO_AUDIO_PATH);
  common::uri::GURL pathunk(unknownAudio);
  ASSERT_TRUE(pathunk.is_valid());
  ASSERT_TRUE(pathunk.SchemeIsUnknown());
  ASSERT_EQ(SCREEN_PATH, pathunk.path());
  ASSERT_EQ(pathunk.query(), AUDIO_QUERY);
  ASSERT_EQ(unknownAudio, pathunk.spec());

  common::uri::GURL http_path(HTTP_PATH);
  ASSERT_FALSE(http_path.is_valid());

  common::uri::GURL webrtc(WEBRTC_LINK);
  ASSERT_TRUE(webrtc.is_valid());
  ASSERT_TRUE(webrtc.SchemeIsWebRTC());
  ASSERT_EQ(webrtc.host(), "239.0.3.3");
  ASSERT_EQ(webrtc.port(), "3003");
  ASSERT_EQ(webrtc.spec(), WEBRTC_LINK);

  common::uri::GURL webrtcs(WEBRTCS_LINK);
  ASSERT_TRUE(webrtcs.is_valid());
  ASSERT_TRUE(webrtcs.SchemeIsWebRTCBased());
  ASSERT_EQ(webrtcs.host(), "live-api-s.facebook.com");
  ASSERT_EQ(webrtcs.EffectiveIntPort(), 443);
  ASSERT_EQ(webrtcs.path(), "/1696076320540361");
  ASSERT_EQ(webrtcs.spec(), WEBRTCS_LINK);

  common::uri::GURL udp(UDP_LINK);
  ASSERT_TRUE(udp.is_valid());
  ASSERT_TRUE(udp.SchemeIsUdp());
  ASSERT_EQ(udp.host(), "239.0.3.3");
  ASSERT_EQ(udp.port(), "3003");
  ASSERT_EQ(udp.spec(), UDP_LINK);

  common::uri::GURL udpq(UDP_LINK_QUERY);
  ASSERT_TRUE(udpq.is_valid());
  ASSERT_TRUE(udpq.SchemeIsUdp());
  ASSERT_EQ(udpq.host(), "224.96.9.196");
  ASSERT_EQ(udpq.port(), "2777");
  ASSERT_EQ(udpq.spec(), UDP_LINK_QUERY);
  ASSERT_EQ(udpq.query(), "localaddr=192.168.40.10");

  common::uri::GURL rtp(RTP_LINK);
  ASSERT_TRUE(rtp.is_valid());
  ASSERT_TRUE(rtp.SchemeIsRtp());
  ASSERT_EQ(rtp.host(), "0.0.0.0");
  ASSERT_EQ(rtp.port(), "5555");
  ASSERT_EQ(rtp.spec(), RTP_LINK);

  common::uri::GURL srt(SRT_LINK);
  ASSERT_TRUE(srt.is_valid());
  ASSERT_TRUE(srt.SchemeIsSrt());
  ASSERT_EQ(srt.host(), "239.0.3.3");
  ASSERT_EQ(srt.port(), "3003");
  ASSERT_EQ(srt.spec(), SRT_LINK);

  common::uri::GURL srt_query(SRT_LINK2);
  ASSERT_TRUE(srt_query.is_valid());
  ASSERT_TRUE(srt_query.SchemeIsSrt());
  ASSERT_EQ(srt_query.host(), "");
  ASSERT_EQ(srt_query.port(), "7001");
  ASSERT_EQ(srt_query.spec(), SRT_LINK2);

  common::uri::GURL srt_query_streamid(SRT_WITH_QUERY);
  ASSERT_TRUE(srt_query_streamid.is_valid());
  ASSERT_TRUE(srt_query_streamid.SchemeIsSrt());
  ASSERT_EQ(srt_query_streamid.host(), "196.202.177.146");
  ASSERT_EQ(srt_query_streamid.port(), "9710");
  ASSERT_EQ(srt_query_streamid.GetOrigin(), "srt://196.202.177.146:9710/");
  ASSERT_EQ(srt_query_streamid.query(), "streamid=broadcast/golive/push");
  ASSERT_EQ(srt_query_streamid.spec(), SRT_WITH_QUERY);

  common::uri::GURL srt_template(SRT_TEMPLATE);
  ASSERT_TRUE(srt_template.is_valid());
  ASSERT_TRUE(srt_template.SchemeIsSrt());
  ASSERT_EQ(srt_template.host(), "161.97.174.127");
  ASSERT_EQ(srt_template.port(), "9998");
  ASSERT_EQ(srt_template.GetOrigin(), "srt://161.97.174.127:9998/");
  ASSERT_EQ(srt_template.query(), "streamid=");
  ASSERT_EQ(srt_template.ref(), "!::r=fastocloud");
  ASSERT_EQ(srt_template.spec(), SRT_TEMPLATE);

  common::uri::GURL tcp(TCP_LINK);
  ASSERT_TRUE(tcp.is_valid());
  ASSERT_TRUE(tcp.SchemeIsTcp());
  ASSERT_EQ(tcp.host(), "google.com");
  ASSERT_EQ(tcp.port(), "2121");
  ASSERT_EQ(tcp.spec(), TCP_LINK);

  common::uri::GURL rtmp(RTMP_LINK);
  ASSERT_TRUE(rtmp.is_valid());
  ASSERT_TRUE(rtmp.SchemeIsRtmp());
  ASSERT_EQ(rtmp.host(), "192.168.1.105");
  ASSERT_EQ(rtmp.port(), "5423");
  ASSERT_EQ(rtmp.path(), "/live");
  ASSERT_EQ(rtmp.spec(), RTMP_LINK);

  common::uri::GURL rtmp_default(RTMP_LINK_DEFAULT);
  ASSERT_TRUE(rtmp_default.is_valid());
  ASSERT_TRUE(rtmp_default.SchemeIsRtmp());
  ASSERT_EQ(rtmp_default.host(), "a.rtmp.youtube.com");
  ASSERT_EQ(rtmp_default.EffectiveIntPort(), 1935);
  ASSERT_EQ(rtmp_default.path(), "/live2");
  ASSERT_EQ(rtmp_default.spec(), RTMP_LINK_DEFAULT);

  common::uri::GURL rtmps(RTMPS_LINK);
  ASSERT_TRUE(rtmps.is_valid());
  ASSERT_TRUE(rtmps.SchemeIsRtmpBased());
  ASSERT_EQ(rtmps.host(), "live-api-s.facebook.com");
  ASSERT_EQ(rtmps.port(), "443");
  ASSERT_EQ(rtmps.path(), "/rtmp/1696076320540361");
  ASSERT_EQ(rtmps.spec(), RTMPS_LINK);

  common::uri::GURL rtmpt(RTMPT_LINK);
  ASSERT_TRUE(rtmpt.is_valid());
  ASSERT_TRUE(rtmpt.SchemeIsRtmpBased());
  ASSERT_EQ(rtmpt.host(), "192.168.1.105");
  ASSERT_EQ(rtmpt.port(), "5423");
  ASSERT_EQ(rtmpt.path(), "/live");
  ASSERT_EQ(rtmpt.spec(), RTMPT_LINK);

  common::uri::GURL rtmpe(RTMPE_LINK);
  ASSERT_TRUE(rtmpe.is_valid());
  ASSERT_TRUE(rtmpe.SchemeIsRtmpBased());
  ASSERT_EQ(rtmpe.host(), "192.168.1.105");
  ASSERT_EQ(rtmpe.port(), "5423");
  ASSERT_EQ(rtmpe.path(), "/live");
  ASSERT_EQ(rtmpe.spec(), RTMPE_LINK);

  common::uri::GURL rtmfp(RTMFP_LINK);
  ASSERT_TRUE(rtmfp.is_valid());
  ASSERT_TRUE(rtmfp.SchemeIsRtmpBased());
  ASSERT_EQ(rtmfp.host(), "192.168.1.105");
  ASSERT_EQ(rtmfp.port(), "5423");
  ASSERT_EQ(rtmfp.path(), "/live");
  ASSERT_EQ(rtmfp.spec(), RTMFP_LINK);

  common::uri::GURL rtsp(RTSP_LINK);
  ASSERT_TRUE(rtsp.is_valid());
  ASSERT_TRUE(rtsp.SchemeIsRtsp());
  ASSERT_EQ(rtsp.host(), "192.168.1.210");
  ASSERT_EQ(rtsp.port(), "555");
  ASSERT_EQ(rtsp.path(), "/Streaming/Channels/101");
  ASSERT_EQ(rtsp.spec(), RTSP_LINK);

  common::uri::GURL rtsp_user(RTSP_LINK_USER);
  ASSERT_TRUE(rtsp_user.is_valid());
  ASSERT_TRUE(rtsp_user.SchemeIsRtsp());
  ASSERT_EQ(rtsp_user.host(), "192.168.1.111");
  ASSERT_EQ(rtsp_user.EffectiveIntPort(), 554);
  ASSERT_EQ(rtsp_user.path(), "/axis-media/media.amp");
  ASSERT_EQ(rtsp_user.query(), "videocodec=h264&audiocodec=aac");
  ASSERT_EQ(rtsp_user.spec(), RTSP_LINK_USER);

  common::uri::GURL post("http://panel.fastotv.com:8083/panel_pro/api/load_balance/log/5ec602f6d392b6b89f1814f7");
  ASSERT_EQ(post.host(), "panel.fastotv.com");
  ASSERT_EQ(post.EffectiveIntPort(), 8083);

  common::uri::GURL dvb("dvb://?modulation=\"QAM 64\"&trans-mode=8k&bandwidth=8&frequency=514000000");
  ASSERT_TRUE(dvb.SchemeIs("dvb"));
  const auto spl = common::SplitString(dvb.query(), "&", common::TRIM_WHITESPACE, common::SPLIT_WANT_ALL);
  ASSERT_EQ(spl.size(), 4);
  ASSERT_EQ(dvb.query(), "modulation=\"QAM 64\"&trans-mode=8k&bandwidth=8&frequency=514000000");
}
