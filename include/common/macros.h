/*  Copyright (C) 2014-2016 FastoGT. All right reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are
    met:

        * Redistributions of source code must retain the above copyright
    notice, this list of conditions and the following disclaimer.
        * Redistributions in binary form must reproduce the above
    copyright notice, this list of conditions and the following disclaimer
    in the documentation and/or other materials provided with the
    distribution.
        * Neither the name of FastoGT. nor the names of its
    contributors may be used to endorse or promote products derived from
    this software without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
    "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
    LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
    A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
    OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
    SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
    LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
    DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
    THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
    (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
    OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/

#pragma once

#include <common/logger.h>

#define INVALID_DESCRIPTOR -1
#define ERROR_RESULT_VALUE -1
#define ZERO_RESULT_VALUE 0
#define SUCCESS_RESULT_VALUE ZERO_RESULT_VALUE

#if defined(OS_WIN)
#if defined(COMPILER_MINGW)
typedef int descriptor_t;
#elif defined(COMPILER_MSVC)
typedef HANDLE descriptor_t;
#endif
#else
typedef int descriptor_t;
#endif

#define INVALID_TID std::thread::id()
#define PTHREAD_INVALID_TID 0

#ifndef PRIuS
#define PRIuS "zu"
#endif

// Processor architecture detection.  For more info on what's defined, see:
//   http://msdn.microsoft.com/en-us/library/b0084kay.aspx
//   http://www.agner.org/optimize/calling_conventions.pdf
//   or with gcc, run: "echo | gcc -E -dM -"
#if defined(_M_X64) || defined(__x86_64__)
#define ARCH_CPU_X86_FAMILY 1
#define ARCH_CPU_X86_64 1
#define ARCH_CPU_64_BITS 1
#define ARCH_CPU_LITTLE_ENDIAN 1
#define WORDSIZE 64
#elif defined(_M_IX86) || defined(__i386__)
#define ARCH_CPU_X86_FAMILY 1
#define ARCH_CPU_X86 1
#define ARCH_CPU_32_BITS 1
#define ARCH_CPU_LITTLE_ENDIAN 1
#define WORDSIZE 32
#elif defined(__ARMEL__)
#define ARCH_CPU_ARM_FAMILY 1
#define ARCH_CPU_ARMEL 1
#define ARCH_CPU_32_BITS 1
#define ARCH_CPU_LITTLE_ENDIAN 1
#define WORDSIZE 32
#elif defined(__aarch64__)
#define ARCH_CPU_ARM_FAMILY 1
#define ARCH_CPU_ARM64 1
#define ARCH_CPU_64_BITS 1
#define ARCH_CPU_LITTLE_ENDIAN 1
#define WORDSIZE 64
#elif defined(__pnacl__)
#define ARCH_CPU_32_BITS 1
#define ARCH_CPU_LITTLE_ENDIAN 1
#define WORDSIZE 32
#elif defined(__MIPSEL__)
#define ARCH_CPU_MIPS_FAMILY 1
#define ARCH_CPU_MIPSEL 1
#define ARCH_CPU_32_BITS 1
#define ARCH_CPU_LITTLE_ENDIAN 1
#define WORDSIZE 32
#else
#error Please add support for your architecture
#endif

// Type detection for wchar_t.
#if defined(OS_WIN)
#define WCHAR_T_IS_UTF16
#elif defined(OS_POSIX) && (defined(COMPILER_GCC) || defined(COMPILER_CLANG)) && defined(__WCHAR_MAX__) && \
    (__WCHAR_MAX__ == 0x7fffffff || __WCHAR_MAX__ == 0xffffffff)
#define WCHAR_T_IS_UTF32
#define UNICODE
#elif defined(OS_POSIX) && (defined(COMPILER_GCC) || defined(COMPILER_CLANG)) && defined(__WCHAR_MAX__) && \
    (__WCHAR_MAX__ == 0x7fff || __WCHAR_MAX__ == 0xffff)
// On Posix, we'll detect short wchar_t, but projects aren't guaranteed to
// compile in this mode (in particular, Chrome doesn't). This is intended for
// other projects using base who manage their own dependencies and make sure
// short wchar works for them.
#define WCHAR_T_IS_UTF16
#else
#error Please add support for your compiler in build/build_config.h
#endif

#if defined(COMPILER_GCC) || defined(COMPILER_CLANG)
#define WARN_UNUSED_RESULT __attribute__((warn_unused_result))
#define PRINTF_FORMAT(format_param, dots_param) __attribute__((format(printf, format_param, dots_param)))
#else
#define WARN_UNUSED_RESULT
#define PRINTF_FORMAT(format_param, dots_param)
#endif

void immediate_assert();
void immediate_exit();

#define STRINGIZE_HELPER(x) #x
#define STRINGIZE(x) STRINGIZE_HELPER(x)

#define LINE_TEXT __FILE__ ":" STRINGIZE(__LINE__)

#define UNUSED(x) (void)x

#if defined(NDEBUG) && !defined(DCHECK_ALWAYS_ON)
#define DCHECK_IS_ON 0
#else
#define DCHECK_IS_ON 1
#endif

#define DCHECK(x) LAZY_STREAM(LOG_FILE_LINE_STREAM(CRIT), DCHECK_IS_ON && !(x)) << "Check failed: " #x "."
#define DCHECK_EQ(x, y) DCHECK((x == y))
#define DCHECK_NE(x, y) DCHECK((x != y))
#define DCHECK_LE(x, y) DCHECK((x <= y))
#define DCHECK_LT(x, y) DCHECK((x < y))
#define DCHECK_GE(x, y) DCHECK((x >= y))
#define DCHECK_GT(x, y) DCHECK((x > y))

#define CHECK(x) LAZY_STREAM(LOG_FILE_LINE_STREAM(CRIT), !(x)) << "Check failed: " #x "."
#define CHECK_EQ(x, y) CHECK((x == y))
#define CHECK_NE(x, y) CHECK((x != y))
#define CHECK_LE(x, y) CHECK((x <= y))
#define CHECK_LT(x, y) CHECK((x < y))
#define CHECK_GE(x, y) CHECK((x >= y))
#define CHECK_GT(x, y) CHECK((x > y))

#define DNOTREACHED() \
  LAZY_STREAM(LOG_FILE_LINE_STREAM(CRIT), DCHECK_IS_ON) << "DNOTREACHED() hit in " << __FUNCTION__ << "."
#define NOTREACHED() LOG_FILE_LINE_STREAM(CRIT) << "NOTREACHED() hit in " << __FUNCTION__ << "."

#if defined(NDEBUG)
#define VERIFY(x) LAZY_STREAM(LOG_FILE_LINE_STREAM(ERROR), !(x)) << "Verify failed: " #x "."
#else
#define VERIFY(x) DCHECK(x)
#endif

#define NOOP() asm("nop")

#define PROJECT_VERSION_CHECK(major, minor, patch) ((major << 16) | (minor << 8) | (patch))
#define PROJECT_GET_MAJOR_VERSION(version) ((version >> 16) & 0xFF)
#define PROJECT_GET_MINOR_VERSION(version) ((version >> 8) & 0xFF)
#define PROJECT_GET_PATCH_VERSION(version) (version & 0xFF)
#define PROJECT_VERSION_GENERATE PROJECT_VERSION_CHECK

// Put this in the declarations for a class to be uncopyable.
#define DISALLOW_COPY(TypeName) TypeName(const TypeName&) = delete
// Put this in the declarations for a class to be unassignable.
#define DISALLOW_ASSIGN(TypeName) void operator=(const TypeName&) = delete
// A macro to disallow the copy constructor and operator= functions.
// This should be used in the private: declarations for a class.
#define DISALLOW_COPY_AND_ASSIGN(TypeName) \
  TypeName(const TypeName&) = delete;      \
  void operator=(const TypeName&) = delete

#define SIZEOFMASS(type) sizeof(type) / sizeof(*type)
#define arraysize(type) SIZEOFMASS(type)

#define COMPILE_ASSERT(expr, msg) static_assert(expr, msg)

#define SIZEOF_DATA_MUST_BE(str, size) \
  COMPILE_ASSERT(sizeof(str) == size, "sizeof " STRINGIZE(str) " must be " STRINGIZE(size) " byte" \
                                                                                           "!")

#if defined(OS_POSIX) || (defined(OS_WIN) && defined(COMPILER_MINGW))
#if defined(NDEBUG)
#define HANDLE_EINTR(x)                                     \
  ({                                                        \
    decltype(x) eintr_wrapper_result;                       \
    do {                                                    \
      eintr_wrapper_result = (x);                           \
    } while (eintr_wrapper_result == -1 && errno == EINTR); \
    eintr_wrapper_result;                                   \
  })
#else
#define HANDLE_EINTR(x)                                                                      \
  ({                                                                                         \
    int eintr_wrapper_counter = 0;                                                           \
    decltype(x) eintr_wrapper_result;                                                        \
    do {                                                                                     \
      eintr_wrapper_result = (x);                                                            \
    } while (eintr_wrapper_result == -1 && errno == EINTR && eintr_wrapper_counter++ < 100); \
    eintr_wrapper_result;                                                                    \
  })
#endif  // NDEBUG

#define IGNORE_EINTR(x)                                   \
  ({                                                      \
    decltype(x) eintr_wrapper_result;                     \
    do {                                                  \
      eintr_wrapper_result = (x);                         \
      if (eintr_wrapper_result == -1 && errno == EINTR) { \
        eintr_wrapper_result = 0;                         \
      }                                                   \
    } while (0);                                          \
    eintr_wrapper_result;                                 \
  })
#else
#define HANDLE_EINTR(x) (x)
#define IGNORE_EINTR(x) (x)
#endif

void* betoh_memcpy(void* dst, const void* src, unsigned int sz);

template <typename T>
inline void destroy(T** v) {
  if (!v) {
    return;
  }

  T* lv = *v;
  if (!lv) {
    return;
  }

  delete lv;
  *v = nullptr;
}
