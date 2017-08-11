#include <gtest/gtest.h>

#include <thread>

#include "test_thread_func.h"

#include <common/threads/thread_manager.h>

closure_t getWFunction(int index) {
  if (index == 0) {
    return &test0;
  } else if (index == 1) {
    return &test1;
  } else if (index == 2) {
    return &test2;
  } else if (index == 3) {
    return &test3;
  } else if (index == 4) {
    return &test4;
  } else if (index == 5) {
    return &test5;
  } else if (index == MANY_FUNCTION_COUNT) {
    return &test6;
  } else {
    NOTREACHED();
    return NULL;
  }
}

closure_t getOneBodyMultFunction(int index) {
  if (index == 0) {
    return &test0c;
  } else if (index == 1) {
    return &test1c;
  } else if (index == 2) {
    return &test2c;
  } else if (index == 3) {
    return &test3c;
  } else if (index == 4) {
    return &test4c;
  } else if (index == 5) {
    return &test5c;
  } else if (index == MANY_FUNCTION_COUNT) {
    return &test6c;
  } else {
    NOTREACHED();
    return NULL;
  }
}

TEST(Thread, basic_many_function) {
  std::thread tp[THREAD_COUNTS];

  for (size_t i = 0; i < SIZEOFMASS(tp); ++i) {
    closure_t cl = getWFunction(i % (MANY_FUNCTION_COUNT + 1));
    tp[i] = std::thread(cl);
  }

  for (size_t i = 0; i < SIZEOFMASS(tp); ++i) {
    tp[i].join();
  }
}

TEST(Thread, fasto_many_function) {
  std::shared_ptr<common::threads::Thread<void> > tp[THREAD_COUNTS];

  for (size_t i = 0; i < SIZEOFMASS(tp); ++i) {
    closure_t cl = getWFunction(i % (MANY_FUNCTION_COUNT + 1));
    tp[i] = THREAD_MANAGER()->CreateThread(cl);
    GTEST_ASSERT_EQ(tp[i]->GetHandle(), common::threads::invalid_thread_handle());
    bool res = tp[i]->Start();
    DCHECK(res);
  }

  for (size_t i = 0; i < SIZEOFMASS(tp); ++i) {
    tp[i]->Join();
  }
}

TEST(Thread, basic_many_onebody_function) {
  std::thread tp[THREAD_COUNTS];

  for (size_t i = 0; i < SIZEOFMASS(tp); ++i) {
    closure_t cl = getOneBodyMultFunction(i % (MANY_FUNCTION_COUNT + 1));
    tp[i] = std::thread(cl);
  }

  for (size_t i = 0; i < SIZEOFMASS(tp); ++i) {
    tp[i].join();
  }
}

TEST(Thread, fasto_many_onebody_function) {
  std::shared_ptr<common::threads::Thread<void> > tp[THREAD_COUNTS];

  for (size_t i = 0; i < SIZEOFMASS(tp); ++i) {
    closure_t cl = getOneBodyMultFunction(i % (MANY_FUNCTION_COUNT + 1));
    tp[i] = THREAD_MANAGER()->CreateThread(cl);
    GTEST_ASSERT_EQ(tp[i]->GetHandle(), common::threads::invalid_thread_handle());
    bool res = tp[i]->Start();
    DCHECK(res);
  }

  for (size_t i = 0; i < SIZEOFMASS(tp); ++i) {
    tp[i]->Join();
  }
}
