#include <gtest/gtest.h>

#include <common/http/http2.h>
#include <common/net/http_client.h>

using namespace common;

TEST(Http, parse) {
  http::HttpRequest r1;
  std::pair<http::http_status, Error> err = http::parse_http_request(std::string(), &r1);
  ASSERT_TRUE(err.second);
}

TEST(Http, parse_GET) {
  http::HttpRequest r1;
  const std::string request =
      "GET /path/file.html HTTP/1.0\r\nFrom: someuser@jmarshall.com\r\n User-Agent: "
      "HTTPTool/1.0\r\n\r\n";
  std::pair<http::http_status, Error> err = http::parse_http_request(request, &r1);
  ASSERT_EQ(r1.GetMethod(), http::HM_GET);
  ASSERT_EQ(r1.GetPath().GetPath(), "path/file.html");
  ASSERT_TRUE(r1.GetHeaders().size() == 2);
  ASSERT_TRUE(r1.GetBody().empty());
  ASSERT_FALSE(err.second);

  http::HttpRequest r2;
  const std::string request2 =
      "GET /hello.htm HTTP/1.1\r\n"
      "User-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)\r\n"
      "Host: www.tutorialspoint.com\r\n"
      "Accept-Language: en-us\r\n"
      "Accept-Encoding: gzip, deflate\r\n"
      "Connection: Keep-Alive\r\n\r\n";
  err = http::parse_http_request(request2, &r2);
  ASSERT_EQ(r2.GetMethod(), http::HM_GET);
  ASSERT_EQ(r2.GetPath().GetPath(), "hello.htm");
  ASSERT_TRUE(r2.GetHeaders().size() == 5);
  ASSERT_TRUE(r2.GetBody().empty());
  ASSERT_FALSE(err.second);

  http::HttpRequest r4;
  const std::string request4 =
      "GET /hello.htm?home=Cosby&favorite+flavor=flies HTTP/1.1\r\n"
      "User-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)\r\n"
      "Host: www.tutorialspoint.com\r\n"
      "Accept-Language: en-us\r\n"
      "Accept-Encoding: gzip, deflate\r\n"
      "Connection: Keep-Alive\r\n\r\n";
  err = http::parse_http_request(request4, &r4);
  ASSERT_EQ(r4.GetMethod(), http::HM_GET);
  ASSERT_EQ(r4.GetPath().GetPath(), "hello.htm");
  // ASSERT_EQ(r4.path().Query(), "home=Cosby&favorite flavor=flies");
  ASSERT_TRUE(r4.GetHeaders().size() == 5);
  ASSERT_TRUE(r4.GetBody().empty());
  ASSERT_FALSE(err.second);

  http::HttpRequest r5;
  const std::string request5 =
      "GET /[object LocalMediaStream] HTTP/1.1\r\n"
      "Host:localhost:8080\r\n"
      "User-Agent:Mozilla/5.0 (Windows NT 6.1; WOW64; rv:43.0) Gecko/20100101 Firefox/43.0\r\n"
      "Accept:video/webm,video/ogg,video/*;q=0.9,application/ogg;q=0.7,audio/*;q=0.6,*/*;q=0.5\r\n"
      "Accept-Language:en-US,en;q=0.5\r\n"
      "Range:bytes=0-\r\n"
      "Referer:http://localhost:8080/\r\n"
      "Connection:keep-alive\r\n\r\n";

  err = http::parse_http_request(request5, &r5);
  ASSERT_FALSE(err.second);
  ASSERT_EQ(r5.GetMethod(), http::HM_GET);
  ASSERT_EQ(r5.GetPath().GetPath(), "[object LocalMediaStream]");
  ASSERT_TRUE(r5.GetProtocol() == http::HP_1_1);
  ASSERT_TRUE(r5.GetHeaders().size() == 7);
  ASSERT_TRUE(r5.GetBody().empty());
}

TEST(Http, parse_HEAD) {
  http::HttpRequest r3;
  const std::string request3 =
      "HEAD /hello.htm HTTP/1.1\r\n"
      "User-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)\r\n"
      "Host: www.tutorialspoint.com\r\n"
      "Accept-Language: en-us\r\n"
      "Accept-Encoding: gzip, deflate\r\n"
      "Connection: Keep-Alive\r\n\r\n";
  std::pair<http::http_status, Error> err = http::parse_http_request(request3, &r3);
  ASSERT_EQ(r3.GetMethod(), http::HM_HEAD);
  ASSERT_EQ(r3.GetPath().GetPath(), "hello.htm");
  ASSERT_TRUE(r3.GetHeaders().size() == 5);
  ASSERT_TRUE(r3.GetBody().empty());
  ASSERT_FALSE(err.second);
}

TEST(Http, parse_POST) {
  http::HttpRequest r3;
  const std::string request3 =
      "POST /hello.htm HTTP/1.1\r\n"
      "User-Agent: Mozilla/4.0 (compatible; MSIE5.01; Windows NT)\r\n"
      "Host: www.tutorialspoint.com\r\n"
      "Accept-Language: en-us\r\n"
      "Accept-Encoding: gzip, deflate\r\n"
      "Content-Length: 32\r\n\r\n"
      "home=Cosby&favorite+flavor=flies";
  std::pair<http::http_status, Error> err = http::parse_http_request(request3, &r3);
  ASSERT_EQ(r3.GetMethod(), http::HM_POST);
  ASSERT_EQ(r3.GetPath().GetPath(), "hello.htm");
  ASSERT_TRUE(r3.GetHeaders().size() == 5);
  ASSERT_EQ(r3.GetBody(), "home=Cosby&favorite flavor=flies");
  ASSERT_TRUE(r3.GetBody().size() == 32);
  ASSERT_FALSE(err.second);
}

void checkFrameData(const http2::frame_base& frame,
                    http2::frame_t type,
                    uint8_t flags,
                    uint32_t len,
                    uint32_t stream_id,
                    const byte_t* rawdata,
                    uint32_t rawdata_size) {
  ASSERT_TRUE(frame.type() == type);
  ASSERT_TRUE(frame.flags() == flags);
  ASSERT_TRUE(frame.payload_size() == len);
  ASSERT_TRUE(frame.stream_id() == stream_id);

  http2::frame_hdr h0local(type, flags, stream_id, len);
  ASSERT_TRUE(frame.type() == h0local.type());
  ASSERT_TRUE(frame.flags() == h0local.flags());
  ASSERT_TRUE(frame.payload_size() == h0local.length());
  ASSERT_TRUE(frame.stream_id() == h0local.stream_id());

  const uint32_t payload_size = rawdata_size - FRAME_HEADER_SIZE;
  const byte_t* payload = rawdata + FRAME_HEADER_SIZE;
  ASSERT_TRUE(frame.payload_size() == payload_size);
  ASSERT_TRUE(memcmp(frame.c_payload(), payload, payload_size) == 0);
  buffer_t rbuffer = frame.raw_data();
  const byte_t* rdata = &rbuffer[0];
  ASSERT_TRUE(memcmp(rdata, rawdata, rawdata_size) == 0);
}

TEST(Http2, parse_frames) {
  http2::frame_base fr;
  ASSERT_FALSE(fr.IsValid());

  http2::frame_hdr fh;
  ASSERT_FALSE(fh.IsValid());

  const http2::frame_hdr epr(http2::HTTP2_PRIORITY, 0, 0, 6);
  ASSERT_FALSE(epr.IsValid());

  const http2::frame_hdr epr2(http2::HTTP2_PRIORITY, 0, 0, 5);
  ASSERT_TRUE(epr2.IsValid());

  const http2::frame_hdr emfh(http2::HTTP2_SETTINGS, http2::HTTP2_FLAG_ACK, 0, 0);
  ASSERT_TRUE(emfh.IsValid());
  const http2::frame_base emptySettings = http2::frame_base::create_frame(emfh, NULL);
  ASSERT_TRUE(emptySettings.IsValid());

  ASSERT_TRUE(emptySettings.payload_size() == 0);
  ASSERT_TRUE(emptySettings.payload().empty());
  ASSERT_TRUE(emfh.flags() == http2::HTTP2_FLAG_ACK);
  ASSERT_TRUE(emptySettings.raw_data().size() == FRAME_HEADER_SIZE);

  const http2::frame_hdr invalidlenfh(http2::HTTP2_SETTINGS, http2::HTTP2_FLAG_ACK, 0, 1);
  ASSERT_FALSE(invalidlenfh.IsValid());

  const http2::frame_hdr invalidlen2fh(http2::HTTP2_SETTINGS, 0, 0, 1);
  ASSERT_FALSE(invalidlen2fh.IsValid());

  const http2::frame_hdr invalidstreamfh(http2::HTTP2_SETTINGS, http2::HTTP2_FLAG_ACK, 1, 0);
  ASSERT_FALSE(invalidstreamfh.IsValid());
  // 00000C04000000000000030000006400040000FFFF00000502000000000300000000C800000502000000000500000000640000050200000000070000000000000005
  // 020000000009000000070000000502000000000B000000030000002901250000000D0000000B0F828486418A089D5C0B8170DC780F0353032A2F2A907A8DAA69D29AC4C0576576D6BF838F
  const uint8_t raw_frame0[] = {0x00, 0x00, 0x0C, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, /*header*/
                                0x00, 0x03, 0x00, 0x00, 0x00, 0x64, 0x00, 0x04, 0x00, 0x00, 0xFF, 0xFF};
  const uint8_t raw_frame1[] = {0x00, 0x00, 0x05, 0x02, 0x00, 0x00, 0x00, 0x00, 0x03, /*header*/
                                0x00, 0x00, 0x00, 0x00, 0xC8};
  const uint8_t raw_frame2[] = {0x00, 0x00, 0x05, 0x02, 0x00, 0x00, 0x00, 0x00, 0x05, /*header*/
                                0x00, 0x00, 0x00, 0x00, 0x64};
  const uint8_t raw_frame3[] = {0x00, 0x00, 0x05, 0x02, 0x00, 0x00, 0x00, 0x00, 0x07, /*header*/
                                0x00, 0x00, 0x00, 0x00, 0x00};
  const uint8_t raw_frame4[] = {0x00, 0x00, 0x05, 0x02, 0x00, 0x00, 0x00, 0x00, 0x09, /*header*/
                                0x00, 0x00, 0x00, 0x07, 0x00};
  const uint8_t raw_frame5[] = {0x00, 0x00, 0x05, 0x02, 0x00, 0x00, 0x00, 0x00, 0x0B, /*header*/
                                0x00, 0x00, 0x00, 0x03, 0x00};
  const uint8_t raw_frame6[] = {0x00, 0x00, 0x29, 0x01, 0x25, 0x00, 0x00, 0x00, 0x0D, /*header*/
                                0x00, 0x00, 0x00, 0x0B, 0x0F, 0x82, 0x84, 0x86, 0x41, 0x8A, 0x08, 0x9D, 0x5C, 0x0B,
                                0x81, 0x70, 0xDC, 0x78, 0x0F, 0x03, 0x53, 0x03, 0x2A, 0x2F, 0x2A, 0x90, 0x7A, 0x8D,
                                0xAA, 0x69, 0xD2, 0x9A, 0xC4, 0xC0, 0x57, 0x65, 0x76, 0xD6, 0xBF, 0x83, 0x8F};

  char raw_frames[141] = {0};
  size_t len = 0;
  memcpy(raw_frames + len, raw_frame0, sizeof(raw_frame0));
  len += sizeof(raw_frame0);
  memcpy(raw_frames + len, raw_frame1, sizeof(raw_frame1));
  len += sizeof(raw_frame1);
  memcpy(raw_frames + len, raw_frame2, sizeof(raw_frame2));
  len += sizeof(raw_frame2);
  memcpy(raw_frames + len, raw_frame3, sizeof(raw_frame3));
  len += sizeof(raw_frame3);
  memcpy(raw_frames + len, raw_frame4, sizeof(raw_frame4));
  len += sizeof(raw_frame4);
  memcpy(raw_frames + len, raw_frame5, sizeof(raw_frame5));
  len += sizeof(raw_frame5);
  memcpy(raw_frames + len, raw_frame6, sizeof(raw_frame6));
  len += sizeof(raw_frame6);

  http2::frames_t frames = http2::parse_frames((const char*)raw_frames, len);
  ASSERT_TRUE(frames.size() == 7);

  // settings frame //
  checkFrameData(frames[0], http2::HTTP2_SETTINGS, 0, 12, 0, raw_frame0, sizeof(raw_frame0));
  // settings frame //

  // priority frame //
  checkFrameData(frames[1], http2::HTTP2_PRIORITY, 0, 5, 3, raw_frame1, sizeof(raw_frame1));
  checkFrameData(frames[2], http2::HTTP2_PRIORITY, 0, 5, 5, raw_frame2, sizeof(raw_frame2));
  checkFrameData(frames[3], http2::HTTP2_PRIORITY, 0, 5, 7, raw_frame3, sizeof(raw_frame3));
  checkFrameData(frames[4], http2::HTTP2_PRIORITY, 0, 5, 9, raw_frame4, sizeof(raw_frame4));
  checkFrameData(frames[5], http2::HTTP2_PRIORITY, 0, 5, 11, raw_frame5, sizeof(raw_frame5));
  // priority frame //

  // headers frame//
  checkFrameData(frames[6], http2::HTTP2_HEADERS, 0x25, 41, 13, raw_frame6, sizeof(raw_frame6));
  // headers frame//
}

TEST(Http2, frame_settings) {
  const uint8_t raw_settings_frame[] = {
      0x00, 0x00, 0x0C, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, /*header*/
      0x00, 0x03, 0x00, 0x00, 0x00,
      0x64,  // settings_id_/value
      0x00, 0x04, 0x00, 0x00, 0xFF,
      0xFF  // settings_id_/value
  };

  http2::frames_t frames = http2::parse_frames((const char*)raw_settings_frame, sizeof(raw_settings_frame));
  ASSERT_TRUE(frames.size() == 1);

  http2::frame_settings* set = (http2::frame_settings*)(&frames[0]);
  ASSERT_TRUE(set->type() == 0x04);
  ASSERT_TRUE(set->flags() == 0x00);
  ASSERT_TRUE(set->stream_id() == 0x00);
  ASSERT_TRUE(set->payload_size() == 0x0C);

  ASSERT_TRUE(set->niv() == 2);
  const http2::http2_settings_entry* setev = set->iv();
  ASSERT_TRUE(setev->settings_id() == 0x03);
  ASSERT_TRUE(setev->value() == 0x64);
  const http2::http2_settings_entry* setev2 = set->iv() + 1;
  ASSERT_TRUE(setev2->settings_id() == 0x04);
  ASSERT_TRUE(setev2->value() == 0xFFFF);
}

TEST(Http2, frame_priority) {
  // priority frame //
  const uint8_t raw_priority_frames[] = {
      0x00, 0x00, 0x05, 0x02, 0x00, 0x00, 0x00, 0x00, 0x03, /*header*/
      0x00, 0x00, 0x00,
      0x00,  // stream_id
      0xC8,  // weight

      0x00, 0x00, 0x05, 0x02, 0x00, 0x00, 0x00, 0x00, 0x05, /*header*/
      0x00, 0x00, 0x00,
      0x00,  // stream_id
      0x64,  // weight

      0x00, 0x00, 0x05, 0x02, 0x00, 0x00, 0x00, 0x00, 0x07, /*header*/
      0x00, 0x00, 0x00,
      0x00,  // stream_id
      0x00,  // weight

      0x00, 0x00, 0x05, 0x02, 0x00, 0x00, 0x00, 0x00, 0x09, /*header*/
      0x00, 0x00, 0x00,
      0x07,  // stream_id
      0x00,  // weight

      0x00, 0x00, 0x05, 0x02, 0x00, 0x00, 0x00, 0x00, 0x0B, /*header*/
      0x00, 0x00, 0x00,
      0x03,  // stream_id
      0x00   // weight
  };

  http2::frames_t frames = http2::parse_frames((const char*)raw_priority_frames, sizeof(raw_priority_frames));
  ASSERT_TRUE(frames.size() == 5);

  http2::frame_priority* pri1 = (http2::frame_priority*)(&frames[0]);
  ASSERT_TRUE(pri1->type() == 0x02);
  ASSERT_TRUE(pri1->flags() == 0x00);
  ASSERT_TRUE(pri1->stream_id() == 0x03);
  ASSERT_TRUE(pri1->payload_size() == 0x05);

  const http2::http2_priority_spec* prior1 = pri1->priority();
  ASSERT_TRUE(prior1->stream_id() == 0x00);
  ASSERT_TRUE(prior1->weight() == 0xC8 + 1);

  http2::frame_priority* pri2 = (http2::frame_priority*)(&frames[1]);
  ASSERT_TRUE(pri2->type() == 0x02);
  ASSERT_TRUE(pri2->flags() == 0x00);
  ASSERT_TRUE(pri2->stream_id() == 0x05);
  ASSERT_TRUE(pri2->payload_size() == 0x05);

  const http2::http2_priority_spec* prior2 = pri2->priority();
  ASSERT_TRUE(prior2->stream_id() == 0x00);
  ASSERT_TRUE(prior2->weight() == 0x64 + 1);

  const http2::frame_priority* pri3 = (http2::frame_priority*)(&frames[2]);
  ASSERT_TRUE(pri3->type() == 0x02);
  ASSERT_TRUE(pri3->flags() == 0x00);
  ASSERT_TRUE(pri3->stream_id() == 0x07);
  ASSERT_TRUE(pri3->payload_size() == 0x05);

  const http2::http2_priority_spec* prior3 = pri3->priority();
  ASSERT_TRUE(prior3->stream_id() == 0x00);
  ASSERT_TRUE(prior3->weight() == 0x0 + 1);

  http2::frame_priority* pri4 = (http2::frame_priority*)(&frames[3]);
  ASSERT_TRUE(pri4->type() == 0x02);
  ASSERT_TRUE(pri4->flags() == 0x00);
  ASSERT_TRUE(pri4->stream_id() == 0x09);
  ASSERT_TRUE(pri4->payload_size() == 0x05);

  const http2::http2_priority_spec* prior4 = pri4->priority();
  ASSERT_TRUE(prior4->stream_id() == 0x07);
  ASSERT_TRUE(prior4->weight() == 0x00 + 1);

  const http2::frame_priority* pri5 = (http2::frame_priority*)(&frames[4]);
  ASSERT_TRUE(pri5->type() == 0x02);
  ASSERT_TRUE(pri5->flags() == 0x00);
  ASSERT_TRUE(pri5->stream_id() == 0x0B);
  ASSERT_TRUE(pri5->payload_size() == 0x05);

  const http2::http2_priority_spec* prior5 = pri5->priority();
  ASSERT_TRUE(prior5->stream_id() == 0x03);
  ASSERT_TRUE(prior5->weight() == 0x00 + 1);
  // priority frame //
}

TEST(Http2, frame_headers) {
  // headers frame//
  const uint8_t headers_frame0[] = {0x00, 0x00, 0x29, 0x01, 0x25, 0x00, 0x00, 0x00, 0x0D, /*header*/
                                    0x00, 0x00, 0x00,
                                    0x0B,  // stream_id
                                    0x0F,  // weight
                                    0x82, 0x84, 0x86, 0x41, 0x8A, 0x08, 0x9D, 0x5C, 0x0B, 0x81, 0x70, 0xDC,
                                    0x78, 0x0F, 0x03, 0x53, 0x03, 0x2A, 0x2F, 0x2A, 0x90, 0x7A, 0x8D, 0xAA,
                                    0x69, 0xD2, 0x9A, 0xC4, 0xC0, 0x57, 0x65, 0x76, 0xD6, 0xBF, 0x83, 0x8F};

  http2::frames_t framesh0 = http2::parse_frames((const char*)headers_frame0, sizeof(headers_frame0));
  ASSERT_TRUE(framesh0.size() == 1);

  http2::frame_headers* head0 = (http2::frame_headers*)(&framesh0[0]);
  ASSERT_TRUE(head0->type() == 0x01);
  ASSERT_TRUE(head0->flags() == 0x25);
  ASSERT_TRUE(head0->stream_id() == 0x0D);
  ASSERT_TRUE(head0->payload_size() == 0x29);

  uint8_t padlen0 = head0->padlen();
  ASSERT_TRUE(padlen0 == 0);
  const http2::http2_priority_spec* priorh0 = head0->priority();
  ASSERT_TRUE(priorh0->stream_id() == 0x0B);
  ASSERT_TRUE(priorh0->weight() == 0x0F + 1);

  std::vector<http2::http2_nv> nva0 = head0->nva();
  ASSERT_TRUE(nva0.size() == 7);
  http::HttpRequest r0;
  std::pair<http::http_status, Error> err0 = http2::parse_http_request(*head0, &r0);
  ASSERT_EQ(r0.GetMethod(), http::HM_GET);
  ASSERT_EQ(r0.GetPath().GetPath(), "/");
  ASSERT_TRUE(r0.GetHeaders().size() == 3);
  ASSERT_TRUE(r0.GetBody().empty());
  ASSERT_FALSE(err0.second);

  const uint8_t headers_frame[] = {0x00, 0x00, 0x29, 0x01, 0x25, 0x00, 0x00, 0x00, 0x0D, /*header*/
                                   0x00, 0x00, 0x00,
                                   0x0B,  // dep_stream_id
                                   0x0F,  // weight
                                   0x82, 0x84, 0x86, 0x41, 0x8A, 0x08, 0x9D, 0x5C, 0x0B, 0x81, 0x70, 0xDC,
                                   0x78, 0x0F, 0x03, 0x53, 0x03, 0x2A, 0x2F, 0x2A, 0x90, 0x7A, 0x8D, 0xAA,
                                   0x69, 0xD2, 0x9A, 0xC4, 0xC0, 0x57, 0x65, 0x76, 0xD6, 0xBF, 0x83, 0x8F};

  http2::frames_t framesh1 = http2::parse_frames((const char*)headers_frame, sizeof(headers_frame));
  ASSERT_TRUE(framesh1.size() == 1);

  http2::frame_headers* head1 = (http2::frame_headers*)(&framesh1[0]);
  ASSERT_TRUE(head1->type() == 0x01);
  ASSERT_TRUE(head1->flags() == 0x25);
  ASSERT_TRUE(head1->stream_id() == 0x0D);
  ASSERT_TRUE(head1->payload_size() == 0x29);

  uint8_t padlen = head1->padlen();
  ASSERT_TRUE(padlen == 0);
  const http2::http2_priority_spec* priorh1 = head1->priority();
  ASSERT_TRUE(priorh1->stream_id() == 0x0B);
  ASSERT_TRUE(priorh1->weight() == 0x0F + 1);

  std::vector<http2::http2_nv> nva1 = head1->nva();
  ASSERT_TRUE(nva1.size() == 7);
  http::HttpRequest r1;
  std::pair<http::http_status, Error> err = http2::parse_http_request(*head1, &r1);
  ASSERT_EQ(r1.GetMethod(), http::HM_GET);
  ASSERT_EQ(r1.GetPath().GetPath(), "/");
  ASSERT_TRUE(r1.GetHeaders().size() == 3);
  ASSERT_TRUE(r1.GetBody().empty());
  ASSERT_FALSE(err.second);
  // headers frame//
}

TEST(Http2, frame_goaway) {
  // 000019070000000000000000000000000153455454494E4753206578706563746564
  const uint8_t away_frame[] = {0x00, 0x00, 0x19, 0x07, 0x00, 0x00, 0x00, 0x00, 0x00, /*header*/
                                0x00, 0x00, 0x00,
                                0x00,  // stream_id
                                0x00, 0x00, 0x00,
                                0x01,  // error_code
                                0x53, 0x45, 0x54, 0x54, 0x49, 0x4E, 0x47, 0x53, 0x20,
                                0x65, 0x78, 0x70, 0x65, 0x63, 0x74, 0x65, 0x64};

  // goaway frame//
  http2::frames_t frames1 = http2::parse_frames((const char*)away_frame, sizeof(away_frame));
  ASSERT_TRUE(frames1.size() == 1);

  http2::frame_goaway* goaway = (http2::frame_goaway*)(&frames1[0]);
  ASSERT_TRUE(goaway->type() == 0x07);
  ASSERT_TRUE(goaway->flags() == 0x00);
  ASSERT_TRUE(goaway->stream_id() == 0x00);
  ASSERT_TRUE(goaway->payload_size() == 0x19);

  ASSERT_TRUE(goaway->last_stream_id() == 0x00);
  ASSERT_TRUE(goaway->error_code() == http2::HTTP2_PROTOCOL_ERROR);
  uint32_t oplen = goaway->opaque_data_len();
  uint8_t* opd = goaway->opaque_data();
  const uint8_t* opd2 = goaway->c_payload() + sizeof(uint32_t) * 2;
  ASSERT_TRUE(sizeof(uint32_t) * 2 == goaway->payload_size() - oplen);
  ASSERT_TRUE(memcmp(opd, opd2, oplen) == 0);
  // goaway frame///
}

TEST(http_client, get) {
  net::HostAndPort example("example.com", 80);
  uri::Upath root;
  net::HttpClient cl(example);
  ErrnoError err = cl.Connect();
  ASSERT_FALSE(err);
  Error err2 = cl.Get(root);
  ASSERT_FALSE(err2);
  http::HttpResponse resp;
  err2 = cl.ReadResponce(&resp);
  ASSERT_FALSE(err2);
  err = cl.Disconnect();
  ASSERT_FALSE(err);
}
