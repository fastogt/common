/*  Copyright (C) 2014-2017 FastoGT. All right reserved.

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

#include <gtest/gtest.h>

#include <common/text_decoders/base64_edcoder.h>
#include <common/text_decoders/compress_bzip2_edcoder.h>
#include <common/text_decoders/compress_lz4_edcoder.h>
#include <common/text_decoders/compress_snappy_edcoder.h>
#include <common/text_decoders/compress_zlib_edcoder.h>
#include <common/text_decoders/hex_edcoder.h>
#include <common/text_decoders/html_edcoder.h>
#include <common/text_decoders/iedcoder.h>
#include <common/text_decoders/iedcoder_factory.h>
#include <common/text_decoders/none_edcoder.h>
#include <common/text_decoders/unicode_edcoder.h>

TEST(html, enc_dec) {
  const common::char_buffer_t raw_data = MAKE_CHAR_BUFFER("alex aalex talex balex");
  common::HtmlEscEDcoder zl;
  common::char_buffer_t enc_data;
  common::Error err = zl.Encode(raw_data, &enc_data);
  ASSERT_FALSE(err);

  common::char_buffer_t dec_data;
  err = zl.Decode(enc_data, &dec_data);
  ASSERT_FALSE(err);
  ASSERT_EQ(raw_data, dec_data);
}

TEST(hex, enc_dec) {
  const common::char_buffer_t raw_data = MAKE_CHAR_BUFFER("alex aalex talex balex");
  common::HexEDcoder zl;
  common::char_buffer_t enc_data;
  common::Error err = zl.Encode(raw_data, &enc_data);
  ASSERT_FALSE(err);

  common::char_buffer_t dec_data;
  err = zl.Decode(enc_data, &dec_data);
  ASSERT_FALSE(err);
  ASSERT_EQ(raw_data, dec_data);
}

TEST(xhex, enc_dec) {
  const common::char_buffer_t raw_data = MAKE_CHAR_BUFFER("alex aalex talex balex");
  common::HexEDcoder zl;
  common::char_buffer_t enc_data;
  common::Error err = zl.Decode(raw_data, &enc_data);
  ASSERT_TRUE(err);

  common::char_buffer_t dec_data;
  err = zl.Encode(enc_data, &dec_data);
  ASSERT_TRUE(err);
}

TEST(unicode, enc_dec) {
  const common::char_buffer_t raw_data = MAKE_CHAR_BUFFER("alex aalex talex balex");
  common::UnicodeEDcoder zl;
  common::char_buffer_t enc_data;
  common::Error err = zl.Encode(raw_data, &enc_data);
  ASSERT_FALSE(err);

  common::char_buffer_t dec_data;
  err = zl.Decode(enc_data, &dec_data);
  ASSERT_FALSE(err);
  ASSERT_EQ(raw_data, dec_data);
}

TEST(base64, enc_dec) {
  const common::char_buffer_t raw_data = MAKE_CHAR_BUFFER("alex aalex talex balex");
  common::Base64EDcoder zl;
  common::char_buffer_t enc_data;
  common::Error err = zl.Encode(raw_data, &enc_data);
  ASSERT_FALSE(err);

  common::char_buffer_t dec_data;
  err = zl.Decode(enc_data, &dec_data);
  ASSERT_FALSE(err);
  ASSERT_EQ(raw_data, dec_data);
}

#ifdef HAVE_ZLIB
TEST(zlib, enc_dec) {
  const common::char_buffer_t raw_data = MAKE_CHAR_BUFFER("alex aalex talex balexalex aalex talex balex");
  common::CompressZlibEDcoder zl(false);
  common::char_buffer_t enc_data;
  common::Error err = zl.Encode(raw_data, &enc_data);
  ASSERT_FALSE(err);

  common::char_buffer_t dec_data;
  err = zl.Decode(enc_data, &dec_data);
  ASSERT_FALSE(err);
  ASSERT_EQ(raw_data, dec_data);
}
#endif

#ifdef HAVE_BZIP2
TEST(bzip2, enc_dec) {
  const common::char_buffer_t raw_data =
      MAKE_CHAR_BUFFER("alex aalex talex balexalex aalex talex balexalex aalex talex balex");
  common::CompressBZip2EDcoder bz;
  common::char_buffer_t enc_data;
  common::Error err = bz.Encode(raw_data, &enc_data);
  ASSERT_FALSE(err);

  common::char_buffer_t dec_data;
  err = bz.Decode(enc_data, &dec_data);
  ASSERT_FALSE(err);
  ASSERT_EQ(raw_data, dec_data);
}
#endif

#ifdef HAVE_LZ4
TEST(lz4, enc_dec) {
  const common::char_buffer_t raw_data = MAKE_CHAR_BUFFER("alex aalex talex balex");
  common::CompressLZ4EDcoder lz4;
  common::char_buffer_t enc_data;
  common::Error err = lz4.Encode(raw_data, &enc_data);
  ASSERT_FALSE(err);

  common::char_buffer_t dec_data;
  err = lz4.Decode(enc_data, &dec_data);
  ASSERT_FALSE(err);
  ASSERT_EQ(raw_data, dec_data);
}
#endif

#ifdef HAVE_SNAPPY
TEST(snappy, enc_dec) {
  const common::char_buffer_t raw_data = MAKE_CHAR_BUFFER("alex aalex talex balex");
  common::CompressSnappyEDcoder zl;
  common::char_buffer_t enc_data;
  common::Error err = zl.Encode(raw_data, &enc_data);
  ASSERT_FALSE(err);

  common::char_buffer_t dec_data;
  err = zl.Decode(enc_data, &dec_data);
  ASSERT_FALSE(err);
  ASSERT_EQ(raw_data, dec_data);
}
#endif

TEST(none, enc_dec) {
  const common::char_buffer_t raw_data = MAKE_CHAR_BUFFER("alex aalex talex balex");
  common::NoneEDcoder zl;
  common::char_buffer_t enc_data;
  common::Error err = zl.Encode(raw_data, &enc_data);
  ASSERT_FALSE(err);
  ASSERT_EQ(raw_data, enc_data);

  common::char_buffer_t dec_data;
  err = zl.Decode(enc_data, &dec_data);
  ASSERT_FALSE(err);
  ASSERT_EQ(raw_data, dec_data);
}

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
