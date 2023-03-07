#include "voice.h"
#include "error.h"
#include "bin.h"
#include "gba.h"


const Voice& VoiceGroup::operator[](u32 index) {
  if (index >= voices.size()) {
    auto old_size = voices.size();
    voices.resize(index + 1);
    for (auto i = old_size; i < index + 1; i++) {
      voices[i] = ParseNext();
    }
  }
  return *voices[index];
}

std::unique_ptr<Voice> VoiceGroup::ParseNext() {
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

      u32 loop_start = util::Read<u32>(&wave_data[8]);
      u32 size       = util::Read<u32>(&wave_data[12]);

      _voice->samples = {&wave_data[16], &wave_data[16 + size]};
      _voice->loop_start = loop_start;
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
    case Voice::Type::KeysplitAlt: {
      auto _voice = std::make_unique<Keysplit>();
      // already skipped one byte for "base" data
      data++;  // sbz
      data++;  // sbz

      auto* voice_group = util::GetPointer(util::Read<u32>(data));
      data += sizeof(u32);

      _voice->split = (voice_group == original_data) ? nullptr : std::make_unique<VoiceGroup>(voice_group);
      _voice->table = (void*)util::GetPointer(util::Read<u32>(data));
      data += sizeof(u32);

      voice = std::move(_voice);
      break;
    }
    default: {
      Error("Unknown type for voice: %x (%d)", type, type);
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