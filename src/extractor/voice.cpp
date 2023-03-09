#include "voice.h"
#include "error.h"
#include "util/bin.h"
#include "util/gba.h"

#include "midi.h"
#include <cmath>


double ProgrammableWave::WaveForm(int midi_key, double dt) const {
  // sample rate is 2097152 / 131072 = 16 times the frequency of the noise channel,
  double freq = CgbMidiKeyToFreq(midi_key, 0);

  int sample_index = (int)(16 * freq * dt) % 32;
  u8 sample = samples[sample_index >> 1];
  if (!(sample_index & 1)) {
    sample = sample >> 4;
  }
  return double(sample & 0xf) / 15.0;
}

double DirectSound::WaveForm(int midi_key, double dt) const {
  // compute channel frequency at which samples are requested
  const double chanFreq = MidiKeyToFreq(freq, midi_key, 0);

  // so if time is dt, we are at sample
  const double sampleIndex = dt * chanFreq;
  u32 sampleIndexCourse = u32(sampleIndex);

  // todo: interpolation with:
  const double sampleIndexFine = sampleIndex - sampleIndexCourse;
  if (sampleIndexCourse > samples.size()) {
    sampleIndexCourse = loop_start + (sampleIndexCourse % (samples.size() - loop_start));
  }

  return double(i8(samples[sampleIndexCourse])) / 256.0;
}

double Square::WaveForm(int midi_key, double dt) const {
  double freq = CgbMidiKeyToFreq(midi_key, 0);
  double in_period = (freq * dt) - std::floor(freq * dt);

  return (DutyCycle[duty_cycle] & (0x80 >> u32(in_period * 8))) ? -0.5 : 0.5;
}

double Noise::WaveForm(int midi_key, double dt) const {
  return 0;
}

double Keysplit::WaveForm(int midi_key, double dt) const {
  return 0;
  u8 key = midi_key;
  const Voice* voice;
  if (table) {
    voice = &(*split)[table[key]];
  }
  else {
    voice = &(*split)[key];
  }

  if (voice->type == Voice::Type::KeysplitAltRhythm || voice->type == Voice::Type::Keysplit) {
    // nested keysplit instruments
    return 0;
  }
  if (type == Voice::Type::KeysplitAltRhythm) {
    // todo: pan
    key = voice->base;
  }

  return voice->WaveForm(key, dt);
}

const Voice& VoiceGroup::operator[](u32 index) const {
  if (index >= voices.size()) {
    auto old_size = voices.size();
    voices.resize(index + 1);
    for (auto i = old_size; i < index + 1; i++) {
      voices[i] = ParseNext();
    }
  }
  return *voices[index];
}

std::unique_ptr<Voice> VoiceGroup::ParseNext() const {
  std::unique_ptr<Voice> voice;
  u8 type = *data++;
  u8 base = *data++;

  switch (type) {
    case Voice::Type::DirectSound:
    case Voice::Type::DirectSoundNoResample:
    case Voice::Type::DirectSoundAlt: {
      auto _voice = std::make_unique<DirectSound>();
      data++;  // sbz
      _voice->pan = *data++;

      // struct WaveData
      //{
      //    u16 type;
      //    u16 status;
      //    u32 freq;
      //    u32 loopStart;
      //    u32 size; // number of samples
      //    s8 data[size]; // samples
      //};
      auto* wave_data = util::GetPointer(util::Read<u32>(data));
      data += sizeof(u32);

      // check if type is 0
      if (util::Read<u16>(wave_data)) {
        Error("Unknown directsound wave data type: %x", type);
      }

      _voice->freq       = util::Read<u32>(&wave_data[4]);
      _voice->loop_start = util::Read<u32>(&wave_data[8]);

      u32 size = util::Read<u32>(&wave_data[12]);
      _voice->samples = {&wave_data[16], &wave_data[16 + size]};

      voice = std::move(_voice);
      break;
    }
    case Voice::Type::Square:
    case Voice::Type::SquareAlt:
    case Voice::Type::Square2:
    case Voice::Type::Square2Alt: {
      auto _voice = std::make_unique<Square>();
      _voice->pan = *data++;
      _voice->sweep = *data++;
      _voice->duty_cycle = *data++;

      data++;  // sbz
      data++;  // sbz
      data++;  // sbz

      voice = std::move(_voice);
      break;
    }
    case Voice::Type::Wave:
    case Voice::Type::WaveAlt: {
      auto _voice = std::make_unique<ProgrammableWave>();
      _voice->pan = *data++;

      data++;  // sbz

      auto* wave_data = util::GetPointer(util::Read<u32>(data));
      data += sizeof(u32);

      std::memcpy(_voice->samples.data(), wave_data, _voice->samples.size());

      voice = std::move(_voice);
      break;
    }
    case Voice::Type::Noise:
    case Voice::Type::NoiseAlt: {
      auto _voice = std::make_unique<Noise>();
      data++;  // sbz

      _voice->pan = *data++;
      _voice->period = *data++;

      data++;  // sbz
      data++;  // sbz
      data++;  // sbz

      voice = std::move(_voice);
      break;
    }
    case Voice::Type::Keysplit:
    case Voice::Type::KeysplitAltRhythm: {
      auto _voice = std::make_unique<Keysplit>();
      // already skipped one byte for "base" data
      data++;  // sbz
      data++;  // sbz

      auto* voice_group = util::GetPointer(util::Read<u32>(data));
      data += sizeof(u32);

      _voice->split = (voice_group == original_data) ? nullptr : std::make_unique<VoiceGroup>(voice_group);
      _voice->table = util::GetPointer(util::Read<u32>(data));
      data += sizeof(u32);

      voice = std::move(_voice);
      break;
    }
    default: {
      Error("Unknown type for voice: %x (%d) while parsing index %llu", type, type, voices.size());
    }
  }

  voice->type = type;
  voice->base = base;

  voice->attack  = *data++;
  voice->decay   = *data++;
  voice->sustain = *data++;
  voice->release = *data++;

  return voice;
}