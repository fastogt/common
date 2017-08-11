#include <gtest/gtest.h>

#include <thread>

#include "test_thread_func.h"

#include <common/threads/thread_manager.h>

closure_t getFunction(int index) {
  if (index == 0) {
    return &test_empty0;
  } else if (index == 1) {
    return &test_empty1;
  } else if (index == 2) {
    return &test_empty2;
  } else if (index == 3) {
    return &test_empty3;
  } else if (index == 4) {
    return &test_empty4;
  } else if (index == 5) {
    return &test_empty5;
  } else if (index == MANY_EMPTY_FUNCTION_COUNT) {
    return &test_empty6;
  } else {
    NOTREACHED();
    return NULL;
  }
}

TEST(Thread, basic_many_empty_function) {
  std::thread tp[THREAD_COUNTS];

  for (size_t i = 0; i < SIZEOFMASS(tp); ++i) {
    closure_t cl = getFunction(i % (MANY_EMPTY_FUNCTION_COUNT + 1));
    tp[i] = std::thread(cl);
  }

  for (size_t i = 0; i < SIZEOFMASS(tp); ++i) {
    tp[i].join();
  }
}

TEST(Thread, fasto_many_empty_function) {
  std::shared_ptr<common::threads::Thread<void> > tp[THREAD_COUNTS];

  for (size_t i = 0; i < SIZEOFMASS(tp); ++i) {
    closure_t cl = getFunction(i % (MANY_EMPTY_FUNCTION_COUNT + 1));
    tp[i] = THREAD_MANAGER()->CreateThread(cl);
    GTEST_ASSERT_EQ(tp[i]->GetHandle(), common::threads::invalid_thread_handle());
    bool res = tp[i]->Start();
    DCHECK(res);
  }

  for (size_t i = 0; i < SIZEOFMASS(tp); ++i) {
    tp[i]->Join();
  }
}
