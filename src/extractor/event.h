#pragma once

#include "types.h"
#include "mplaydef.h"


struct Goto {
  i32 tick;
};

struct Tempo {
  i32 bpm;
};

struct VoiceChange {
  i32 voice;
};

struct Controller {
  enum class Type {
    KEYSH = GbaCmd::KEYSH,
    VOL   = GbaCmd::VOL,
    PAN   = GbaCmd::PAN,
    BEND  = GbaCmd::BEND,
    BENDR = GbaCmd::BENDR,
    LFOS  = GbaCmd::LFOS,
    LFODL = GbaCmd::LFODL,
    MOD   = GbaCmd::MOD,
    MODT  = GbaCmd::MODT,
    TUNE  = GbaCmd::TUNE,
  };

  Type type;
  i32 val;
};

struct Note {
  i32 length;
  i32 key;
  i32 velocity;
};

struct Fine {

};

struct Meta {
  enum class Type {
    Wait,
    Tie,
    Eot,
    Pend,
  };

  Type type;
  union {
    struct { Note note; } tie;
    struct { i32 key; } eot;
  };
};

struct Event {
  enum class Type {
    Meta = 0,
    Goto,
    Tempo,
    VoiceChange,
    Controller,
    Note,
    Fine,
  };

  Type type;
  i32 tick;

  union {
    Goto got;
    Tempo tempo;
    VoiceChange voice_change;
    Controller controller;
    Note note;
    Fine fine;
    Meta meta;
  };
};