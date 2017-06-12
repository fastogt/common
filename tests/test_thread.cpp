#include <gtest/gtest.h>

#include "test_thread_func.h"

#include <common/threads/thread_manager.h>

TEST(Thread, basic_loop) {
  common::thread tp[THREAD_COUNTS];
  DCHECK(SIZEOFMASS(tp) == THREAD_COUNTS);
  for (size_t i = 0; i < SIZEOFMASS(tp); ++i) {
    tp[i] = common::thread(&test);
  }

  for (size_t i = 0; i < SIZEOFMASS(tp); ++i) {
    tp[i].join();
  }
}

TEST(Thread, fasto_loop) {
  std::shared_ptr<common::threads::Thread<void> > tp[THREAD_COUNTS];

  for (size_t i = 0; i < SIZEOFMASS(tp); ++i) {
    tp[i] = THREAD_MANAGER()->CreateThread(&test);
    GTEST_ASSERT_EQ(tp[i]->GetTid(), common::threads::invalid_tid);
    tp[i]->Start();
  }

  for (size_t i = 0; i < SIZEOFMASS(tp); ++i) {
    tp[i]->Join();
  }
}
