#include <gtest/gtest.h>

#include <common/text_decoders/base64_edcoder.h>
#include <common/text_decoders/compress_snappy_edcoder.h>
#include <common/text_decoders/compress_zlib_edcoder.h>
#include <common/text_decoders/compress_bzip2_edcoder.h>
#include <common/text_decoders/compress_lz4_edcoder.h>
#include <common/text_decoders/hex_edcoder.h>
#include <common/text_decoders/html_edcoder.h>
#include <common/text_decoders/iedcoder.h>
#include <common/text_decoders/iedcoder_factory.h>
#include <common/text_decoders/msgpack_edcoder.h>

TEST(msg_pack, enc_dec) {
  const std::string raw_data = "alex aalex talex 123 balex";
  common::MsgPackEDcoder zl;
  std::string enc_data;
  common::Error err = zl.Encode(raw_data, &enc_data);
  ASSERT_FALSE(err);

  std::string dec_data;
  err = zl.Decode(enc_data, &dec_data);
  ASSERT_FALSE(err);
  ASSERT_EQ(raw_data, dec_data);
}

TEST(html, enc_dec) {
  const std::string raw_data = "alex aalex talex balex";
  common::HtmlEscEDcoder zl;
  std::string enc_data;
  common::Error err = zl.Encode(raw_data, &enc_data);
  ASSERT_FALSE(err);

  std::string dec_data;
  err = zl.Decode(enc_data, &dec_data);
  ASSERT_FALSE(err);
  ASSERT_EQ(raw_data, dec_data);
}

TEST(hex, enc_dec) {
  const std::string raw_data = "alex aalex talex balex";
  common::HexEDcoder zl;
  std::string enc_data;
  common::Error err = zl.Encode(raw_data, &enc_data);
  ASSERT_FALSE(err);

  std::string dec_data;
  err = zl.Decode(enc_data, &dec_data);
  ASSERT_FALSE(err);
  ASSERT_EQ(raw_data, dec_data);
}

TEST(base64, enc_dec) {
  const std::string raw_data = "alex aalex talex balex";
  common::Base64EDcoder zl;
  std::string enc_data;
  common::Error err = zl.Encode(raw_data, &enc_data);
  ASSERT_FALSE(err);

  std::string dec_data;
  err = zl.Decode(enc_data, &dec_data);
  ASSERT_FALSE(err);
  ASSERT_EQ(raw_data, dec_data);
}

#ifdef HAVE_ZLIB
TEST(zlib, enc_dec) {
  const std::string raw_data = "alex aalex talex balex";
  common::CompressZlibEDcoder zl;
  std::string enc_data;
  common::Error err = zl.Encode(raw_data, &enc_data);
  ASSERT_FALSE(err);

  std::string dec_data;
  err = zl.Decode(enc_data, &dec_data);
  ASSERT_FALSE(err);
  ASSERT_EQ(raw_data, dec_data);
}
#endif

#ifdef HAVE_BZIP2
TEST(bzip2, enc_dec) {
  const std::string raw_data = "alex aalex talex balex";
  common::CompressBZip2EDcoder bz;
  std::string enc_data;
  common::Error err = bz.Encode(raw_data, &enc_data);
  ASSERT_FALSE(err);

  std::string dec_data;
  err = bz.Decode(enc_data, &dec_data);
  ASSERT_FALSE(err);
  ASSERT_EQ(raw_data, dec_data);
}
#endif

#ifdef HAVE_LZ4
TEST(lz4, enc_dec) {
  const std::string raw_data = "alex aalex talex balex";
  common::CompressLZ4EDcoder lz4;
  std::string enc_data;
  common::Error err = lz4.Encode(raw_data, &enc_data);
  ASSERT_FALSE(err);

  std::string dec_data;
  err = lz4.Decode(enc_data, &dec_data);
  ASSERT_FALSE(err);
  ASSERT_EQ(raw_data, dec_data);
}
#endif

#ifdef HAVE_SNAPPY
TEST(snappy, enc_dec) {
  const std::string raw_data = "alex aalex talex balex";
  common::CompressSnappyEDcoder zl;
  std::string enc_data;
  common::Error err = zl.Encode(raw_data, &enc_data);
  ASSERT_FALSE(err);

  std::string dec_data;
  err = zl.Decode(enc_data, &dec_data);
  ASSERT_FALSE(err);
  ASSERT_EQ(raw_data, dec_data);
}
#endif

TEST(iedcoder, factory) {
  for (size_t i = 0; i < common::ENCODER_DECODER_NUM_TYPES; ++i) {
    std::string fac = common::edecoder_types[i];
    common::IEDcoder* dec = common::CreateEDCoder(fac);
    ASSERT_TRUE(dec);

    common::EDType t;
    ASSERT_TRUE(common::ConvertFromString(fac, &t));
    ASSERT_EQ(fac, common::ConvertToString(t));

    common::IEDcoder* dec2 = common::CreateEDCoder(t);
    ASSERT_TRUE(dec2);
    ASSERT_EQ(dec->GetType(), dec2->GetType());

    delete dec;
    delete dec2;
  }
}
