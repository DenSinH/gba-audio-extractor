#pragma once

#include "types.h"

#include <vector>
#include <array>
#include <memory>


struct Voice {
  enum Type : u8 {
    DirectSound = 0,
    DirectSoundNoResample = 8,
    DirectSoundAlt = 16,
    Square = 1,
    SquareAlt = 9,
    Square2 = 2,
    Square2Alt = 10,
    Wave = 3,
    WaveAlt = 11,
    Noise = 4,
    NoiseAlt = 12,
    Keysplit = 0x40,
    KeysplitAlt = 0x80,
  };

  u8 type;  // value depends on subtype
  u8 base;
  u8 pan;
  u8 attack;
  u8 decay;
  u8 sustain;
  u8 release;
};


struct ProgrammableWave : public Voice {
  // always 16 bytes
  std::array<u8, 16> samples{};
};

struct DirectSound : public Voice {
  std::vector<u8> samples;
  u32 loop_start;
};

struct Square : public Voice {
  u8 sweep;
  u8 duty_cycle;
};

struct Noise : public Voice {
  u8 period;
};

struct VoiceGroup {
  explicit VoiceGroup(const u8* data) : data{data}, original_data{data} {}

  const Voice& operator[](u32 index);

private:
  const u8* data;
  const u8* original_data;
  std::vector<std::unique_ptr<Voice>> voices;

  std::unique_ptr<Voice> ParseNext();
};

struct Keysplit : public Voice {
  std::unique_ptr<VoiceGroup> split;  // nullptr ==> self referential
  void* table;
};

