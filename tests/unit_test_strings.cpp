#include <gtest/gtest.h>

#include <common/convert2string.h>
#include <common/sprintf.h>
#include <common/string_piece.h>
#include <common/string_util.h>
#include <common/utf_string_conversions.h>

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
  std::string result = common::MemSPrintf("%d %s", val_int, val_string);
  ASSERT_EQ(result, std::to_string(val_int) + " " + val_string);
}

TEST(string, utils) {
  const std::string test_data_low = "alex,palec,malec";
  const std::string test_data_up = "ALEX,PALEC,MALEC";
  std::vector<std::string> splited;
  size_t tok = common::Tokenize(test_data_low, ",", &splited);
  ASSERT_EQ(tok, splited.size());
  const std::string merged = common::JoinString(splited, ",");
  ASSERT_EQ(test_data_low, merged);

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
