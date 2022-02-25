/// \file
/// \brief General purpose utilities

#pragma once

#include <cassert>
#include <cstdio>
#include <stdexcept>

#ifdef DEBUG
  #define ALWAYS_INLINE
#else
  #define ALWAYS_INLINE __attribute__((__always_inline__))
#endif

#define LIKELY(expr) __builtin_expect(!!(expr), true)
#define UNLIKELY(expr) __builtin_expect(!!(expr), false)

#define UNREACHABLE()                                                          \
  do {                                                                         \
    assert(false && "UNREACHABLE was reached");                                \
    __builtin_unreachable();                                                   \
  } while (false)

template <auto F>
struct deleter_from_fn {
  template <typename T>
  constexpr void operator()(T* arg) const { F(arg); }
};

#if !defined(NDEBUG)
  #define VERIFY assert
#else
  #define VERIFY(expr) do { (void)(expr); } while (0)
#endif

/// Returns formatted string (sprintf() behavior).
template <typename... Args>
std::string format(const char *fmt, Args... args) {
  // TODO: make safe with C++ streams and parameter unpacking
  // NOTE: use std::format (C++20) or boost::format instead

  // This implementation does not precalculate formatted string size for sake of
  // performance and simplicity. It just uses some resonable size; too long
  // strings are returned truncated.

  char buf[1024]; // some sane size to hold formatted string
  // snprintf() writes at most sizeof(buf) - 1 chars, ensures zero termination
  int chars_written = snprintf(buf, sizeof(buf), fmt, args...);
  if (chars_written < 0) {
    throw std::runtime_error("std::snprintf()'s internal error.");
  }
  return buf;
}

template <typename... Args>
[[noreturn]] void error(const char *fmt, Args... args) {
  throw std::runtime_error(format(fmt, args...));
}

#if !defined(NO_DPRINT)
  #define DPRINT(fmt, ...) [&](const char *func) \
    { fprintf(stderr, "%s(): " fmt "\n", func, ##__VA_ARGS__); }(__func__);
#else
  #define DPRINT(fmt, ...)
#endif

// OK:
//#if !defined(NO_DPRINT)
//template <typename... Args>
//void dprint(const char *fmt, Args... args) {
//  // TODO: make safe with C++ streams and parameter unpacking
//  // NOTE: use std::format (C++20) or boost::format instead

//  // This implementation does not precalculate formatted string size for sake of
//  // performance and simplicity. It just uses some resonable size; too long
//  // strings are truncated.

//  char buf[1024]; // some sane size to hold formatted string
//  // snprintf() writes at most sizeof(buf) - 1 chars, ensures zero termination
//  int chars_written = snprintf(buf, sizeof(buf) - 1, fmt, args...);  // -1 for '\n'
//  assert(chars_written >= 0);
//  buf[chars_written] = '\n';
//  buf[chars_written + 1] = '\0';
//  fprintf(stderr, buf);
//}
//#else
//template <typename... Args>
//void dprint(const char *, Args...) {
//}
//#endif


/* ==TRASH==
*/
