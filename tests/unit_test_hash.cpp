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

#include <common/convert2string.h>
#include <common/hash/md5.h>
#include <common/hash/sha1.h>
#include <common/hash/sha256.h>

TEST(hash, md5) {
  common::hash::MD5_CTX ctx;
  const common::buffer_t test = MAKE_BUFFER("test");
  unsigned char md5_result[MD5_HASH_LENGHT];
  common::hash::MD5_Init(&ctx);
  common::hash::MD5_Update(&ctx, test.data(), test.size());
  common::hash::MD5_Final(&ctx, md5_result);
  std::string hexed;
  bool is_ok = common::utils::hex::encode(MAKE_CHAR_BUFFER_SIZE(md5_result, MD5_HASH_LENGHT), true, &hexed);
  ASSERT_TRUE(is_ok);
  ASSERT_EQ(hexed, "098f6bcd4621d373cade4e832627b4f6");

  const common::buffer_t test2 = MAKE_BUFFER("q;{sNM@Nq@*GG\6`");
  common::hash::MD5_Init(&ctx);
  common::hash::MD5_Update(&ctx, test2.data(), test2.size());
  common::hash::MD5_Final(&ctx, md5_result);
  is_ok = common::utils::hex::encode(MAKE_CHAR_BUFFER_SIZE(md5_result, MD5_HASH_LENGHT), true, &hexed);
  ASSERT_TRUE(is_ok);
  ASSERT_EQ(hexed, "596bfb58af00a1900a2a738fec648577");  // "8c22fc4554ed971072e3a5ac2a3a615c"
}

TEST(hash, sha1) {
  common::hash::SHA1_CTX ctx;
  const common::buffer_t test = MAKE_BUFFER("test");
  common::hash::SHA1_Init(&ctx);
  common::hash::SHA1_Update(&ctx, test.data(), test.size());
  unsigned char sha1_result[SHA1_HASH_LENGTH];
  common::hash::SHA1_Final(&ctx, sha1_result);
  std::string hexed;
  bool is_ok = common::utils::hex::encode(MAKE_CHAR_BUFFER_SIZE(sha1_result, SHA1_HASH_LENGTH), true, &hexed);
  ASSERT_TRUE(is_ok);
  ASSERT_EQ(hexed, "a94a8fe5ccb19ba61c4c0873d391e987982fbbd3");
}

TEST(hash, sha256) {
  common::hash::SHA256_CTX ctx;
  const common::buffer_t test = MAKE_BUFFER("test");
  unsigned char sha256_result[SHA256_HASH_LENGHT];
  common::hash::SHA256_Init(&ctx);
  common::hash::SHA256_Update(&ctx, test.data(), test.size());
  common::hash::SHA256_Final(&ctx, sha256_result);
  std::string hexed;
  bool is_ok = common::utils::hex::encode(MAKE_CHAR_BUFFER_SIZE(sha256_result, SHA256_HASH_LENGHT), true, &hexed);
  ASSERT_TRUE(is_ok);
  ASSERT_EQ(hexed, "9f86d081884c7d659a2feaa0c55ad015a3bf4f1b2b0b822cd15d6c15b0f00a08");
}
