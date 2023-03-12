#pragma once

#include "types.h"

#include <array>
#include <cmath>

// pitch adjust is incorporated in midi_key
static inline double CgbMidiKeyToFreq(double midi_key) {
  const double corrected_key = (midi_key - 69.0) / 12.0;  // An3
  return std::pow(2.0, corrected_key) * 440;
}

// pitch adjust is incorporated in midi_key
static inline double MidiKeyToFreq(u32 base, double midi_key) {
  const double corrected_key = (midi_key - 60.0) / 12.0;  // Cn3
  return std::pow(2.0, corrected_key) * (base / 1024.0);
}