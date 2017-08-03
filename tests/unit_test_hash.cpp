#include <gtest/gtest.h>

#include <common/convert2string.h>
#include <common/hash/md5.h>
#include <common/hash/sha1.h>

TEST(hash, md5) {
  common::hash::MD5_CTX ctx;
  std::string test = "test";
  unsigned char md5_result[MD5_HASH_LENGHT];
  common::hash::MD5_Init(&ctx);
  common::hash::MD5_Update(&ctx, test.c_str(), test.size());
  common::hash::MD5_Final(md5_result, &ctx);
  std::string hexed = common::utils::hex::encode(std::string(md5_result, md5_result + MD5_HASH_LENGHT), true);
  ASSERT_EQ(hexed, "098f6bcd4621d373cade4e832627b4f6");
}

TEST(hash, sha1) {
  common::hash::sha1nfo ctx;
  std::string test = "test";
  common::hash::sha1_init(&ctx);
  common::hash::sha1_write(&ctx, test.c_str(), test.size());
  uint8_t* sha1_result = common::hash::sha1_result(&ctx);
  std::string sha1_str = std::string(sha1_result, sha1_result + SHA1_HASH_LENGTH);
  std::string hexed = common::utils::hex::encode(sha1_str, true);
  ASSERT_EQ(hexed, "a94a8fe5ccb19ba61c4c0873d391e987982fbbd3");
}
