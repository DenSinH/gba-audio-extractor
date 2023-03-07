#include "track.h"
#include "error.h"
#include "mplaydef.h"
#include "gba.h"
#include "bin.h"


Track::Track(const u8* data) {
  Parse(data);
}

void Track::Parse(const u8* data) {
  const u8* const track_start = data;
  std::vector<i32> branch_targets{};

  i32 keyshift    = 0;
  i32 last_cmd    = -1;
  i32 last_note   = -1;
  i32 last_vel    = -1;

  const auto read_byte = [&](bool branchable = false) {
    if (branchable)
      branch_targets.emplace_back(events.size());
    else
      branch_targets.emplace_back(-1);
    return *data++;
  };

  const auto read_word = [&]() {
    // never branchable
    branch_targets.emplace_back(-1);
    branch_targets.emplace_back(-1);
    branch_targets.emplace_back(-1);
    branch_targets.emplace_back(-1);
    u32 result = util::Read<u32>(data);
    data += sizeof(u32);
    return result;
  };

  while (true) {
    u8 cmd = read_byte(true);
    if (cmd < 0x80) {
      if (last_cmd > 0) {
        cmd = last_cmd;
      }
      else {
        Error("Expected command as first track byte, got %02x", cmd);
      }
    }
    else if (cmd > GbaCmd::PEND && cmd != GbaCmd::TEMPO) {
      // repeatable commands
      last_cmd = cmd;
    }

    Debug("Parsing %02x command", cmd);
    events.push_back(Event{cmd});

    switch (cmd) {
      case GbaCmd::W00 ... GbaCmd::W96: {
        // wait event
        events.back().wait = Wait{LengthTable[cmd - W00]};
        break;
      }
      case GbaCmd::FINE: {
        return;
      }
      case GbaCmd::GOTO: {
        const auto dest = util::GetPointer(read_word());
        const auto diff = dest - track_start;
        if (diff < 0 || diff >= branch_targets.size()) {
          Error("Invalid branch target for goto: offset %d", diff);
        }

        events.back().got = Goto{branch_targets[diff]};
        break;
      }
      case GbaCmd::PATT: {
        const auto dest = util::GetPointer(read_word());
        const auto diff = dest - track_start;
        if (diff < 0 || diff >= branch_targets.size()) {
          Error("Invalid branch target for goto: offset %d", diff);
        }

        events.back().patt = Patt{branch_targets[diff]};  // NOLINT
        break;
      }
      case GbaCmd::PEND: {
        events.back().pend = Pend{};
        break;
      }
      case GbaCmd::TEMPO: {
        events.back().tempo = Tempo{read_byte()};
        break;
      }
      case GbaCmd::KEYSH: {
        keyshift = read_byte();
        break;
      }
      case GbaCmd::VOICE: {
        events.back().voice_change = VoiceChange{read_byte()};
        break;
      }
      case GbaCmd::VOL:
      case GbaCmd::PAN:
      case GbaCmd::BEND:
      case GbaCmd::BENDR:
      case GbaCmd::LFOS:
      case GbaCmd::LFODL:
      case GbaCmd::MOD:
      case GbaCmd::MODT:
      case GbaCmd::TUNE: {
        // * 127 / 128 in agb.cpp
        events.back().controller = Controller{cmd, read_byte()};
        break;
      }
      case GbaCmd::N01 ... GbaCmd::N96: {
        i32 length = LengthTable[cmd - GbaCmd::TIE];
        i32 note, vel;
        if (*data < 0x80) {
          last_note = note = read_byte();
          if (*data < 0x80) {
            last_vel = vel = read_byte();
            if (*data < 0x80) {
              // todo: is this used?
              length += read_byte();
            }
          }
          else {
            vel = last_vel;
          }
        }
        else {
          note = last_note;
          vel  = last_vel;
        }

        events.back().note = {
            length,
            note + keyshift,
            vel,
        };
        break;
      }
      default: {
        Error("Invalid or unimplemented command: %02x", cmd);
      }
    }
  }
}