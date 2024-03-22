#pragma once

#include "types.h"

#include <vector>

#define AGB_ROM_OFFSET 0x0800'0000

extern std::vector<u8> file;

namespace util {

static inline bool ValidPointer(u32 pointer) {
  return (pointer > AGB_ROM_OFFSET) && (pointer < AGB_ROM_OFFSET + file.size());
}

static inline u32 ReducePointer(u32 pointer) {
  return (pointer < AGB_ROM_OFFSET) ? pointer : (pointer - AGB_ROM_OFFSET);
}

static inline const u8* GetPointer(u32 pointer) {
  if (!pointer) return nullptr;
  if (ReducePointer(pointer) > file.size()) return nullptr;
  return &file[ReducePointer(pointer)];
}

}
