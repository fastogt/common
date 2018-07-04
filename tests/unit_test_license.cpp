#include <gtest/gtest.h>

#include <common/license/gen_hardware_hash.h>

TEST(License, Generate) {
  std::string hdd_hash_text;
  bool hdd_hash = common::license::GenerateHardwareHash(common::license::HDD, &hdd_hash_text);
  ASSERT_TRUE(hdd_hash);

  std::string mach_hash_text;
  bool mach_hash = common::license::GenerateHardwareHash(common::license::MACHINE_ID, &mach_hash_text);
  ASSERT_TRUE(mach_hash);
}
