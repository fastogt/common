#include <gtest/gtest.h>

#include <common/system_info/cpu_info.h>

TEST(Cpu, CurrentCpuInfo) {
  const auto c1 = common::system_info::CPU();
  const auto c2 = common::system_info::CPU();
  const std::string c1_br = c1.cpu_brand();
  const std::string c2_br = c2.cpu_brand();

  GTEST_ASSERT_EQ(c1_br, c2_br);
}
