#pragma once

#include "types.h"

extern const u8* file;

namespace util {

static inline u32 ReducePointer(u32 pointer) {
  return (pointer < 0x0800'0000) ? pointer : (pointer - 0x0800'0000);
}

static inline const u8* GetPointer(u32 pointer) {
  if (!pointer) return nullptr;
  return &file[ReducePointer(pointer)];
}

}
