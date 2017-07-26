#include <gtest/gtest.h>

#include "test_thread_func.h"

#include <common/utils.h>

#include <common/threads/thread_manager.h>

TEST(Thread, common) {
  auto c1 = THREAD_MANAGER()->CreateThread(&test);
  GTEST_ASSERT_EQ(c1->GetTid(), common::threads::invalid_tid);
  bool res = c1->Start();
  DCHECK(res);
  c1->Join();
  common::threads::platform_thread_id_t tid = c1->GetTid();
  GTEST_ASSERT_EQ(tid, common::threads::invalid_tid);
}

#ifndef NDEBUG

void testIsCurrent(void** thr) {
  common::threads::Thread<void>* ct = (common::threads::Thread<void>*)(*thr);
  ASSERT_TRUE(THREAD_MANAGER()->CurrentThread<void>() == ct);
}

TEST(Thread, isCurrentThread) {
  void* thr = NULL;
  auto c1 = THREAD_MANAGER()->CreateThread(&testIsCurrent, &thr);
  thr = c1.get();
  GTEST_ASSERT_EQ(c1->GetTid(), common::threads::invalid_tid);
  bool res = c1->Start();
  DCHECK(res);
  c1->Join();
  common::threads::platform_thread_id_t tid = c1->GetTid();
  GTEST_ASSERT_EQ(tid, common::threads::invalid_tid);
}

#endif

void sleep5Sec() {
  common::utils::msleep(5000);
}

TEST(Thread, outOfScope) {
  auto c1 = THREAD_MANAGER()->CreateThread(&sleep5Sec);
  GTEST_ASSERT_EQ(c1->GetTid(), common::threads::invalid_tid);
  bool res = c1->Start();
  DCHECK(res);
}

int valueGetFunc() {
  return 5;
}

TEST(Thread, valueAfterExec) {
  auto c1 = THREAD_MANAGER()->CreateThread(&valueGetFunc);
  GTEST_ASSERT_EQ(c1->GetTid(), common::threads::invalid_tid);
  bool res = c1->Start();
  DCHECK(res);
  int result = c1->JoinAndGet();
  GTEST_ASSERT_EQ(result, 5);
  common::threads::platform_thread_id_t tid = c1->GetTid();
  GTEST_ASSERT_EQ(tid, common::threads::invalid_tid);
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
  GTEST_ASSERT_EQ(c1->GetTid(), common::threads::invalid_tid);
  bool res = c1->Start();
  DCHECK(res);
  int result = c1->JoinAndGet();
  GTEST_ASSERT_EQ(result, 1);
  common::threads::platform_thread_id_t tid = c1->GetTid();
  GTEST_ASSERT_EQ(tid, common::threads::invalid_tid);
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
  GTEST_ASSERT_EQ(c1->GetTid(), common::threads::invalid_tid);
  bool res = c1->Start();
  DCHECK(res);
  int result = c1->JoinAndGet();
  GTEST_ASSERT_EQ(result, 2);
  common::threads::platform_thread_id_t tid = c1->GetTid();
  GTEST_ASSERT_EQ(tid, common::threads::invalid_tid);

  /*auto c2 = THREAD_MANAGER()->CreateThread(&B::run, b);
  GTEST_ASSERT_EQ(c2->tid(), fastocpu::invalid_tid);
  c2->start();
  result = c2->JoinAndGet();
  GTEST_ASSERT_EQ(result, 2);
  tid = c2->tid();
  GTEST_ASSERT_EQ(tid, fastocpu::invalid_tid);*/

  B* bb = dynamic_cast<B*>(b);
  auto c3 = THREAD_MANAGER()->CreateThread(&B::run, bb);
  GTEST_ASSERT_EQ(c3->GetTid(), common::threads::invalid_tid);
  bool rses = c3->Start();
  DCHECK(rses);
  result = c3->JoinAndGet();
  GTEST_ASSERT_EQ(result, 2);
  tid = c3->GetTid();
  GTEST_ASSERT_EQ(tid, common::threads::invalid_tid);
  delete b;
}

int testint() {
  return 1;
}

TEST(Thread, afterstartValidTid) {
  std::shared_ptr<common::threads::Thread<int> > tp[THREAD_COUNTS];

  for (size_t i = 0; i < SIZEOFMASS(tp); ++i) {
    tp[i] = THREAD_MANAGER()->CreateThread(&testint);
    GTEST_ASSERT_EQ(tp[i]->GetTid(), common::threads::invalid_tid);
    bool ress = tp[i]->Start();
    DCHECK(ress);
    int res = tp[i]->JoinAndGet();
    GTEST_ASSERT_EQ(res, 1);
    GTEST_ASSERT_EQ(tp[i]->GetTid(), common::threads::invalid_tid);
  }
}
