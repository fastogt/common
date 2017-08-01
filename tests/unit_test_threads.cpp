#include <gtest/gtest.h>

#include <common/threads/thread_manager.h>

std::shared_ptr<common::threads::Thread<void> > some_thread;
void test() {
  auto cur_thr = THREAD_MANAGER()->CurrentThread<void>();
  auto cur_handle = cur_thr->GetHandle();
  ASSERT_EQ(cur_handle, some_thread->GetHandle());
  ASSERT_EQ(cur_handle.GetHandle(), common::threads::PlatformThread::GetCurrentHandle());
  ASSERT_EQ(cur_handle.GetTid(), common::threads::PlatformThread::GetCurrentId());
  ASSERT_TRUE(cur_thr->IsRunning());
  ASSERT_TRUE(THREAD_MANAGER()->IsCurrentThread(cur_thr));
  auto main_thread = THREAD_MANAGER()->CurrentThread<int>();
  ASSERT_NE(cur_thr->GetHandle(), main_thread->GetHandle());
}

TEST(THREAD_MANAGER, tests) {
  ASSERT_TRUE(THREAD_MANAGER()->IsMainThread());
  auto main_thread = THREAD_MANAGER()->CurrentThread<int>();
  auto main_handle = main_thread->GetHandle();
  ASSERT_TRUE(THREAD_MANAGER()->IsCurrentThread(main_thread));
  ASSERT_EQ(main_handle.GetHandle(), common::threads::PlatformThread::GetCurrentHandle());
  ASSERT_EQ(main_handle.GetTid(), common::threads::PlatformThread::GetCurrentId());
  ASSERT_TRUE(main_thread->IsRunning());

  some_thread = THREAD_MANAGER()->CreateThread(&test);
  ASSERT_TRUE(some_thread->Start());
  some_thread->Join();
}
