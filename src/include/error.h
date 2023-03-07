#pragma once

#include <cstdio>

template<typename... Args>
static _Noreturn void Error(const char* format, const Args&... args) {
  std::printf("[ERROR]: ");
  std::printf(format, args...);
  std::printf("\n");
  exit(1);
}

template<typename... Args>
static void Debug(const char* format, const Args&... args) {
  std::printf("[DEBUG]: ");
  std::printf(format, args...);
  std::printf("\n");
}