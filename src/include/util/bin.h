#pragma once

#include "types.h"

namespace util {

template<typename T>
static inline T Read(const u8* data) {
  T result = 0;
  for (int i = 0; i < sizeof(T); i++) {
    result |= (T(data[i]) << (8 * i));
  }
  return result;
}

}