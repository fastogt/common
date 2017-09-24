#include <gtest/gtest.h>

#include <common/media/bandwidth_estimation.h>

TEST(ConvertToString, media) {
  const common::media::Rational rat = {1, 25};
  std::string rat_str = common::ConvertToString(rat);
  ASSERT_EQ(rat_str, "1:25");

  common::media::Rational rat2;
  ASSERT_TRUE(common::ConvertFromString(rat_str, &rat2));
  ASSERT_EQ(rat2, rat);
}

TEST(DesireBytesPerSec, Instance) {
  const common::media::DesireBytesPerSec invalid;
  ASSERT_FALSE(invalid.IsValid());

  const common::media::DesireBytesPerSec can_be(100, 200);
  ASSERT_TRUE(can_be.IsValid());
  ASSERT_TRUE(can_be.InRange(150));

  common::media::DesireBytesPerSec can_be_2x = can_be + can_be;
  ASSERT_TRUE(can_be_2x.IsValid());
  ASSERT_FALSE(can_be_2x.InRange(100));
  ASSERT_TRUE(can_be_2x.InRange(200));
  ASSERT_TRUE(can_be_2x.InRange(300));
  ASSERT_TRUE(can_be_2x.InRange(400));
  ASSERT_FALSE(can_be_2x.InRange(500));
  ASSERT_EQ(can_be_2x.max, 400);
  ASSERT_EQ(can_be_2x.min, 200);
}
