#include <gtest/gtest.h>

#include <common/logger.h>

TEST(Logger, log_level) {
  for (size_t i = 0; i < common::logging::LEVEL_LOG_COUNT; ++i) {
    common::logging::LEVEL_LOG cur = static_cast<common::logging::LEVEL_LOG>(i);
    const char* log_level_text = common::logging::log_level_to_text(cur);
    common::logging::LEVEL_LOG lg;
    ASSERT_TRUE(log_level_text && text_to_log_level(log_level_text, &lg));
    ASSERT_EQ(cur, lg);
  }
}

TEST(Logger, set_loglevel) {
  const common::logging::LEVEL_LOG cur = common::logging::L_DEBUG;
  common::logging::SET_CURRENT_LOG_LEVEL(cur);
  ASSERT_EQ(cur, common::logging::CURRENT_LOG_LEVEL());
}
