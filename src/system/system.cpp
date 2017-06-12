#include <common/threads/thread.h>

#include <common/threads/thread_manager.h>

namespace common {
namespace threads {

template <typename RT>
void WrapThread(Thread<RT>* thread) {
  THREAD_MANAGER()->WrapThread(thread);
  CHECK(IsCurrentThread(thread));
}

template <typename RT>
void UnWrapThread(Thread<RT>* thread) {
  THREAD_MANAGER()->UnWrapThread(thread);
  CHECK(IsCurrentThread(thread));
}

template <typename RT>
bool IsCurrentThread(Thread<RT>* thread) {
  return THREAD_MANAGER()->IsCurrentThread(thread);
}

template void WrapThread(Thread<void>* thread);
template void UnWrapThread(Thread<void>* thread);
template bool IsCurrentThread(Thread<void>* thread);

template void WrapThread(Thread<int>* thread);
template void UnWrapThread(Thread<int>* thread);
template bool IsCurrentThread(Thread<int>* thread);
}  // namespace threads
}  // namespace common
