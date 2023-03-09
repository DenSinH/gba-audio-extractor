#pragma once

#include "types.h"

#include <array>
#include <cmath>

static inline double CgbMidiKeyToFreq(int midi_key, u8 pitch) {
  const double corrected_key = (midi_key - 69.0 + ((double)pitch / 256.0)) / 12.0;  // An3
  return std::pow(2, corrected_key) * 440;
}

static inline double MidiKeyToFreq(u32 freq, u8 midi_key, u8 pitch) {
  const double corrected_key = (midi_key - 60.0 + ((double)pitch / 256.0)) / 12.0;  // Cn3
  return std::pow(2, corrected_key) * (freq / 1024.0);
}