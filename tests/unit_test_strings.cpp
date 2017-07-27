#include <gtest/gtest.h>

#include <common/convert2string.h>
#include <common/sprintf.h>
#include <common/string_util.h>
#include <common/utf_string_conversions.h>

#define OPENSSL_VERSION_NUMBER_EXAMPLE 0x00090301
#define OPENSSL_VERSION_TEXT_EXAMPLE "0.9.3.1"

TEST(string, convertTo) {
  int val = 11;
  const std::string stext = "11";
  const std::string conv_text = common::ConvertToString(val);
  ASSERT_EQ(stext, conv_text);
  int res_val;
  ASSERT_TRUE(common::ConvertFromString(stext, &res_val));
  ASSERT_EQ(res_val, val);

  long lval = 11;
  const std::string ltext = "11";
  const std::string lconv_text = common::ConvertToString(lval);
  ASSERT_EQ(ltext, lconv_text);
  int res_lval;
  ASSERT_TRUE(common::ConvertFromString(ltext, &res_lval));
  ASSERT_EQ(res_lval, lval);

  double dval = 11.12;
  const std::string dtext = "11.12";
  const std::string dconv_text = common::ConvertToString(dval, 2);
  ASSERT_EQ(dtext, dconv_text);
  double res_dval;
  ASSERT_TRUE(common::ConvertFromString(dtext, &res_dval));
  ASSERT_EQ(res_dval, dval);

  const std::string dconv1_text = common::ConvertToString(dval, 1);
  ASSERT_EQ(dconv1_text, dtext.substr(0, dtext.size() - 1));
  double res_dval1;
  ASSERT_TRUE(common::ConvertFromString(dconv1_text, &res_dval1));

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

TEST(string, MemSPrintf) {
  const int val_int = 11;
  const std::string val_string = "alex";
  std::string result = common::MemSPrintf("%d %s", val_int, val_string);
  ASSERT_EQ(result, std::to_string(val_int) + " " + val_string);
}

TEST(string, utils) {
  const std::string test_data = "alex,palec,malec";
  std::vector<std::string> splited;
  size_t tok = common::Tokenize(test_data, ",", &splited);
  ASSERT_EQ(tok, splited.size());
  std::string merged = common::JoinString(splited, ",");
  ASSERT_EQ(test_data, merged);
}
