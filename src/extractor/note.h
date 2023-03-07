#pragma once

#include "voice.h"
#include "types.h"


struct Note {
  u8 duration;
  u8 key;
  u8 velocity;
};