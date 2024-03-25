#pragma once

#include "types.h"

#include <vector>
#include <array>
#include <memory>



struct WaveFormData {
  std::vector<float> xdata{};
  std::vector<float> ydata{};

  void AddPoint(float x, float y) {
    xdata.emplace_back(x);
    ydata.emplace_back(y);
  }
};


struct Voice;

struct VoiceState {
  explicit VoiceState(const Voice* voice, i32 base_key) :
      voice{voice}, base_key{base_key} {

  }

  double GetEnvelopeVolume(double time_since_start, double time_since_release) const;
  void Tick(i32 midi_key, double pitch_adjust, double dt);
  double GetSample(double time_since_release) const;

  WaveFormData GetEnvelopeWaveFormData(i32 duration) const;
  WaveFormData GetWaveFormData() const;

private:
  const Voice* voice = nullptr;

  // keysplit based voices have a base key to override the given key
  i32 base_key = -1;

  // for envelope progression:
  double actual_time = 0;
  // for wave progression:
  double integrated_time = 0;
};

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

  virtual VoiceState GetState(i32 midi_key) const;
  virtual WaveFormData GetWaveFormData() const = 0;

protected:
  friend struct Keysplit;
  friend struct VoiceState;
  friend struct VoiceGroup;

  // get frequency given a midi key (pitch adjusted)
  virtual double GetFrequency(double midi_key) const = 0;

  // waveform based on "integrated time"
  // i.e. time corrected for frequency fluctuations
  virtual double WaveForm(double integrated_time) const = 0;

  // get envelope volume based on time since note start and
  // time since note end
  virtual double GetEnvelopeVolume(double time_since_start, double time_since_release) const = 0;

  // calculate envelope timings only once when created
  virtual void CalculateEnvelope() = 0;

  double attack_time;
  double decay_time;
};


struct DirectSound final : public Voice {
  std::vector<u8> samples;
  u32 freq;
  u32 loop_start;
  bool do_loop;

  WaveFormData GetWaveFormData() const final;

  double GetFrequency(double midi_key) const final;
  double WaveForm(double integrated_time) const final;
  double GetEnvelopeVolume(double dt, double time_since_release) const final;

  void CalculateEnvelope() final;
};


struct CgbVoice : public Voice {
  double GetFrequency(double midi_key) const;
  double GetEnvelopeVolume(double dt, double time_since_release) const final;

  void CalculateEnvelope() final;
};


struct ProgrammableWave final : public CgbVoice {
  // always 16 bytes
  std::array<u8, 16> samples{};

  WaveFormData GetWaveFormData() const final;

  double WaveForm(double integrated_time) const final;
};


struct Square final : public CgbVoice {
  u8 sweep;
  u8 duty_cycle;
  
  WaveFormData GetWaveFormData() const final;

  double WaveForm(double integrated_time) const final;

private:
  static constexpr std::array<u8, 4> DutyCycle {
    0b1000'0000,
    0b1100'0000,
    0b1111'0000,
    0b1111'1100,
  };
};


struct Noise final : public CgbVoice {
  u8 period;
  
  WaveFormData GetWaveFormData() const final;

  // noise frequency is special
  double GetFrequency(double midi_key) const final;
  double WaveForm(double integrated_time) const final;
};


struct VoiceGroup;

struct Keysplit final : public Voice {
  std::unique_ptr<VoiceGroup> split;  // nullptr ==> self referential
  const u8* table;

  // get state for nested voice
  VoiceState GetState(i32 midi_key) const final;

  // these will all raise an error, since
  // there is no waveform / envelope for a keysplit voice
  // as it is not an actual voice
  WaveFormData GetWaveFormData() const final;
  double GetFrequency(double midi_key) const final;
  double WaveForm(double integrated_time) const final;
  double GetEnvelopeVolume(double dt, double time_since_release) const final;

  // there is no envelope volume for a keysplit voice
  // as it is not an actual voice
  void CalculateEnvelope() final { }

private:
  const Voice& GetVoice(int midi_key) const;
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
