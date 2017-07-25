#include <gtest/gtest.h>

#include <common/text_decoders/compress_snappy_edcoder.h>
#include <common/text_decoders/compress_zlib_edcoder.h>

TEST(zlib, enc_dec) {
  const std::string raw_data = "alex alex alex alex";
  common::CompressZlibEDcoder zl;
  std::string enc_data;
  common::Error err = zl.Encode(raw_data, &enc_data);
  ASSERT_FALSE(err && err->IsError());

  std::string dec_data;
  err = zl.Decode(enc_data, &dec_data);
  ASSERT_FALSE(err && err->IsError());
  ASSERT_EQ(raw_data, dec_data);
}

TEST(snappy, enc_dec) {
  const std::string raw_data = "alex alex alex alex";
  common::CompressSnappyEDcoder zl;
  std::string enc_data;
  common::Error err = zl.Encode(raw_data, &enc_data);
  ASSERT_FALSE(err && err->IsError());

  std::string dec_data;
  err = zl.Decode(enc_data, &dec_data);
  ASSERT_FALSE(err && err->IsError());
  ASSERT_EQ(raw_data, dec_data);
}
