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
    KeysplitAltRhythm = 0x80,
  };

  u8 type;  // value depends on subtype
  u8 base;
  u8 pan;
  u8 attack;
  u8 decay;
  u8 sustain;
  u8 release;

  virtual ~Voice() = default;

  virtual double WaveForm(int midi_key, double dt) const = 0;
  void CalculateEnvelope();

private:

};


struct ProgrammableWave final : public Voice {
  // always 16 bytes
  std::array<u8, 16> samples{};

  double WaveForm(int midi_key, double dt) const final;
};

struct DirectSound : public Voice {
  std::vector<u8> samples;
  u32 freq;
  u32 loop_start;

  double WaveForm(int midi_key, double dt) const final;
};

struct Square : public Voice {
  u8 sweep;
  u8 duty_cycle;

  double WaveForm(int midi_key, double dt) const final;

private:
  static constexpr std::array<u8, 4> DutyCycle {
    0b1000'0000,
    0b1100'0000,
    0b1111'0000,
    0b1111'1100,
  };
};

struct Noise : public Voice {
  u8 period;

  double WaveForm(int midi_key, double dt) const final;
};

struct VoiceGroup {
  explicit VoiceGroup(const u8* data) : data{data}, original_data{data} {}

  const Voice& operator[](u32 index) const;

private:
  const u8* original_data;

  // we parse lazily, so we make this mutable
  mutable const u8* data;
  mutable std::vector<std::unique_ptr<Voice>> voices;

  std::unique_ptr<Voice> ParseNext() const;
};

struct Keysplit : public Voice {
  std::unique_ptr<VoiceGroup> split;  // nullptr ==> self referential
  const u8* table;

  double WaveForm(int midi_key, double dt) const final;
};

