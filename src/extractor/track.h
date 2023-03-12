#pragma once

#include "event.h"

#include <vector>


struct Track {
  std::vector<Event> events{};
  u32 length;

  Track(const u8* data);

  void Parse(const u8* data);

private:
  void PostProcess();
};