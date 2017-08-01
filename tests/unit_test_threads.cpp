#include <gtest/gtest.h>

#include <common/threads/thread_manager.h>

std::shared_ptr<common::threads::Thread<void> > some_thread;
void test() {
  common::threads::Thread<void>* cur_thr = THREAD_MANAGER()->CurrentThread<void>();
  common::threads::PlatformThreadHandle cur_handle = cur_thr->GetHandle();
  ASSERT_EQ(cur_handle, some_thread->GetHandle());
  ASSERT_EQ(cur_handle, common::threads::current_thread_handle());
  ASSERT_TRUE(cur_thr->IsRunning());
  ASSERT_TRUE(THREAD_MANAGER()->IsCurrentThread(cur_thr));
}

TEST(THREAD_MANAGER, tests) {
  ASSERT_NE(common::threads::current_thread_handle(), common::threads::invalid_thread_handle());

  ASSERT_TRUE(THREAD_MANAGER()->IsMainThread());
  auto main_thread = THREAD_MANAGER()->CurrentThread<int>();
  auto main_handle = main_thread->GetHandle();
  ASSERT_TRUE(THREAD_MANAGER()->IsCurrentThread(main_thread));
  ASSERT_EQ(main_handle, common::threads::current_thread_handle());
  ASSERT_TRUE(main_thread->IsRunning());

  some_thread = THREAD_MANAGER()->CreateThread(&test);
  ASSERT_EQ(some_thread->GetHandle(), common::threads::invalid_thread_handle());
  ASSERT_TRUE(some_thread->Start());
  some_thread->Join();
  ASSERT_EQ(some_thread->GetHandle(), common::threads::invalid_thread_handle());
}
