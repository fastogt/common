#include <gtest/gtest.h>

#include <common/convert2string.h>
#include <common/utf_string_conversions.h>

#define OPENSSL_VERSION_NUMBER_EXAMPLE 0x00090301
#define OPENSSL_VERSION_TEXT_EXAMPLE "0.9.3.1"

TEST(string, convertTo) {
  int val = 11;
  const std::string stext = "11";
  const std::string convText = common::ConvertToString(val);
  ASSERT_EQ(stext, convText);

  long lval = 11;
  const std::string ltext = "11";
  const std::string lconvText = common::ConvertToString(lval);
  ASSERT_EQ(ltext, lconvText);

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
  const common::string16 text = common::UTF8ToUTF16("text");
  const std::string stext = "text";
  const common::string16 convText = common::ConvertToString16(stext);
  ASSERT_EQ(text, convText);

  std::string fstext;
  bool res = common::ConvertFromString16(convText, &fstext);
  ASSERT_TRUE(res);
  ASSERT_EQ(stext, fstext);

  const common::string16 text2 = common::UTF8ToUTF16("Привет");
  const std::string stext2 = "Привет";
  const common::string16 convText2 = common::ConvertToString16(stext2);
  ASSERT_EQ(text2, convText2);

  std::string fstext2;
  res = common::ConvertFromString16(convText2, &fstext2);
  ASSERT_TRUE(res);
  ASSERT_EQ(stext2, fstext2);
}
