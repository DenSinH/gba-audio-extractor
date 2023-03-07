#pragma once

#include "event.h"

#include <vector>


struct Track {
  std::vector<Event> events{};

  Track(const u8* data);

  void Parse(const u8* data);
};