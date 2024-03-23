#include "track.h"
#include "error.h"
#include "mplaydef.h"
#include "util/gba.h"
#include "util/bin.h"
#include "util/algorithm.h"

#include <array>


Track::Track(const u8* data) {
  Parse(data);
}

void Track::Parse(const u8* data) {
  const u8* const track_start = data;

  // keep track of branch targets
  // branch_target[offset from track_start] = index into events
  std::vector<i32> branch_targets{};

  i32 last_cmd      = -1;
  i32 last_note     = -1;
  i32 last_vel      = -1;
  i32 current_tick  = 0;

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
    u8 cmd;
    if (*data < 0x80) {
      if (last_cmd > 0) {
        cmd = last_cmd;
      }
      else {
        Error("Expected command as first track byte, got %02x", *data);
      }
    }
    else {
      cmd = read_byte(true);
      if (cmd > GbaCmd::PEND && cmd != GbaCmd::TEMPO) {
        // repeatable commands
        last_cmd = cmd;
      }
    }

    // Debug("Parsing %02x command", cmd);
    events.push_back(Event{
      // initialize as meta event (ignored)
      Event::Type::Meta,
      current_tick
    });

    switch (cmd) {
      case GbaCmd::W00: {
        // basically a NOP
        events.pop_back();
        break;
      }
      case GbaCmd::W01 ... GbaCmd::W96: {
        current_tick += LengthTable[cmd - W00];
        // Debug("Wait %d until %d", LengthTable[cmd - W00], current_tick);
        // replace with null events to make sure patterns end at the right tick
        events.back().meta.type = Meta::Type::Wait;
        events.push_back(Event{Event::Type::Meta, current_tick});
        events.back().meta.type = Meta::Type::Wait;
        break;
      }
      case GbaCmd::FINE: {
        events.back().type = Event::Type::Fine;

        length = current_tick;
        PostProcess();
        Debug("Track end, found %d commands", events.size());
        return;
      }
      case GbaCmd::GOTO: {
        events.back().type = Event::Type::Goto;
        const auto dest = util::GetPointer(read_word());
        const auto diff = dest - track_start;
        if (diff < 0 || diff >= branch_targets.size()) {
          Error("Invalid branch target for goto: offset %d", diff);
        }

        // tick of event at branch target is always the tick to jump to,
        // since we insert the null events on WAITs
        events.back().got = Goto{events[branch_targets[diff]].tick};
        break;
      }
      case GbaCmd::PATT: {
        const auto dest = util::GetPointer(read_word());
        const auto diff = dest - track_start;
        if (diff < 0 || diff >= branch_targets.size()) {
          Error("Invalid branch target for patt: offset %d", diff);
        }

        const i32 patt_start_tick  = events[branch_targets[diff]].tick;
        // Debug("Pattern start tick %d", patt_start_tick);
        events.pop_back();  // no PATT event, unpack pattern right away

        for (int i = branch_targets[diff];; i++) {
          if (events[i].type == Event::Type::Meta && events[i].meta.type == Meta::Type::Pend) {
            break;
          }
          if (i >= events.size()) {
            Error("Expected end of pattern while scanning pattern");
          }
          auto event = events[i];

          // displace tick from pattern_start_tick to current_tick
          event.tick = event.tick - patt_start_tick + current_tick;

          events.push_back(event);
        }

        // correct current tick to last events tick
        // we insert null events on WAITs, so the last event will always hold
        // the correct "current" tick
        // Debug("Pattern took %d, tick from %d to %d", events.back().tick - current_tick, current_tick, events.back().tick);
        current_tick = events.back().tick;
        break;
      }
      case GbaCmd::PEND: {
        events.back().meta.type = Meta::Type::Pend;
        // leave event uninitialized, we will remove these events later anyway
        break;
      }
      case GbaCmd::TEMPO: {
        events.back().type = Event::Type::Tempo;
        events.back().tempo = Tempo{2 * read_byte()};
        break;
      }
      case GbaCmd::VOICE: {
        events.back().type = Event::Type::VoiceChange;
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
      case GbaCmd::KEYSH:
      case GbaCmd::TUNE: {
        events.back().type = Event::Type::Controller;
        // VOL * 127 / 128 in agb.cpp
        events.back().controller = Controller{static_cast<Controller::Type>(cmd), read_byte()};
        break;
      }
      case GbaCmd::N01 ... GbaCmd::N96: {
        events.back().type = Event::Type::Note;
        i32 length = LengthTable[cmd - GbaCmd::N01 + 1];
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
        };
        break;
      }
      // TIE: start note at TIE command, end at EOT command
      case GbaCmd::TIE: {
        events.back().meta.type = Meta::Type::Tie;

        if (*data < 0x80) {
          events.back().meta.tie.note.key = last_note = read_byte();
          if (*data < 0x80) {
            events.back().meta.tie.note.velocity = last_vel = read_byte();
          }
          else {
            events.back().meta.tie.note.velocity = last_vel;
          }
        }
        else {
          events.back().meta.tie.note.key      = last_note;
          events.back().meta.tie.note.velocity = last_vel;
        }
        // set note length to current tick
        // to be set to end - start in EOT event
        events.back().meta.tie.note.length = current_tick;
        break;
      }
      case GbaCmd::EOT: {
        events.back().meta.type = Meta::Type::Eot;

        i32 key = -1;
        if (*data < 0x80) {
          key = read_byte();
        }
        else {
          key = last_note;
        }
        events.back().meta.eot.key = key;
        break;
      }
      case GbaCmd::PRIO: {
        u8 prio = read_byte();
        break;
      }
      case GbaCmd::MEMACC:
      case GbaCmd::XCMD:
      default: {
        Error("Invalid or unimplemented command: %02x", cmd);
      }
    }
  }
}

void Track::PostProcess() {
  // filter out meta events and fix TIEs
  std::vector<Event> processed{};
  processed.reserve(events.size());
  std::array<i32, 0x80> current_ties{};
  std::fill(current_ties.begin(), current_ties.end(), -1);

  for (auto& event : events) {
    if (event.type == Event::Type::Meta) {
      // meta events may need to be handled differently
      switch (event.meta.type) {
        case Meta::Type::Tie: {
          if (current_ties[event.meta.tie.note.key] != -1) {
            Error("Double note tie in track");
          }

          // save the tie, convert to note at EOT
          current_ties[event.meta.tie.note.key] = processed.size();
          processed.push_back(event);
          break;
        }
        case Meta::Type::Eot: {
          if (current_ties[event.meta.eot.key] == -1) {
            Error("End of tie without active tie on note");
          }
          auto& tie = processed[current_ties[event.meta.eot.key]];
          if (event.meta.eot.key > 0 && (tie.meta.tie.note.key != event.meta.eot.key)) {
            Error(
                "End tie of different note, got %d, expected %d",
                tie.meta.tie.note.key, event.meta.eot.key
            );
          }

          auto note = tie.meta.tie.note;
          // note length is difference between EOT and TIE
          note.length = event.tick - tie.tick;

          // change TIE event to note
          tie.type = Event::Type::Note;
          tie.note = note;
          current_ties[event.meta.eot.key] = -1;
          break;
        }
        default: {
          break;
        }
      }
    }
    else {
      // just copy over normal events
      if (event.type == Event::Type::Goto) {
        const bool active_tie = std::any_of(current_ties.cbegin(), current_ties.cend(), [](auto element) {
            return element != -1;
        });
        if (active_tie) {
          Error("Active tie during GOTO command");
        }
      }
      processed.push_back(event);
    }
  }

  const bool active_tie = std::any_of(current_ties.cbegin(), current_ties.cend(), [](auto element) {
    return element != -1;
  });
  if (active_tie) {
    Error("Active tie at FINE");
  }
  events = std::move(processed);
}