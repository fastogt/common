#include <gtest/gtest.h>

#include <common/license/hardware_hash.h>

TEST(License, GenerateInvalid) {
  bool hdd_hash = common::license::GenerateHardwareHash(common::license::HDD, nullptr);
  ASSERT_FALSE(hdd_hash);

  bool mach_hash = common::license::GenerateHardwareHash(common::license::MACHINE_ID, nullptr);
  ASSERT_FALSE(mach_hash);
}

TEST(License, Generate) {
  common::license::hardware_hash_t hash;
  bool hdd_hash = common::license::GenerateHardwareHash(common::license::HDD, &hash);
  if (hdd_hash) {
    ASSERT_TRUE(common::license::IsValidHardwareHash(hash));
  }

  bool mach_hash = common::license::GenerateHardwareHash(common::license::MACHINE_ID, &hash);
  if (mach_hash) {
    ASSERT_TRUE(common::license::IsValidHardwareHash(hash));
  }
}
