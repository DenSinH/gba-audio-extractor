#pragma once

#include <cstdio>

template<typename... Args>
[[noreturn]] static void Error(const char* format, const Args&... args) {
  std::printf("[ERROR]: ");
  if constexpr (sizeof...(args) == 0) {
    std::printf("%s\n", format);
  }
  else {
    std::printf(format, args...);
    std::printf("\n");
  }
  exit(1);
}

template<typename... Args>
static void Debug(const char* format, const Args&... args) {
#ifndef NDEBUG
  std::printf("[DEBUG]: ");
  if constexpr (sizeof...(args) == 0) {
    std::printf("%s\n", format);
  }
  else {
    std::printf(format, args...);
    std::printf("\n");
  }
#endif
}

template<typename... Args>
static void Warn(const char* format, const Args&... args) {
  std::printf("[WARN]: ");
  if constexpr (sizeof...(args) == 0) {
    std::printf("%s\n", format);
  }
  else {
    std::printf(format, args...);
    std::printf("\n");
  }
}