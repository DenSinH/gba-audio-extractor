#include "voice.h"
#include "error.h"
#include "util/bin.h"
#include "util/gba.h"

#include "midi.h"
#include "noise.h"
#include "constants.h"

#include <cmath>


void VoiceState::Tick(i32 midi_key, double pitch_adjust, double dt) {
  if (base_key >= 0) {
    // keep fine adjust
    midi_key = base_key;
  }

  const double freq = voice->GetFrequency(midi_key + pitch_adjust);
  integrated_time += dt * freq;
  actual_time     += dt;
}

double VoiceState::GetSample(double time_since_release) const {
  const double wave_sample     = voice->WaveForm(integrated_time);
  const double envelope_volume = voice->GetEnvelopeVolume(actual_time, time_since_release);
  return wave_sample * envelope_volume;
}

double VoiceState::GetEnvelopeVolume(double time_since_start, double time_since_release) {
  return voice->GetEnvelopeVolume(time_since_start, time_since_release);
}

// in general, we don't need the base_key for the voice state
// so we set it to -1
VoiceState Voice::GetState([[maybe_unused]] i32 base_key) const {
  return VoiceState(this, -1);
}

WaveFormData Voice::GetEnvelopeWaveFormData(i32 duration) const {
  WaveFormData data{};
  const float dt = duration / 1000;
  for (int i = 0; i < 1000; i++) {
    const float time = i * dt;
    data.AddPoint(time, GetEnvelopeVolume(time, 0));
  }
  for (int i = 0; i < 200; i++) {
    const float time = i * dt;
    const float env = GetEnvelopeVolume(time, 0);
    data.AddPoint(duration + time, env);
    if (env < 1 / 256.0) {
      break;
    }
  }

  return data;
}

void DirectSound::CalculateEnvelope() {
  // from https://github.com/Kurausukun/pokeemerald/blob/54e55cf040e8ef4b11632a0af14b9f512827d92a/src/sound_mixer.c#L122:
  // MP2K envelope shape
  //                                                                 |
  // (linear)^                                                       |
  // Attack / \Decay (exponential)                                   |
  //       /   \_                                                    |
  //      /      '.,        Sustain                                  |
  //     /          '.______________                                 |
  //    /                           '-.       Echo (linear)          |
  //   /                 Release (exp) ''--..|\                      |
  //  /                                        \                     |
  if (!attack) {
    Error("Zero attack on direct sound envelope");
  }
  // subtract one since envelope ticks before sound is generated,
  // and so attack is incremented right away
  // this makes it so a 255 value of attack gives a zero time until attack finish
  attack_time = FrameTime * (255.0 / attack - 1);

  // exponential decay with a factor of (decay / 255) per frame
  if (!decay) {
    // a decay of zero is instant
    decay_time = 0;
  }
  else if (sustain == 255) {
    // a maximum sustain volume also gives instant decay
    decay_time = 0;
  }
  else {
    // we have that volume = 255 * (decay / 255) ^ (time / FrameTime) = sustain
    const double frames_until_sustain = std::log(sustain / 255.0) / std::log(decay / 255.0);
    decay_time = frames_until_sustain * FrameTime;
  }
}

double DirectSound::GetEnvelopeVolume(double time_since_start, double time_since_release) const {
  if (time_since_release > 0) {
    return (sustain / 255.0) * std::pow(release / 256.0, time_since_release / FrameTime);
  }
  else {
    if (time_since_start < attack_time) {
      return (time_since_start * FrameFreq) * attack / 255.0;
    }
    else {
      time_since_start -= attack_time;
      if (time_since_start < decay_time) {
        // 255 * this exponential, but we normalize to [0, 1]
        return std::pow(decay / 256.0, time_since_start * FrameFreq);
      }
      else {
        return sustain / 255.0;
      }
    }
  }
}

double DirectSound::GetFrequency(double midi_key) const {
  return MidiKeyToFreq(freq, midi_key);
}

double DirectSound::WaveForm(double integrated_time) const {
  // so if time is dt, we are at sample
  u32 sample_index_course = u32(integrated_time);
  const double sample_index_fine = integrated_time - sample_index_course;

  if (sample_index_course >= samples.size()) {
    if (!do_loop) {
      // return last sample
      return double(i8(samples.back())) / 256.0;
    }
    sample_index_course = loop_start + (sample_index_course % (samples.size() - loop_start));
  }
  const double current_sample = double(i8(samples[sample_index_course])) / 256.0;

  double next_sample;
  if (sample_index_course + 1 >= samples.size()) {
    if (!do_loop) {
      // no interpolation
      return current_sample;
    }
    next_sample = double(i8(samples[loop_start])) / 256.0;
  }
  else {
    next_sample = double(i8(samples[sample_index_course + 1])) / 256.0;
  }

  // linear interpolation
  return (1 - sample_index_fine) * current_sample + sample_index_fine * next_sample;
}

void CgbVoice::CalculateEnvelope() {
  // todo: check this
  attack_time = attack / 64.0;
  decay_time  = decay  / 64.0;
}

double CgbVoice::GetEnvelopeVolume(double time_since_start, double time_since_release) const {
  if (time_since_release > 0) {
    const double release_time = release / 64.0;
    return (sustain / 15.0) * std::max((release_time - time_since_release) / release_time, 0.0);
  }
  else {
    if (time_since_start < attack_time) {
      return time_since_start / attack_time;
    }
    else {
      time_since_start -= attack_time;
      if (time_since_start < decay_time) {
        const double fraction = time_since_start / decay_time;
        return 1.0 * (1 - fraction) + (sustain / 15.0) * fraction;
      }
      else {
        return sustain / 15.0;
      }
    }
  }
}

double CgbVoice::GetFrequency(double midi_key) const {
  return CgbMidiKeyToFreq(midi_key);
}

double ProgrammableWave::WaveForm(double integrated_time) const {
  // sample rate is 2097152 / 131072 = 16 times the frequency of the noise channel,
  integrated_time *= 16;
  int sample_index = (int)(integrated_time) % 32;
  double sample_index_fine = integrated_time - std::floor(integrated_time);

  u8 sample = samples[sample_index >> 1];
  if (!(sample_index & 1)) sample = sample >> 4;
  sample &= 0xf;

  u8 next_sample;
  // if we are in the first sample, interpolate with 0 instead
  if (integrated_time < 0.5) {
    next_sample = 0;
    sample_index_fine = 1 - sample_index_fine;
  }
  else {
    next_sample = samples[((sample_index + 1) % 32) >> 1];
    if (!(sample_index & 1)) next_sample = next_sample >> 4;
    next_sample &= 0xf;
  }

  // linear interpolation
  // - 0.5 to prevent popping on note starts
  return (1 - sample_index_fine) * (sample / 15.0) + sample_index_fine * (next_sample / 15.0) - 0.5;
}

double Square::WaveForm(double integrated_time) const {
  double in_period = integrated_time - std::floor(integrated_time);

  return (DutyCycle[duty_cycle] & (0x80 >> u32(in_period * 8))) ? -0.5 : 0.5;
}

double Noise::GetFrequency(double midi_key) const {
  return NoiseFreqTable[(i32)midi_key];
}

double Noise::WaveForm(double integrated_time) const {
  if (period) {
    // 7 bits
    const u32 index = (u32)integrated_time % (8 * NoiseSequence7.size());
    const u8  sample_set = NoiseSequence7[index / 8];
    const u32 sample_index = index % 8;
    return (sample_set & (0x80 >> sample_index)) ? -0.5 : 0.5;
  }
  else {
    // 15 bits
    const u32 index = (u32)integrated_time % (8 * NoiseSequence15.size());
    const u8  sample_set = NoiseSequence15[index / 8];
    const u32 sample_index = index % 8;
    return (sample_set & (0x80 >> sample_index)) ? -0.5 : 0.5;
  }
}

const Voice& Keysplit::GetVoice(int midi_key) const {
  if (table) {
    return (*split)[table[midi_key]];
  }
  else {
    return (*split)[midi_key];
  }
}

double Keysplit::GetEnvelopeVolume(double time_since_start, double time_since_release) const {
  Error("Attempted to get envelope volume of keysplit voice");
}

double Keysplit::GetFrequency(double midi_key) const {
  Error("Attempted to get frequency of keysplit voice");
}

double Keysplit::WaveForm(double integrated_time) const {
  // todo: forced pan in Keysplit voice
  Error("Attempted to get waveform of keysplit voice");
}

VoiceState Keysplit::GetState(i32 midi_key) const {
  const auto& voice = GetVoice(midi_key);

  i32 key = midi_key;
  if (type == Voice::Type::KeysplitAltRhythm) {
    // todo: pan
    key = voice.base;
  }
  return voice.GetState(key);
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
      u16 flags          = util::Read<u16>(&wave_data[2]);
      _voice->do_loop    = (flags & 0xc000) != 0;

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

      // no attack / decay / sustain / release in keysplit voices
      voice->type = type;
      voice->base = base;
      return voice;
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

  // compute envelope only once
  voice->CalculateEnvelope();

  return voice;
}
