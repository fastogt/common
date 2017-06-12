#include "test_thread_func.h"

#include <stdint.h>
#include <stdlib.h>

#define TEST_BODY_FUNC()                      \
  for (uint16_t i = 0; i < UINT16_MAX; ++i) { \
    rand();                                   \
  }
#define CALC_BODY_FUNC()                      \
  for (uint32_t i = 0; i < UINT32_MAX; ++i) { \
  }

void test_empty() {}

void test() {
  TEST_BODY_FUNC()
}

void calc() {
  CALC_BODY_FUNC()
}

void test_empty0() {}
void test_empty1() {}
void test_empty2() {}
void test_empty3() {}
void test_empty4() {}
void test_empty5() {}
void test_empty6() {}

void test0() {
  test();
}

void test1() {
  calc();
}

void test2() {
  test();
}

void test3() {
  calc();
}

void test4() {
  test();
}

void test5() {
  calc();
}

void test6() {
  test();
}

void test0c() {
  TEST_BODY_FUNC()
}

void test1c() {
  CALC_BODY_FUNC()
}

void test2c() {
  TEST_BODY_FUNC()
}

void test3c() {
  CALC_BODY_FUNC()
}

void test4c() {
  TEST_BODY_FUNC()
}

void test5c() {
  CALC_BODY_FUNC()
}

void test6c() {
  TEST_BODY_FUNC()
}
