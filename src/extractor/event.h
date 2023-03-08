#pragma once

#include "types.h"

struct Goto {
  i32 index;
};

struct Tempo {
  i32 bpm;
};

struct VoiceChange {
  i32 voice;
};

struct Controller {
  u8 type;
  i32 vol;
};

struct Note {
  i32 length;
  i32 note;
  i32 velocity;
  i32 keyshift;
};

struct Event {
  u8 type;
  i32 time;

  union {
    Goto got;
    Tempo tempo;
    VoiceChange voice_change;
    Controller controller;
    Note note;
  };
};