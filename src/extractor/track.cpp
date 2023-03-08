#include "track.h"
#include "error.h"
#include "mplaydef.h"
#include "gba.h"
#include "bin.h"
#include "algorithm.h"


Track::Track(const u8* data) {
  Parse(data);
}

void Track::Parse(const u8* data) {
  const u8* const track_start = data;

  // keep track of branch targets
  // branch_target[offset from track_start] = index into events
  std::vector<i32> branch_targets{};

  i32 keyshift     = 0;
  i32 last_cmd     = -1;
  i32 last_note    = -1;
  i32 last_vel     = -1;
  i32 current_time = 0;

  const auto read_byte = [&](bool branchable = false) {
    branch_targets.emplace_back(branchable ? events.size() : -1);
    return *data++;
  };

  const auto read_word = [&]() {
    // never branchable
    branch_targets.insert(branch_targets.end(), {-1, -1, -1, -1});
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
    events.push_back(Event{
      cmd,
      current_time
    });

    switch (cmd) {
      case GbaCmd::W00 ... GbaCmd::W96: {
        current_time += LengthTable[cmd - W00];
        // replace with null event to make sure patterns end at the right time
        events.back() = Event{0, current_time};
        break;
      }
      case GbaCmd::FINE: {
        events.pop_back();  // no event, end track

        // filter out null events and PEND events
        events = util::filter(events, [](const auto& e){ return e.type && e.type != GbaCmd::PEND; });
        Debug("Track end, found %d commands", events.size());
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
          Error("Invalid branch target for patt: offset %d", diff);
        }

        const i32 patt_start_time = events[branch_targets[diff]].time;
        events.pop_back();  // no PATT event, unpack pattern right away

        for (int i = branch_targets[diff]; events[i].type != GbaCmd::PEND; i++) {
          if (i >= events.size()) {
            Error("Expected end of pattern while scanning pattern");
          }
          auto event = events[i];

          // displace time from pattern_start_time to current_time
          event.time = event.time - patt_start_time + current_time;

          // use current keyshift
          if (event.type >= GbaCmd::TIE && event.type <= GbaCmd::N96) {
            event.note.keyshift = keyshift;
          }
          events.push_back(event);
        }

        // correct current time to last events time
        // we insert null events on WAITs, so the last event will always hold
        // the correct "current" time
        current_time = events.back().time;
        break;
      }
      case GbaCmd::PEND: {
        // leave event uninitialized, we will remove these events later anyway
        break;
      }
      case GbaCmd::TEMPO: {
        events.back().tempo = Tempo{read_byte()};
        break;
      }
      case GbaCmd::KEYSH: {
        keyshift = read_byte();
        // no event, keyshift is incorporated in note event
        // todo: KEYSHIFT in PATT / PEND
        events.pop_back();
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
        // VOL * 127 / 128 in agb.cpp
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
            note,
            vel,
            keyshift,
        };
        break;
      }
      case GbaCmd::MEMACC:
      case GbaCmd::XCMD:
      case GbaCmd::EOT:
      case GbaCmd::TIE:  // todo: how is tie different than just note?
      default: {
        Error("Invalid or unimplemented command: %02x", cmd);
      }
    }
  }
}