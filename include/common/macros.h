/*  Copyright (C) 2014-2020 FastoGT. All right reserved.

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

#include <ostream>

#include <common/logger.h>

#define INVALID_PID -1
#define INVALID_DESCRIPTOR -1
#define ERROR_RESULT_VALUE -1
#define ZERO_RESULT_VALUE 0
#define SUCCESS_RESULT_VALUE ZERO_RESULT_VALUE

#if defined(OS_WIN)
#define INVALID_PROCESS_ID nullptr
#if defined(COMPILER_MINGW)
typedef int descriptor_t;
#elif defined(COMPILER_MSVC)
typedef HANDLE descriptor_t;
#endif
#else
#define INVALID_PROCESS_ID -1
typedef int descriptor_t;
#endif

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
#elif defined(_M_IX86) || defined(__i386__)
#define ARCH_CPU_X86_FAMILY 1
#define ARCH_CPU_X86 1
#define ARCH_CPU_32_BITS 1
#define ARCH_CPU_LITTLE_ENDIAN 1
#elif defined(__s390x__)
#define ARCH_CPU_S390_FAMILY 1
#define ARCH_CPU_S390X 1
#define ARCH_CPU_64_BITS 1
#define ARCH_CPU_BIG_ENDIAN 1
#elif defined(__s390__)
#define ARCH_CPU_S390_FAMILY 1
#define ARCH_CPU_S390 1
#define ARCH_CPU_31_BITS 1
#define ARCH_CPU_BIG_ENDIAN 1
#elif (defined(__PPC64__) || defined(__PPC__)) && defined(__BIG_ENDIAN__)
#define ARCH_CPU_PPC64_FAMILY 1
#define ARCH_CPU_PPC64 1
#define ARCH_CPU_64_BITS 1
#define ARCH_CPU_BIG_ENDIAN 1
#elif defined(__PPC64__)
#define ARCH_CPU_PPC64_FAMILY 1
#define ARCH_CPU_PPC64 1
#define ARCH_CPU_64_BITS 1
#define ARCH_CPU_LITTLE_ENDIAN 1
#elif defined(__ARMEL__)
#define ARCH_CPU_ARM_FAMILY 1
#define ARCH_CPU_ARMEL 1
#define ARCH_CPU_32_BITS 1
#define ARCH_CPU_LITTLE_ENDIAN 1
#elif defined(__aarch64__)
#define ARCH_CPU_ARM_FAMILY 1
#define ARCH_CPU_ARM64 1
#define ARCH_CPU_64_BITS 1
#define ARCH_CPU_LITTLE_ENDIAN 1
#elif defined(__pnacl__) || defined(__asmjs__)
#define ARCH_CPU_32_BITS 1
#define ARCH_CPU_LITTLE_ENDIAN 1
#elif defined(__MIPSEL__)
#if defined(__LP64__)
#define ARCH_CPU_MIPS_FAMILY 1
#define ARCH_CPU_MIPS64EL 1
#define ARCH_CPU_64_BITS 1
#define ARCH_CPU_LITTLE_ENDIAN 1
#else
#define ARCH_CPU_MIPS_FAMILY 1
#define ARCH_CPU_MIPSEL 1
#define ARCH_CPU_32_BITS 1
#define ARCH_CPU_LITTLE_ENDIAN 1
#endif
#elif defined(__MIPSEB__)
#if defined(__LP64__)
#define ARCH_CPU_MIPS_FAMILY 1
#define ARCH_CPU_MIPS64 1
#define ARCH_CPU_64_BITS 1
#define ARCH_CPU_BIG_ENDIAN 1
#else
#define ARCH_CPU_MIPS_FAMILY 1
#define ARCH_CPU_MIPS 1
#define ARCH_CPU_32_BITS 1
#define ARCH_CPU_BIG_ENDIAN 1
#endif
#else
#error Please add support for your architecture
#endif

#ifdef ARCH_CPU_64_BITS
#define WORDSIZE 64
#elif defined(ARCH_CPU_32_BITS)
#define WORDSIZE 32
#else
#error Please add word site for your architecture
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
// compile in this mode (in particular, Common doesn't). This is intended for
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

#define CHAR_BIT __CHAR_BIT__

// Macro for hinting that an expression is likely to be false.
#if !defined(UNLIKELY)
#if defined(COMPILER_GCC) || defined(COMPILER_CLANG)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define UNLIKELY(x) (x)
#endif  // defined(COMPILER_GCC)
#endif  // !defined(UNLIKELY)
#if !defined(LIKELY)
#if defined(COMPILER_GCC) || defined(COMPILER_CLANG)
#define LIKELY(x) __builtin_expect(!!(x), 1)
#else
#define LIKELY(x) (x)
#endif  // defined(COMPILER_GCC)
#endif  // !defined(LIKELY)

void immediate_assert();
void immediate_exit();

#define STRINGIZE_HELPER(x) #x
#define STRINGIZE(x) STRINGIZE_HELPER(x)

#define LINE_TEXT __FILE__ ":" STRINGIZE(__LINE__)

#if defined(COMPILER_GCC) || defined(COMPILER_CLANG)
#define ALLOW_UNUSED_TYPE __attribute__((unused))
#else
#define ALLOW_UNUSED_TYPE
#endif

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
#define VERIFY(x) LAZY_STREAM(LOG_FILE_LINE_STREAM(ERR), !(x)) << "Verify failed: " #x "."
#else
#define VERIFY(x) DCHECK(x)
#endif

#define NOOP() asm("nop")

#define PROJECT_VERSION_CHECK_FULL(major, minor, patch, tweak) ((major << 24) | (minor << 16) | (patch << 8) | tweak)
#define PROJECT_VERSION_CHECK(major, minor, patch) PROJECT_VERSION_CHECK_FULL(major, minor, patch, 0)
#define PROJECT_GET_MAJOR_VERSION(version) ((version >> 24) & 0xFF)
#define PROJECT_GET_MINOR_VERSION(version) ((version >> 16) & 0xFF)
#define PROJECT_GET_PATCH_VERSION(version) ((version >> 8) & 0xFF)
#define PROJECT_GET_TWEAK_VERSION(version) (version & 0xFF)

#define PROJECT_VERSION_GENERATE_FULL PROJECT_VERSION_CHECK_FULL
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
  COMPILE_ASSERT(sizeof(str) == size, "sizeof " STRINGIZE(str) " must be " STRINGIZE(size) " byte!")

#if defined(__clang__) && __has_attribute(uninitialized)
// Attribute "uninitialized" disables -ftrivial-auto-var-init=pattern for
// the specified variable.
// Library-wide alternative is
// 'configs -= [ "//build/config/compiler:default_init_stack_vars" ]' in .gn
// file.
//
// See "init_stack_vars" in build/config/compiler/BUILD.gn and
// http://crbug.com/977230
// "init_stack_vars" is enabled for non-official builds and we hope to enable it
// in official build in 2020 as well. The flag writes fixed pattern into
// uninitialized parts of all local variables. In rare cases such initialization
// is undesirable and attribute can be used:
//   1. Degraded performance
// In most cases compiler is able to remove additional stores. E.g. if memory is
// never accessed or properly initialized later. Preserved stores mostly will
// not affect program performance. However if compiler failed on some
// performance critical code we can get a visible regression in a benchmark.
//   2. memset, memcpy calls
// Compiler may replaces some memory writes with memset or memcpy calls. This is
// not -ftrivial-auto-var-init specific, but it can happen more likely with the
// flag. It can be a problem if code is not linked with C run-time library.
//
// Note: The flag is security risk mitigation feature. So in future the
// attribute uses should be avoided when possible. However to enable this
// mitigation on the most of the code we need to be less strict now and minimize
// number of exceptions later. So if in doubt feel free to use attribute, but
// please document the problem for someone who is going to cleanup it later.
// E.g. platform, bot, benchmark or test name in patch description or next to
// the attribute.
#define STACK_UNINITIALIZED __attribute__((uninitialized))
#else
#define STACK_UNINITIALIZED
#endif

#define PLUS_INF "inf"
#define PPLUS_INF "+inf"
#define MINUS_INF "-inf"

// Used to explicitly mark the return value of a function as unused. If you are
// really sure you don't want to do anything with the return value of a function
// that has been marked WARN_UNUSED_RESULT, wrap it with this. Example:
//
//   std::unique_ptr<MyType> my_var = ...;
//   if (TakeOwnership(my_var.get()) == SUCCESS)
//     ignore_result(my_var.release());
//
template <typename T>
inline void ignore_result(const T&) {}

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

template <typename T>
constexpr inline T stable_value_in_range(T a, T amin, T amax) {
  return (a < amin) ? amin : (a > amax) ? amax : a;
}

template <typename T, T amin, T amax>
struct stable_value {
  static constexpr T value(T a) {
    COMPILE_ASSERT(amin <= amax, "max should be greater or equal min");
    return stable_value_in_range(a, amin, amax);
  }
};

template <typename init_func, typename destroy_func>
struct RAII {
  typedef init_func init_closure;
  typedef destroy_func destroy_closure;

  RAII() { init_closure(); }
  ~RAII() { destroy_closure(); }
};
