#include "player.h"
#include "error.h"


static inline double TimePerTick(u32 bpm) {
  // 96 ticks per 4 beats
  return (60.0 / bpm) / 24.0;
}

void Player::SetTrackEnable(i32 track, bool value) {
  if (track >= statuses.size()) {
    Error("Track out of range for song: %d", track);
  }

  statuses[track].enabled = value;
}

int Player::GetCurrentTick() const {
  return statuses[0].tick;
}

void Player::Reset() {
  global_time   = 0;
  time_per_tick = 0;
  fine_time     = 0;
  statuses = {};
  InitTracks();
}

void Player::SkipToTick(i32 tick) {
  // reset and tick tracks until given tick
  Reset();
  for (i32 i = 0; i < tick; i++) {
    global_time += time_per_tick;
    TickTracks();
  }
}

void Player::InitTracks() {
  statuses = {};
  for (const auto& track : song->tracks) {
    statuses.emplace_back();
    statuses.back().current_event = track.events.data();
    HandleEvents(track, statuses.back());
  }

  if (time_per_tick == 0) {
    Error("No tempo found at start of song");
  }
}

void Player::TickTime(double dt) {
  global_time += dt;
  fine_time   += dt;

  // this while is excessive, but may theoretically be
  // needed
  while (fine_time > time_per_tick) {
    fine_time -= time_per_tick;
    TickTracks();
  }
}

void Player::TickTracks() {
  for (int i = 0; i < statuses.size(); i++) {
    statuses[i].tick++;
    TickTrack(song->tracks[i], statuses[i]);
  }
}

void Player::HandleEvents(const Track& track, TrackStatus& status) {
  while (status.current_event->tick <= status.tick) {
    const auto& event = *status.current_event;

    switch (event.type) {
      case Event::Type::Meta: [[unlikely]]
        Error("Stray meta event");
      case Event::Type::Goto: {
        const auto diff = status.tick - event.got.tick;
        // correct currently playing notes for tick so that
        // the release plays off properly, and the note is removed
        // at the right time
        for (auto& note : status.current_notes) {
          note.tick_started -= diff;
        }

        status.tick = event.got.tick;
        status.current_event = track.events.data();
        while (status.current_event->tick < event.got.tick) {
          status.current_event++;
        }
        // we don't want to increment status.current_event
        // there may be more events that have to happen though,
        // for example, a note may start playing right away after
        // the goto
        continue;
      }
      case Event::Type::Tempo: {
        time_per_tick = TimePerTick(event.tempo.bpm);
        break;
      }
      case Event::Type::VoiceChange: {
        status.voice = &song->voicegroup[event.voice_change.voice];
        break;
      }
      case Event::Type::Controller: {
        switch (event.controller.type) {
          case Controller::Type::VOL: {
            status.volume = event.controller.vol / 127.0;
            break;
          }
          case Controller::Type::PAN:
          case Controller::Type::BEND:
          case Controller::Type::BENDR:
          case Controller::Type::LFOS:
          case Controller::Type::LFODL:
          case Controller::Type::MOD:
          case Controller::Type::MODT:
          case Controller::Type::TUNE: {
//            Error("Unimplemented controller event: %02x", static_cast<u8>(event.controller.type));
            break;
          }
        }
        break;
      }
      case Event::Type::Note: {
        status.current_notes.push_back(PlayingNote{
            global_time - fine_time,
            event.tick,
            std::numeric_limits<double>::infinity(),
            &event.note
        });
        break;
      }
      case Event::Type::Fine:
        return;
    }

    status.current_event++;
  }
}

void Player::HandleNotes(const Track& track, TrackStatus& status) {
  if (status.current_notes.empty()) {
    return;
  }

  if (!status.voice) {
    Error("No voice selected while notes are requested");
  }

  auto note = status.current_notes.begin();
  while (note != status.current_notes.end()) {
    const auto note_end = note->tick_started + note->note->length;
    if (note_end == status.tick) {
      // note ended, may be in release state
      if (!status.voice->release) {
        status.current_notes.erase(note++);
        continue;
      }
      else {
        note->time_released = global_time - fine_time;
      }
    }
    else if (note_end < status.tick) {
      // released note
      const double echoVol = status.voice->GetEnvelopeVolume(
          0,
          global_time - note->time_started,
          global_time - note->time_released
      );

      // todo: check echo volume from track
      if (echoVol < (1 / 255.0)) {
        // echo volume cutoff
        status.current_notes.erase(note++);
        continue;
      }
    }
    note++;
  }
}

void Player::TickTrack(const Track& track, TrackStatus& status) {
  if (status.current_event->type == Event::Type::Fine) {
    // track finished
    status.current_notes = {};
    return;
  }

  HandleEvents(track, status);
  HandleNotes(track, status);
}

Sample Player::GetSample() const {
  double total = 0;
  for (const auto& status : statuses) {
    if (!status.enabled) continue;

    double superposition = 0;
    for (const auto& note : status.current_notes) {
      const auto key                  = note.note->key;
      const double dt                 = global_time - note.time_started;
      const double time_since_release = global_time - note.time_released;

      const double amp = status.voice->GetSample(key, dt, time_since_release);
      superposition += amp * note.note->velocity / 128.0;
    }
    total += superposition * status.volume;
  }

  return Sample{
      (float)total,
      (float)total
  };
}
