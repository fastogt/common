#include <gtest/gtest.h>

#include <common/license/gen_hardware_hash.h>

TEST(License, Generate) {
  bool hdd_hash = common::license::GenerateHardwareHash(common::license::HDD, nullptr);
  ASSERT_FALSE(hdd_hash);

  bool mach_hash = common::license::GenerateHardwareHash(common::license::MACHINE_ID, nullptr);
  ASSERT_FALSE(mach_hash);
}
