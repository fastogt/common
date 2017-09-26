#include <gtest/gtest.h>

#include <common/convert2string.h>
#include <common/hash/md5.h>
#include <common/hash/sha1.h>
#include <common/hash/sha256.h>

TEST(hash, md5) {
  common::hash::MD5_CTX ctx;
  common::buffer_t test = MAKE_BUFFER("test");
  unsigned char md5_result[MD5_HASH_LENGHT];
  common::hash::MD5_Init(&ctx);
  common::hash::MD5_Update(&ctx, test.data(), test.size());
  common::hash::MD5_Final(&ctx, md5_result);
  std::string hexed = common::utils::hex::encode(std::string(md5_result, md5_result + MD5_HASH_LENGHT), true);
  ASSERT_EQ(hexed, "098f6bcd4621d373cade4e832627b4f6");
}

TEST(hash, sha1) {
  common::hash::SHA1_CTX ctx;
  common::buffer_t test = MAKE_BUFFER("test");
  common::hash::SHA1_Init(&ctx);
  common::hash::SHA1_Update(&ctx, test.data(), test.size());
  unsigned char sha1_result[SHA1_HASH_LENGTH];
  common::hash::SHA1_Final(&ctx, sha1_result);
  std::string sha1_str = std::string(sha1_result, sha1_result + SHA1_HASH_LENGTH);
  std::string hexed = common::utils::hex::encode(sha1_str, true);
  ASSERT_EQ(hexed, "a94a8fe5ccb19ba61c4c0873d391e987982fbbd3");
}

TEST(hash, sha256) {
  common::hash::SHA256_CTX ctx;
  common::buffer_t test = MAKE_BUFFER("test");
  unsigned char sha256_result[SHA256_HASH_LENGHT];
  common::hash::SHA256_Init(&ctx);
  common::hash::SHA256_Update(&ctx, test.data(), test.size());
  common::hash::SHA256_Final(&ctx, sha256_result);
  std::string hexed = common::utils::hex::encode(std::string(sha256_result, sha256_result + SHA256_HASH_LENGHT), true);
  ASSERT_EQ(hexed, "9f86d081884c7d659a2feaa0c55ad015a3bf4f1b2b0b822cd15d6c15b0f00a08");
}
