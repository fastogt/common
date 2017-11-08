#include <gtest/gtest.h>

#include "test_thread_func.h"

#include <common/utils.h>

#include <common/threads/thread_manager.h>

TEST(Thread, common) {
  auto c1 = THREAD_MANAGER()->CreateThread(&test);
  GTEST_ASSERT_EQ(c1->GetHandle(), common::threads::invalid_thread_handle());
  bool res = c1->Start();
  DCHECK(res);
  c1->Join();
  common::threads::PlatformThreadHandle hand = c1->GetHandle();
  GTEST_ASSERT_EQ(hand, common::threads::invalid_thread_handle());
}

#ifndef NDEBUG

void testIsCurrent(void** thr) {
  common::threads::Thread<void>* ct = (common::threads::Thread<void>*)(*thr);
  common::threads::Thread<void>* curthr = THREAD_MANAGER()->CurrentThread<void>();
  ASSERT_NE(curthr, ct);
}

TEST(Thread, isCurrentThread) {
  void* thr = THREAD_MANAGER()->CurrentThread<void>();
  auto c1 = THREAD_MANAGER()->CreateThread(&testIsCurrent, &thr);
  GTEST_ASSERT_EQ(c1->GetHandle(), common::threads::invalid_thread_handle());
  bool res = c1->Start();
  DCHECK(res);
  c1->Join();
  common::threads::PlatformThreadHandle hand = c1->GetHandle();
  GTEST_ASSERT_EQ(hand, common::threads::invalid_thread_handle());
}

#endif

void sleep5Sec() {
  common::threads::PlatformThread::Sleep(5000);
}

int valueGetFunc() {
  return 5;
}

TEST(Thread, valueAfterExec) {
  auto c1 = THREAD_MANAGER()->CreateThread(&valueGetFunc);
  GTEST_ASSERT_EQ(c1->GetHandle(), common::threads::invalid_thread_handle());
  bool res = c1->Start();
  DCHECK(res);
  int result = c1->JoinAndGet();
  GTEST_ASSERT_EQ(result, 5);
  common::threads::PlatformThreadHandle hand = c1->GetHandle();
  GTEST_ASSERT_EQ(hand, common::threads::invalid_thread_handle());
}

class A {
 public:
  A() {}
  virtual int run() { return 1; }
  virtual ~A() {}

 private:
  DISALLOW_COPY_AND_ASSIGN(A);
};

TEST(Thread, classMethod) {
  A a;
  auto c1 = THREAD_MANAGER()->CreateThread(&A::run, &a);
  GTEST_ASSERT_EQ(c1->GetHandle(), common::threads::invalid_thread_handle());
  bool res = c1->Start();
  DCHECK(res);
  int result = c1->JoinAndGet();
  GTEST_ASSERT_EQ(result, 1);
  common::threads::PlatformThreadHandle hand = c1->GetHandle();
  GTEST_ASSERT_EQ(hand, common::threads::invalid_thread_handle());
}

class B : public A {
 public:
  B() {}
  virtual int run() { return 2; }

 private:
  DISALLOW_COPY_AND_ASSIGN(B);
};

TEST(Thread, classVirtualMethod) {
  A* b = new B;
  auto c1 = THREAD_MANAGER()->CreateThread(&A::run, b);
  GTEST_ASSERT_EQ(c1->GetHandle(), common::threads::invalid_thread_handle());
  bool res = c1->Start();
  DCHECK(res);
  int result = c1->JoinAndGet();
  GTEST_ASSERT_EQ(result, 2);
  common::threads::PlatformThreadHandle hand = c1->GetHandle();
  GTEST_ASSERT_EQ(hand, common::threads::invalid_thread_handle());

  /*auto c2 = THREAD_MANAGER()->CreateThread(&B::run, b);
  GTEST_ASSERT_EQ(c2->tid(), fastocpu::invalid_tid);
  c2->start();
  result = c2->JoinAndGet();
  GTEST_ASSERT_EQ(result, 2);
  tid = c2->tid();
  GTEST_ASSERT_EQ(tid, fastocpu::invalid_tid);*/

  B* bb = dynamic_cast<B*>(b);
  auto c3 = THREAD_MANAGER()->CreateThread(&B::run, bb);
  GTEST_ASSERT_EQ(c3->GetHandle(), common::threads::invalid_thread_handle());
  bool rses = c3->Start();
  DCHECK(rses);
  result = c3->JoinAndGet();
  GTEST_ASSERT_EQ(result, 2);
  hand = c3->GetHandle();
  GTEST_ASSERT_EQ(hand, common::threads::invalid_thread_handle());
  delete b;
}

int testint() {
  return 1;
}

TEST(Thread, afterstartValidTid) {
  std::shared_ptr<common::threads::Thread<int> > tp[THREAD_COUNTS];

  for (size_t i = 0; i < SIZEOFMASS(tp); ++i) {
    tp[i] = THREAD_MANAGER()->CreateThread(&testint);
    GTEST_ASSERT_EQ(tp[i]->GetHandle(), common::threads::invalid_thread_handle());
    bool ress = tp[i]->Start();
    DCHECK(ress);
    int res = tp[i]->JoinAndGet();
    GTEST_ASSERT_EQ(res, 1);
    GTEST_ASSERT_EQ(tp[i]->GetHandle(), common::threads::invalid_thread_handle());
  }
}
