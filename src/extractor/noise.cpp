#include <array>
#include <vector>
#include <cstdint>

using u8  = std::uint8_t;
using u32 = std::uint32_t;

const std::vector<u8> NoiseSequence15 = []{
  std::vector<u8> noise{};
  std::array<bool, 1 << 15> seen{};

  u32 shift_reg  = 0x4000;
  u32 samples    = 0;
  u8  sample_set = 0;
  while (!seen[shift_reg]) {
    seen[shift_reg] = true;
    sample_set <<= 1;
    sample_set |= shift_reg & 1;

    if (shift_reg & 1) {
      shift_reg >>= 1;
      shift_reg ^= 0x6000;
    }
    else {
      shift_reg >>= 1;
    }

    if (++samples == 8) {
      noise.emplace_back(sample_set);
      sample_set = 0;
    }
  }
  // purposefully ignore any leftover samples

  return noise;
}();

const std::vector<u8> NoiseSequence7 = []{
  std::vector<u8> noise{};
  std::array<bool, 1 << 7> seen{};

  u32 shift_reg  = 0x40;
  u32 samples    = 0;
  u8  sample_set = 0;
  while (!seen[shift_reg]) {
    seen[shift_reg] = true;
    sample_set <<= 1;
    sample_set |= shift_reg & 1;

    if (shift_reg & 1) {
      shift_reg >>= 1;
      shift_reg ^= 0x60;
    }
    else {
      shift_reg >>= 1;
    }

    if (++samples == 8) {
      noise.emplace_back(sample_set);
      sample_set = 0;
    }
  }
  // purposefully ignore any leftover samples

  return noise;
}();