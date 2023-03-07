#pragma once

#include "types.h"

#include <vector>
#include <string>


struct Extractor {
  void Extract(u32 start, u32 size, u32 header_offset);
};