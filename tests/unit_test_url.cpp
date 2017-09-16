#include <gtest/gtest.h>

#include <common/uri/url.h>

TEST(Upath, isValidPathAndQuery) {
  common::uri::Upath path;
  ASSERT_FALSE(path.IsValid());

  const std::string file_name = "index.html";
  common::uri::Upath path2(file_name);
  ASSERT_TRUE(path2.IsValid());
  ASSERT_EQ(file_name, path2.GetUpath());

  const std::string file_path = "/example.html";
  const std::string query = "var=This+is+a+simple+%26+short+test";
  common::uri::Upath path3(file_path + "?" + query);
  ASSERT_TRUE(path3.IsValid());
  ASSERT_EQ(file_path, path3.GetPath());
  // const char* dec = common::uri::detail::uri_decode(query.c_str(), query.length());
  // ASSERT_EQ(dec, path3.Query());

  // invalid Upath
  const std::string invalid2 = "..";
  common::uri::Upath pathInvalid(invalid2);
  ASSERT_FALSE(pathInvalid.IsValid());

  const std::string invalid3 = "../";
  pathInvalid = common::uri::Upath(invalid3);
  ASSERT_FALSE(pathInvalid.IsValid());

  const std::string invalid4 = "/../";
  pathInvalid = common::uri::Upath(invalid4);
  ASSERT_FALSE(pathInvalid.IsValid());

  const std::string invalid5 = "/..";
  pathInvalid = common::uri::Upath(invalid5);
  ASSERT_FALSE(pathInvalid.IsValid());
}

TEST(Url, IsValid) {
  common::uri::Url path;
  ASSERT_FALSE(path.IsValid());

  common::uri::Url path2("http://www.permadi.com/index.html");
  ASSERT_TRUE(path2.IsValid());

  common::uri::Url path3("http://www.permadi.com/tutorial/urlEncoding/index.html");
  ASSERT_TRUE(path3.IsValid());

  common::uri::Url path4(
      "http://www.permadi.com/tutorial/urlEncoding/"
      "example.html?var=This+is+a+simple+%26+short+test");
  ASSERT_TRUE(path4.IsValid());
}

TEST(Url, Scheme) {
  common::uri::Url http_uri("http://localhost:8080/hls/69_avformat_test_alex_2/play.m3u8");
  ASSERT_EQ(http_uri.GetScheme(), common::uri::Url::http);
  ASSERT_EQ(http_uri.GetHost(), "localhost:8080");
  common::uri::Upath http_path = http_uri.GetPath();
  ASSERT_EQ(http_path.GetHpath(), "hls/69_avformat_test_alex_2/");
  ASSERT_EQ(http_path.GetPath(), "hls/69_avformat_test_alex_2/play.m3u8");
  ASSERT_EQ(http_path.GetFileName(), "play.m3u8");
  ASSERT_EQ(http_path.GetUpath(), "hls/69_avformat_test_alex_2/play.m3u8");
  ASSERT_EQ(http_path.GetPath(), http_path.GetHpath() + http_path.GetFileName());

  common::uri::Url ftp_uri("ftp://localhost:8080");
  ASSERT_EQ(ftp_uri.GetScheme(), common::uri::Url::ftp);
  ASSERT_EQ(ftp_uri.GetHost(), "localhost:8080");
  common::uri::Upath ftp_path = ftp_uri.GetPath();
  ASSERT_EQ(ftp_path.GetHpath(), std::string());
  ASSERT_EQ(ftp_path.GetPath(), std::string());
  ASSERT_EQ(ftp_path.GetFileName(), std::string());
  ASSERT_EQ(ftp_path.GetUpath(), std::string());
  ASSERT_EQ(ftp_path.GetPath(), ftp_path.GetHpath() + ftp_path.GetFileName());

  common::uri::Url file_uri("file:///home/sasha/2.txt");
  ASSERT_EQ(file_uri.GetScheme(), common::uri::Url::file);
  ASSERT_EQ(file_uri.GetPath(), common::uri::Upath("/home/sasha/2.txt"));
  ASSERT_EQ(file_uri.GetHost(), std::string());

  common::uri::Url invalid_uri("home://user/logo.png");
  ASSERT_FALSE(invalid_uri.IsValid());

  common::uri::Url ws_uri("ws://localhost:8080");
  ASSERT_EQ(ws_uri.GetScheme(), common::uri::Url::ws);
  ASSERT_EQ(ws_uri.GetHost(), "localhost:8080");

  common::uri::Url udp_uri("udp://localhost:8080");
  ASSERT_EQ(udp_uri.GetScheme(), common::uri::Url::udp);
  ASSERT_EQ(udp_uri.GetHost(), "localhost:8080");

  common::uri::Url rtmp_uri("rtmp://localhost:8080");
  ASSERT_EQ(rtmp_uri.GetScheme(), common::uri::Url::rtmp);
  ASSERT_EQ(rtmp_uri.GetHost(), "localhost:8080");
}

TEST(Url, level) {
  common::uri::Url http_uri("http://localhost:8080/hls/69_avformat_test_alex_2/play.m3u8");
  common::uri::Upath p = http_uri.GetPath();
  size_t lv = p.GetLevels();
  ASSERT_EQ(lv, 2);
  std::string l1 = p.GetHpathLevel(1);
  ASSERT_EQ(l1, "hls/");
  std::string l2 = p.GetHpathLevel(2);
  ASSERT_EQ(l2, "hls/69_avformat_test_alex_2/");
}

TEST(Url, urlEncode) {
  const std::string url = "https://mywebsite/docs/english/site/mybook.do";
  char* enc = common::uri::detail::uri_encode(url.c_str(), url.length());
  ASSERT_STREQ(enc, "https%3A%2F%2Fmywebsite%2Fdocs%2Fenglish%2Fsite%2Fmybook.do");
  char* dec = common::uri::detail::uri_decode(enc, strlen(enc));
  ASSERT_STREQ(dec, url.c_str());
  free(enc);
  free(dec);
}
