#include <gtest/gtest.h>

#include <common/system_info/cpu_info.h>

TEST(Cpu, CurrentCpuInfo) {
  const common::system_info::CpuInfo& c1 = common::system_info::CurrentCpuInfo();
  const common::system_info::CpuInfo& c2 = common::system_info::CurrentCpuInfo();
  const std::string c1_br = c1.BrandName();
  const std::string c2_br = c2.BrandName();
  GTEST_ASSERT_EQ(c1_br, c2_br);
  GTEST_ASSERT_EQ(c1, c2);
}
