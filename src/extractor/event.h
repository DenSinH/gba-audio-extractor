#pragma once

#include "types.h"


struct Wait {
  i32 amount;
};

struct Goto {
  i32 index;
};

struct Patt {
  i32 index;
};

struct Pend {

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
};

struct Event {

  u8 type;

  union {
    Wait wait;
    Goto got;
    Patt patt;
    Pend pend;
    Tempo tempo;
    VoiceChange voice_change;
    Controller controller;
    Note note;
  };
};