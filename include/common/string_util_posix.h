#pragma once

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

namespace common {

// Chromium code style is to not use malloc'd strings; this is only for use
// for interaction with APIs that require it.
inline char* strdup(const char* str) {
  return ::strdup(str);
}

inline int strcasecmp(const char* string1, const char* string2) {
  return ::strcasecmp(string1, string2);
}

inline int strncasecmp(const char* string1, const char* string2, size_t count) {
  return ::strncasecmp(string1, string2, count);
}

inline const char* strcasestr(const char* s, const char* find) {
  return ::strcasestr(s, find);
}

inline int vsnprintf(char* buffer, size_t size, const char* format, va_list arguments) {
  return ::vsnprintf(buffer, size, format, arguments);
}

inline int vasprintf(char** ptr, const char* format, va_list ap) {
  int ret;
  va_list ap2;

  va_copy(ap2, ap);
  ret = vsnprintf(NULL, 0, format, ap2);
  va_end(ap2);
  if (ret < 0) {
    return ret;
  }

  (*ptr) = (char*)malloc(ret + 1);
  if (!*ptr) {
    return -1;
  }

  va_copy(ap2, ap);
  ret = vsnprintf(*ptr, ret + 1, format, ap2);
  va_end(ap2);

  return ret;
}

inline int strncmp16(const char16* s1, const char16* s2, size_t count) {
#if defined(WCHAR_T_IS_UTF16)
  return ::wcsncmp(s1, s2, count);
#elif defined(WCHAR_T_IS_UTF32)
  return c16memcmp(s1, s2, count);
#endif
}

inline int vswprintf(wchar_t* buffer, size_t size, const wchar_t* format, va_list arguments) {
  DCHECK(IsWprintfFormatPortable(format));
  return ::vswprintf(buffer, size, format, arguments);
}

}  // namespace common
