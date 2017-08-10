#include <gtest/gtest.h>

#include <common/byte_writer.h>
#include <common/convert2string.h>
#include <common/sprintf.h>
#include <common/string_piece.h>
#include <common/string_util.h>
#include <common/utf_string_conversions.h>

#include <common/compress/base64.h>
#include <common/compress/hex.h>
#include <common/compress/snappy_compress.h>
#include <common/compress/zlib_compress.h>

#ifdef QT_ENABLED
#include <common/qt/convert2string.h>
#endif

#define OPENSSL_VERSION_NUMBER_EXAMPLE 0x00090301
#define OPENSSL_VERSION_TEXT_EXAMPLE "0.9.3.1"

template <typename T>
void ConvertToStringCheck(T val, const std::string& val_expected_str) {
  const std::string val_text = common::ConvertToString(val);
  ASSERT_EQ(val_expected_str, val_text);
  T res_val;
  ASSERT_TRUE(common::ConvertFromString(val_text, &res_val));
  ASSERT_EQ(val, res_val);
}

template <typename T>
void ConvertToString16Check(T val, const common::string16& val_expected_str) {
  const common::string16 val_text = common::ConvertToString16(val);
  ASSERT_EQ(val_expected_str, val_text);
  T res_val;
  ASSERT_TRUE(common::ConvertFromString16(val_text, &res_val));
  ASSERT_EQ(val, res_val);
}

TEST(string, convertTo) {
  ConvertToStringCheck(true, "true");
  ConvertToStringCheck(false, "false");

  ConvertToStringCheck(11, "11");
  ConvertToStringCheck(-11, "-11");

  ConvertToStringCheck(141L, "141");
  ConvertToStringCheck(-141L, "-141");

  ConvertToStringCheck(141UL, "141");
  ConvertToStringCheck(1421UL, "1421");

  ConvertToStringCheck(11.12, "11.12");

  std::string openssl_str_verson = common::ConvertVersionNumberTo3DotString(OPENSSL_VERSION_NUMBER_EXAMPLE);
  ASSERT_EQ(openssl_str_verson, OPENSSL_VERSION_TEXT_EXAMPLE);
  uint32_t openssl_ver_number = common::ConvertVersionNumberFromString(OPENSSL_VERSION_TEXT_EXAMPLE);
  ASSERT_EQ(openssl_ver_number, OPENSSL_VERSION_NUMBER_EXAMPLE);

  uint32_t ver_number = common::ConvertVersionNumberFromString(PROJECT_VERSION);
  ASSERT_EQ(ver_number, PROJECT_VERSION_NUMBER);
  std::string str_verson = common::ConvertVersionNumberTo3DotString(ver_number);
  ASSERT_EQ(str_verson, PROJECT_VERSION);
}

TEST(string16, convertTo) {
  ConvertToString16Check(true, common::UTF8ToUTF16("true"));
  ConvertToString16Check(false, common::UTF8ToUTF16("false"));

  ConvertToString16Check(11, common::UTF8ToUTF16("11"));
  ConvertToString16Check(-11, common::UTF8ToUTF16("-11"));

  ConvertToString16Check(11.32, common::UTF8ToUTF16("11.32"));

  ConvertToString16Check(std::string("Привет"), common::UTF8ToUTF16("Привет"));
}

TEST(string, MemSPrintf) {
  const int val_int = 11;
  const std::string val_string = "alex";
  const std::string val_esc_string = "alex\n";
  std::string result = common::MemSPrintf("%d %s %d", val_int, val_string, true);
  ASSERT_EQ(result, std::to_string(val_int) + " " + val_string + " 1");

  std::string esc = common::EscapedText(val_string);
  ASSERT_EQ(esc, val_esc_string);
  esc = common::EscapedText(esc);
  ASSERT_EQ(esc, val_esc_string);

  const common::buffer_t buff = {0, 1, 2};
  const std::string buff_str = common::ConvertToString(buff);
  const std::string get_command = "GET";
  const std::string plus = get_command + " " + buff_str;

  common::char_writer<512> bw;
  bw << get_command << " " << buff;
  auto vec_result = bw.GetBuffer();
  ASSERT_EQ(std::string(vec_result.begin(), vec_result.end()), plus);
  bw << common::ConvertToString(true);
  vec_result = bw.GetBuffer();
  ASSERT_EQ(std::string(vec_result.begin(), vec_result.end()), plus + common::ConvertToString(true));

  common::unsigned_char_writer<512> bw2;
  bw2 << get_command << " " << buff;
  common::buffer_t result2 = bw2.GetBuffer();
  ASSERT_EQ(result2, common::ConvertToBytes(plus));
  bw2 << common::ConvertToString(11);
  result2 = bw2.GetBuffer();
  ASSERT_EQ(result2, common::ConvertToBytes(plus + common::ConvertToString(11)));
}

TEST(string, utils) {
  const std::string test_data_low = "alex,palec,malec";
  const std::string test_data_up = "ALEX,PALEC,MALEC";
  std::vector<std::string> splited;
  size_t tok = common::Tokenize(test_data_low, ",", &splited);
  ASSERT_EQ(tok, splited.size());
  const std::string merged = common::JoinString(splited, ",");
  ASSERT_EQ(test_data_low, merged);

  const common::buffer_t test_data_low2 = MAKE_BUFFER("alex,palec,malec");
  std::vector<common::buffer_t> splited2;
  size_t tok2 = common::Tokenize(test_data_low2, {','}, &splited2);
  ASSERT_EQ(tok2, splited2.size());
  const common::buffer_t merged2 = common::JoinString(splited2, {','});
  ASSERT_EQ(test_data_low2, merged2);

  const std::string test_data_upper = common::StringToUpperASCII(test_data_low);
  ASSERT_EQ(test_data_upper, test_data_up);

  const std::string test_data_lower = common::StringToLowerASCII(test_data_up);
  ASSERT_EQ(test_data_low, test_data_lower);
  ASSERT_TRUE(common::FullEqualsASCII(test_data_low, test_data_up, false));
  ASSERT_FALSE(common::FullEqualsASCII(test_data_low, test_data_up, true));
  ASSERT_TRUE(common::FullEqualsASCII(test_data_low, test_data_lower, true));
}

TEST(string, StringPiece) {
  const std::string sasha_string = "sasha";
  common::StringPiece str_sasha(sasha_string);
  ASSERT_TRUE(str_sasha.starts_with("sa"));
  ASSERT_FALSE(str_sasha.starts_with("as"));
  common::StringPiece str_sasha2(str_sasha);
  ASSERT_EQ(str_sasha, str_sasha2);
  ASSERT_EQ(str_sasha.as_string(), sasha_string);
}

TEST(string, HexCompress) {
  const common::buffer_t json = MAKE_BUFFER(R"("{"back_end_credentials" : {"creds" :
      { "host" : "127.0.0.1:6379","unix_socket" : "/var/run/redis/redis.sock"},"type" : "1"},
      "id" : "relay_76","input" : {"urls" : [ {"id" : 222,"uri" : "http://example.com"} ]},
      "output" : {"urls" : [ {"id" : 71,"uri" : "http://example.com","hls_type" : "push",
      "http_root" : "/home/sasha/timeshift/26"} ]},
      "stats_credentials" : {"creds" : {"host" : "127.0.0.1:6379","unix_socket" : "/var/run/redis/redis.sock"},
      "type" : "1"},"type" : "relay"})");
  common::buffer_t encoded_hex;
  common::Error err = common::compress::EncodeHex(json, false, &encoded_hex);
  ASSERT_FALSE(err && err->IsError());

  common::buffer_t decoded;
  err = common::compress::DecodeHex(encoded_hex, &decoded);
  ASSERT_FALSE(err && err->IsError());
  ASSERT_EQ(json, decoded);
}

TEST(string, Base64Compress) {
  const common::buffer_t json = MAKE_BUFFER(
      R"("{"back_end_credentials" : {"creds" :
      { "host" : "127.0.0.1:6379","unix_socket" : "/var/run/redis/redis.sock"},"type" : "1"},
      "id" : "relay_76","input" : {"urls" : [ {"id" : 222,"uri" : "http://example.com"} ]},
      "output" : {"urls" : [ {"id" : 71,"uri" : "http://example.com","hls_type" : "push",
      "http_root" : "/home/sasha/timeshift/26"} ]},
      "stats_credentials" : {"creds" : {"host" : "127.0.0.1:6379","unix_socket" : "/var/run/redis/redis.sock"},
      "type" : "1"},"type" : "relay"})");
  common::buffer_t encoded_base64;
  common::Error err = common::compress::EncodeBase64(json, &encoded_base64);
  ASSERT_FALSE(err && err->IsError());

  common::buffer_t decoded;
  err = common::compress::DecodeBase64(encoded_base64, &decoded);
  ASSERT_FALSE(err && err->IsError());
  ASSERT_EQ(json, decoded);
}

#ifdef HAVE_ZLIB
TEST(string, ZlibCompress) {
  const common::buffer_t json = MAKE_BUFFER(
      R"("{"back_end_credentials" : {"creds" :
      { "host" : "127.0.0.1:6379","unix_socket" : "/var/run/redis/redis.sock"},"type" : "1"},
      "id" : "relay_76","input" : {"urls" : [ {"id" : 222,"uri" : "http://example.com"} ]},
      "output" : {"urls" : [ {"id" : 71,"uri" : "http://example.com","hls_type" : "push",
      "http_root" : "/home/sasha/timeshift/26"} ]},
      "stats_credentials" : {"creds" : {"host" : "127.0.0.1:6379","unix_socket" : "/var/run/redis/redis.sock"},
      "type" : "1"},"type" : "relay"})");
  common::buffer_t encoded_zlib;
  common::Error err = common::compress::EncodeZlib(json, &encoded_zlib);
  ASSERT_FALSE(err && err->IsError());

  common::buffer_t decoded;
  err = common::compress::DecodeZlib(encoded_zlib, &decoded);
  ASSERT_FALSE(err && err->IsError());
  ASSERT_EQ(json, decoded);
}
#endif

#ifdef HAVE_SNAPPY
TEST(string, SnappyCompress) {
  const std::string json =
      R"("{"back_end_credentials" : {"creds" :
      { "host" : "127.0.0.1:6379","unix_socket" : "/var/run/redis/redis.sock"},"type" : "1"},
      "id" : "relay_76","input" : {"urls" : [ {"id" : 222,"uri" : "http://example.com"} ]},
      "output" : {"urls" : [ {"id" : 71,"uri" : "http://example.com","hls_type" : "push",
      "http_root" : "/home/sasha/timeshift/26"} ]},
      "stats_credentials" : {"creds" : {"host" : "127.0.0.1:6379","unix_socket" : "/var/run/redis/redis.sock"},
      "type" : "1"},"type" : "relay"})";
  std::string encoded_snappy;
  common::Error err = common::compress::EncodeSnappy(json, &encoded_snappy);
  ASSERT_FALSE(err && err->IsError());

  std::string decoded;
  err = common::compress::DecodeSnappy(encoded_snappy, &decoded);
  ASSERT_FALSE(err && err->IsError());
  ASSERT_EQ(json, decoded);
}
#endif

TEST(Utils, hex) {
  const std::string invalid_hex = "C";
  auto invalid_dec = common::utils::hex::decode(invalid_hex);
  ASSERT_TRUE(invalid_dec.empty());

  const std::string invalid_hex_2 = "CY";
  auto invalid_dec2 = common::utils::hex::decode(invalid_hex_2);
  ASSERT_TRUE(invalid_dec2.empty());

  std::string empty_hex;
  invalid_dec = common::utils::hex::decode(empty_hex);
  ASSERT_TRUE(invalid_dec.empty());

  const common::buffer_t hex = MAKE_BUFFER("CDFF");
  auto dec = common::utils::hex::decode(hex);

  ASSERT_EQ(dec.size(), 2);
  ASSERT_EQ(dec[0], 0xCD);
  ASSERT_EQ(dec[1], 0xFF);

  common::buffer_t enc = common::utils::hex::encode(dec, false);
  ASSERT_EQ(hex, enc);

  const common::buffer_t hex2 = MAKE_BUFFER("11ff");
  dec = common::utils::hex::decode(hex2);

  ASSERT_EQ(dec.size(), 2);
  ASSERT_EQ(dec[0], 0x11);
  ASSERT_EQ(dec[1], 0xFF);

  enc = common::utils::hex::encode(dec, true);
  ASSERT_EQ(hex2, enc);

  const common::buffer_t seq = MAKE_BUFFER("1234567890");
  const std::vector<uint8_t> dec_seq = {18, 52, 86, 120, 144};
  std::vector<uint8_t> dec2 = common::utils::hex::decode(seq);
  ASSERT_EQ(dec_seq, dec2);

  common::buffer_t enc2 = common::utils::hex::encode(dec2, false);
  ASSERT_EQ(seq, enc2);

  const common::buffer_t dec_seq_x = {103, 101, 116, 32, 39, 00, 171, 32, 39};
  const common::buffer_t dec_seq_x2 = MAKE_BUFFER("get '\x00\xAB\x20'");
  ASSERT_EQ(dec_seq_x, dec_seq_x2);

  const std::string bin_data_x = std::string("get '\x00\xAB\x20'", 9);
  const std::string bin_data_x_fail = R"(get '\x00\xAB\x20')";
  const std::string bin_data_x_ok = MAKE_STRING("get '\x00\xAB\x20'");
  std::string dec_seq_x_str = common::ConvertToString(dec_seq_x);
  ASSERT_EQ(dec_seq_x_str, bin_data_x);
  ASSERT_NE(dec_seq_x_str, bin_data_x_fail);
  ASSERT_EQ(dec_seq_x_str, bin_data_x_ok);
  common::buffer_t dec_seq_x3;
  ASSERT_TRUE(common::ConvertFromString(dec_seq_x_str, &dec_seq_x3));
  ASSERT_EQ(dec_seq_x, dec_seq_x3);

  common::buffer_t enc_bin_x = common::utils::hex::encode(dec_seq_x, true);
  common::buffer_t dec_bin_x = common::utils::hex::decode(enc_bin_x);
  ASSERT_EQ(dec_seq_x, dec_bin_x);

  std::string enc_bin_x2 = common::utils::hex::encode(bin_data_x, true);
  std::string dec_bin_x2 = common::utils::hex::decode(enc_bin_x2);
  ASSERT_EQ(bin_data_x, dec_bin_x2);
}

TEST(ConvertFromString, boolean) {
  bool b = false;
  bool* ptr = nullptr;
  ASSERT_FALSE(common::ConvertFromString("true", ptr));

  ASSERT_TRUE(common::ConvertFromString("true", &b));
  ASSERT_TRUE(b);

  ASSERT_TRUE(common::ConvertFromString("TRUE", &b));
  ASSERT_TRUE(b);

  ASSERT_TRUE(common::ConvertFromString("false", &b));
  ASSERT_FALSE(b);

  ASSERT_TRUE(common::ConvertFromString("abc", &b));
  ASSERT_FALSE(b);
}

TEST(ConvertFromString, uint8) {
  uint8_t val;
  uint8_t* ptr = nullptr;
  ASSERT_FALSE(common::ConvertFromString("1", ptr));

  ASSERT_TRUE(common::ConvertFromString("1", &val));
  ASSERT_EQ(val, 1);

  ASSERT_TRUE(common::ConvertFromString("0", &val));
  ASSERT_EQ(val, 0);

  ASSERT_TRUE(common::ConvertFromString("255", &val));
  ASSERT_EQ(val, 255);

  ASSERT_FALSE(common::ConvertFromString("256", &val));
  ASSERT_FALSE(common::ConvertFromString("-1", &val));

  ASSERT_FALSE(common::ConvertFromString("99999999999999999999999999999", &val));
  ASSERT_FALSE(common::ConvertFromString("-99999999999999999999999999999", &val));
  ASSERT_FALSE(common::ConvertFromString("abc42", &val));
}

TEST(ConvertFromString, int8) {
  char val;
  char* ptr = nullptr;
  ASSERT_FALSE(common::ConvertFromString("1", ptr));

  ASSERT_TRUE(common::ConvertFromString("1", &val));
  ASSERT_EQ(val, 1);

  ASSERT_TRUE(common::ConvertFromString("-128", &val));
  ASSERT_EQ(val, -128);

  ASSERT_TRUE(common::ConvertFromString("127", &val));
  ASSERT_EQ(val, 127);

  ASSERT_FALSE(common::ConvertFromString("128", &val));
  ASSERT_FALSE(common::ConvertFromString("-129", &val));

  ASSERT_FALSE(common::ConvertFromString("99999999999999999999999999999", &val));
  ASSERT_FALSE(common::ConvertFromString("-99999999999999999999999999999", &val));
  ASSERT_FALSE(common::ConvertFromString("abc42", &val));
}

TEST(ConvertFromString, uint16) {
  uint16_t val;
  uint16_t* ptr = nullptr;
  ASSERT_FALSE(common::ConvertFromString("1", ptr));

  ASSERT_TRUE(common::ConvertFromString("1", &val));
  ASSERT_EQ(val, 1);

  ASSERT_TRUE(common::ConvertFromString("0", &val));
  ASSERT_EQ(val, 0);

  ASSERT_TRUE(common::ConvertFromString("65535", &val));
  ASSERT_EQ(val, 65535);

  ASSERT_FALSE(common::ConvertFromString("-1", &val));
  ASSERT_FALSE(common::ConvertFromString("65536", &val));

  ASSERT_FALSE(common::ConvertFromString("99999999999999999999999999999", &val));
  ASSERT_FALSE(common::ConvertFromString("-99999999999999999999999999999", &val));
  ASSERT_FALSE(common::ConvertFromString("abc42", &val));
}

TEST(ConvertFromString, int16) {
  int16_t val;
  int16_t* ptr = nullptr;
  ASSERT_FALSE(common::ConvertFromString("1", ptr));

  ASSERT_TRUE(common::ConvertFromString("1", &val));
  ASSERT_EQ(val, 1);

  ASSERT_TRUE(common::ConvertFromString("-32768", &val));
  ASSERT_EQ(val, -32768);

  ASSERT_TRUE(common::ConvertFromString("32767", &val));
  ASSERT_EQ(val, 32767);

  ASSERT_FALSE(common::ConvertFromString("-32769", &val));
  ASSERT_FALSE(common::ConvertFromString("32768", &val));

  ASSERT_FALSE(common::ConvertFromString("99999999999999999999999999999", &val));
  ASSERT_FALSE(common::ConvertFromString("-99999999999999999999999999999", &val));
  ASSERT_FALSE(common::ConvertFromString("abc42", &val));
}

TEST(ConvertFromString, uint32) {
  uint32_t val;
  uint32_t* ptr = nullptr;
  ASSERT_FALSE(common::ConvertFromString("1", ptr));

  ASSERT_TRUE(common::ConvertFromString("1", &val));
  ASSERT_EQ(val, 1);

  ASSERT_TRUE(common::ConvertFromString("0", &val));
  ASSERT_EQ(val, 0);

  ASSERT_TRUE(common::ConvertFromString("4294967295", &val));
  ASSERT_EQ(val, 4294967295);

  ASSERT_FALSE(common::ConvertFromString("-1", &val));
  ASSERT_FALSE(common::ConvertFromString("4294967297", &val));

  ASSERT_FALSE(common::ConvertFromString("99999999999999999999999999999", &val));
  ASSERT_FALSE(common::ConvertFromString("-99999999999999999999999999999", &val));
  ASSERT_FALSE(common::ConvertFromString("abc42", &val));
}

TEST(ConvertFromString, int32) {
  int32_t val;
  int16_t* ptr = nullptr;
  ASSERT_FALSE(common::ConvertFromString("1", ptr));

  ASSERT_TRUE(common::ConvertFromString("1", &val));
  ASSERT_EQ(val, 1);

  ASSERT_TRUE(common::ConvertFromString("-2147483648", &val));
  ASSERT_EQ(val, -2147483648);

  ASSERT_TRUE(common::ConvertFromString("2147483647", &val));
  ASSERT_EQ(val, 2147483647);

  ASSERT_FALSE(common::ConvertFromString("-2147483649", &val));
  ASSERT_FALSE(common::ConvertFromString("2147483648", &val));

  ASSERT_FALSE(common::ConvertFromString("99999999999999999999999999999", &val));
  ASSERT_FALSE(common::ConvertFromString("-99999999999999999999999999999", &val));
  ASSERT_FALSE(common::ConvertFromString("abc42", &val));
}

TEST(ConvertFromString, uint64) {
  uint64_t val;
  uint64_t* ptr = nullptr;
  ASSERT_FALSE(common::ConvertFromString("1", ptr));

  ASSERT_TRUE(common::ConvertFromString("1", &val));
  ASSERT_EQ(val, 1);

  ASSERT_TRUE(common::ConvertFromString("0", &val));
  ASSERT_EQ(val, 0);

  ASSERT_TRUE(common::ConvertFromString("18446744073709551615", &val));
  ASSERT_EQ(val, 18446744073709551615ULL);

  ASSERT_FALSE(common::ConvertFromString("18446744073709551616", &val));

  ASSERT_FALSE(common::ConvertFromString("99999999999999999999999999999", &val));
  ASSERT_FALSE(common::ConvertFromString("-99999999999999999999999999999", &val));
  ASSERT_FALSE(common::ConvertFromString("abc42", &val));
}

TEST(ConvertFromString, int64) {
  int64_t val;
  int64_t* ptr = nullptr;
  ASSERT_FALSE(common::ConvertFromString("1", ptr));

  ASSERT_TRUE(common::ConvertFromString("1", &val));
  ASSERT_EQ(val, 1);

  ASSERT_TRUE(common::ConvertFromString("-9223372036854775807", &val));
  ASSERT_EQ(val, -9223372036854775807LL);

  ASSERT_TRUE(common::ConvertFromString("9223372036854775807", &val));
  ASSERT_EQ(val, 9223372036854775807LL);

  ASSERT_FALSE(common::ConvertFromString("-9223372036854775809", &val));
  ASSERT_FALSE(common::ConvertFromString("9223372036854775808", &val));

  ASSERT_FALSE(common::ConvertFromString("99999999999999999999999999999", &val));
  ASSERT_FALSE(common::ConvertFromString("-99999999999999999999999999999", &val));
  ASSERT_FALSE(common::ConvertFromString("abc42", &val));
}

TEST(ConvertFromString, string) {
  std::string val;
  std::string* ptr = nullptr;
  ASSERT_FALSE(common::ConvertFromString("1", ptr));

  ASSERT_TRUE(common::ConvertFromString("", &val));
  ASSERT_EQ(val, "");

  ASSERT_TRUE(common::ConvertFromString(std::string(), &val));
  ASSERT_EQ(val, std::string());

  ASSERT_TRUE(common::ConvertFromString("abc\n ", &val));
  ASSERT_EQ(val, "abc\n ");
}

TEST(ConvertFromString, float) {
  float val;
  float* ptr = nullptr;
  ASSERT_FALSE(common::ConvertFromString("1", ptr));

  ASSERT_TRUE(common::ConvertFromString("3.1415926", &val));
  ASSERT_FLOAT_EQ(val, 3.1415926f);

  ASSERT_TRUE(common::ConvertFromString("3.4028234e38", &val));
  ASSERT_FLOAT_EQ(val, 3.4028234e38f);

  ASSERT_TRUE(common::ConvertFromString("1.1754943e-38", &val));
  ASSERT_FLOAT_EQ(val, 1.1754943e-38f);

  ASSERT_TRUE(common::ConvertFromString("0.0000001", &val));
  ASSERT_FLOAT_EQ(val, 0.0000001f);
}

TEST(ConvertFromString, double) {
  double val;
  double* ptr = nullptr;
  ASSERT_FALSE(common::ConvertFromString("1", ptr));

  ASSERT_TRUE(common::ConvertFromString("3.1415926", &val));
  ASSERT_DOUBLE_EQ(val, 3.1415926);

  ASSERT_TRUE(common::ConvertFromString("1.7976931348623157e308", &val));
  ASSERT_DOUBLE_EQ(val, 1.7976931348623157e308);

  ASSERT_TRUE(common::ConvertFromString("2.2250738585072014e-308", &val));
  ASSERT_DOUBLE_EQ(val, 2.2250738585072014e-308);

  ASSERT_TRUE(common::ConvertFromString("0.0000000000000001", &val));
  ASSERT_DOUBLE_EQ(val, 0.0000000000000001);
}

TEST(ConvertToString, boolean) {
  std::string s = common::ConvertToString(true);
  ASSERT_EQ(s, "true");
  s = common::ConvertToString(false);
  ASSERT_EQ(s, "false");
}

TEST(ConvertToString, uint8) {
  std::string s = common::ConvertToString(std::numeric_limits<uint8_t>::min());
  ASSERT_EQ(s, "0");
  s = common::ConvertToString(std::numeric_limits<uint8_t>::max());
  ASSERT_EQ(s, "255");
}

TEST(ConvertToString, int8) {
  std::string s = common::ConvertToString(std::numeric_limits<int8_t>::min());
  ASSERT_EQ(s, "-128");
  s = common::ConvertToString(std::numeric_limits<int8_t>::max());
  ASSERT_EQ(s, "127");
}

TEST(ConvertToString, uint16) {
  std::string s = common::ConvertToString(std::numeric_limits<uint16_t>::min());
  ASSERT_EQ(s, "0");
  s = common::ConvertToString(std::numeric_limits<uint16_t>::max());
  ASSERT_EQ(s, "65535");
}

TEST(ConvertToString, int16) {
  std::string s = common::ConvertToString(std::numeric_limits<int16_t>::min());
  ASSERT_EQ(s, "-32768");
  s = common::ConvertToString(std::numeric_limits<int16_t>::max());
  ASSERT_EQ(s, "32767");
}

TEST(ConvertToString, uint32) {
  std::string s = common::ConvertToString(std::numeric_limits<uint32_t>::min());
  ASSERT_EQ(s, "0");
  s = common::ConvertToString(std::numeric_limits<uint32_t>::max());
  ASSERT_EQ(s, "4294967295");
}

TEST(ConvertToString, int32) {
  std::string s = common::ConvertToString(std::numeric_limits<int32_t>::min());
  ASSERT_EQ(s, "-2147483648");
  s = common::ConvertToString(std::numeric_limits<int32_t>::max());
  ASSERT_EQ(s, "2147483647");
}

TEST(ConvertToString, uint64) {
  std::string s = common::ConvertToString(std::numeric_limits<uint64_t>::min());
  ASSERT_EQ(s, "0");
  s = common::ConvertToString(std::numeric_limits<uint64_t>::max());
  ASSERT_EQ(s, "18446744073709551615");
}

TEST(ConvertToString, int64) {
  std::string s = common::ConvertToString(std::numeric_limits<int64_t>::min());
  ASSERT_EQ(s, "-9223372036854775808");
  s = common::ConvertToString(std::numeric_limits<int64_t>::max());
  ASSERT_EQ(s, "9223372036854775807");
}

TEST(ConvertToString, string) {
  std::string val = "  \tabcDEF\n";
  std::string s = common::ConvertToString(val);
  ASSERT_EQ(s, val);
}

TEST(ConvertToString, float) {
  std::string s = common::ConvertToString(3.141593f, 6);
  ASSERT_EQ(s, "3.141593");
}

TEST(ConvertToString, double) {
  std::string s = common::ConvertToString(3.141593, 6);
  ASSERT_EQ(s, "3.141593");
}

#ifdef QT_ENABLED
void ConvertQtTests(const char* test_str) {
  const common::string16 string16_str = common::UTF8ToUTF16(test_str);
  const std::string str = test_str;
  const QString qstr = test_str;

  std::string s = common::ConvertToString(qstr);
  ASSERT_EQ(s, str);

  QString qs;
  ASSERT_TRUE(common::ConvertFromString(s, &qs));
  ASSERT_EQ(qs, qstr);

  QString qs2;
  ASSERT_TRUE(common::ConvertFromString16(string16_str, &qs2));
  ASSERT_EQ(qs2, qstr);

  std::string ss = common::ConvertToString(string16_str);
  ASSERT_EQ(ss, str);

  common::StringPiece spiece_s;
  ASSERT_TRUE(common::ConvertFromString(ss, &spiece_s));
  ASSERT_EQ(str, spiece_s);

  common::StringPiece16 spiece16_s;
  ASSERT_TRUE(common::ConvertFromString16(string16_str, &spiece16_s));
  ASSERT_EQ(spiece16_s, string16_str);
}

TEST(ConvertToString, qt) {
  ConvertQtTests("Sasha");
  ConvertQtTests("Привет Андрей");
  ConvertQtTests("Привет Sasha");
}
#endif
