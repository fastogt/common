#include <gtest/gtest.h>

#include <common/bounded_value.h>

template <typename T, T min, T max>
void bound_test_without_overflow(T in_range) {
  typedef common::BoundedValue<T, min, max> boound_int_t;
  boound_int_t boound_int(in_range);
  ASSERT_EQ(boound_int, in_range);
  ASSERT_EQ(boound_int.value(in_range), in_range);

  boound_int = max + 1;
  ASSERT_EQ(boound_int, boound_int_t::max_value);

  boound_int = min - 1;
  ASSERT_EQ(boound_int, boound_int_t::min_value);

  boound_int = min;
  ASSERT_EQ(boound_int, boound_int_t::min_value);

  boound_int = max;
  ASSERT_EQ(boound_int, boound_int_t::max_value);
}

TEST(BoundedValue, ranges) {
  bound_test_without_overflow<char, -10, 126>(10);
  bound_test_without_overflow<char, -10, 126>(-10);
  bound_test_without_overflow<char, -10, 126>(100);

  bound_test_without_overflow<short, 126, 126>(126);
  bound_test_without_overflow<short, 15, 1025>(20);
  bound_test_without_overflow<short, -100, 126>(100);

  bound_test_without_overflow<int, 0, 10>(7);
  bound_test_without_overflow<int, 0, 10>(0);
  bound_test_without_overflow<int, 0, 10>(10);
}
