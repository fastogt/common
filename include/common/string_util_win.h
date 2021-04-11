#pragma once

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <wchar.h>

#include <common/types.h>

namespace common {

// Chromium code style is to not use malloc'd strings; this is only for use
// for interaction with APIs that require it.
inline char* strdup(const char* str) {
  return ::strdup(str);
}

inline int strcasecmp(const char* s1, const char* s2) {
  return ::strcasecmp(s1, s2);
}

inline int strncasecmp(const char* s1, const char* s2, size_t count) {
  return ::strncasecmp(s1, s2, count);
}

inline const char* strcasestr(const char* s, const char* find) {
  char c, sc;
  if ((c = *find++) != 0) {
    c = tolower((unsigned char)c);
    size_t len = strlen(find);
    do {
      do {
        if ((sc = *s++) == 0)
          return nullptr;
      } while ((char)tolower((unsigned char)sc) != c);
    } while (strncasecmp(s, find, len) != 0);
    s--;
  }
  return s;
}

inline int strncmp16(const char16* s1, const char16* s2, size_t count) {
  return ::wcsncmp(s1, s2, count);
}

inline int vsnprintf(char* buffer, size_t size, const char* format, va_list arguments) {
#if defined(COMPILER_MINGW)
  return ::vsnprintf(buffer, size, format, arguments);
#else
  int length = _vsprintf_p(buffer, size, format, arguments);
  if (length < 0) {
    if (size > 0)
      buffer[0] = 0;
    return _vscprintf_p(format, arguments);
  }
  return length;
#endif
}

inline int vasprintf(char** ptr, const char* format, va_list ap) {
  int ret;
  va_list ap2;

  va_copy(ap2, ap);
  ret = vsnprintf(nullptr, 0, format, ap2);
  va_end(ap2);
  if (ret < 0) {
    return ret;
  }

  (*ptr) = static_cast<char*>(malloc(ret + 1));
  if (!*ptr) {
    return -1;
  }

  va_copy(ap2, ap);
  ret = vsnprintf(*ptr, ret + 1, format, ap2);
  va_end(ap2);

  return ret;
}

inline int vswprintf(wchar_t* buffer, size_t size, const wchar_t* format, va_list arguments) {
  DCHECK(IsWprintfFormatPortable(format));

  int length = _vswprintf_p(buffer, size, format, arguments);
  if (length < 0) {
    if (size > 0)
      buffer[0] = 0;
    return _vscwprintf_p(format, arguments);
  }
  return length;
}

}  // namespace common
