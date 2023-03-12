#include "player.h"
#include "error.h"
#include "constants.h"

#include <cmath>
#include <algorithm>


static inline double TimePerTick(u32 bpm) {
  // 96 ticks per 4 beats
  return (60.0 / bpm) / 24.0;
}

void Player::ToggleTrackEnable(i32 track) {
  if (track >= statuses.size()) {
    Error("Track out of range for song: %d", track);
  }

  statuses[track].enabled ^= true;
}

bool Player::GetTrackEnable(i32 track) {
  if (track >= statuses.size()) {
    Error("Track out of range for song: %d", track);
  }

  return statuses[track].enabled;
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
    TickEvents(track, statuses.back());
  }

  if (time_per_tick == 0) {
    Error("No tempo found at start of song");
  }
}

void Player::TickTime(double dt) {
  if (paused) {
    return;
  }

  global_time += dt;
  fine_time   += dt;

  for (auto& status : statuses) {
    for (auto& note : status.current_notes) {
      const double pitch_adjust = status.GetPitchAdjust(global_time - note.time_started);
      const i32 keyshift        = std::floor(pitch_adjust);
      const double fine_adjust  = pitch_adjust - keyshift;
      const i32 key             = note.note->key + keyshift;

      note.voice.Tick(key, fine_adjust, dt);
    }
  }
  
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

void Player::TickEvents(const Track& track, TrackStatus& status) {
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
            status.vol = event.controller.val / 127.0;
            break;
          }
          case Controller::Type::KEYSH: {
            status.keyshift = static_cast<i8>(event.controller.val);
            break;
          }
          case Controller::Type::PAN: {
            status.pan = static_cast<i8>(event.controller.val) - 0x40;
            break;
          }
          case Controller::Type::BEND: {
            status.bend = static_cast<i8>(event.controller.val) - 0x40;
            break;
          }
          case Controller::Type::BENDR:{
            status.bend_range = event.controller.val;
            break;
          }
          case Controller::Type::LFOS: {
            status.lfo_speed = event.controller.val;
            break;
          }
          case Controller::Type::LFODL: {
            status.lfo_delay = event.controller.val;
            break;
          }
          case Controller::Type::MOD: {
            status.mod = event.controller.val;
            break;
          }
          case Controller::Type::MODT: {
            status.modt = event.controller.val;
            break;
          }
          case Controller::Type::TUNE:  {
            status.tune = static_cast<i8>(event.controller.val) - 0x40;
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
            &event.note,
            status.voice->GetState(event.note.key)
        });
        break;
      }
      case Event::Type::Fine:
        return;
    }

    status.current_event++;
  }
}

void Player::TickNotes(const Track& track, TrackStatus& status) {
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
      const double echoVol = note->voice.GetEnvelopeVolume(
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

  TickEvents(track, status);
  TickNotes(track, status);
}

double Player::TrackStatus::modM(double dt) const {
  if (!mod || !lfo_speed) {
    return 0;
  }
  if (lfo_delay && dt < lfo_delay * FrameTime) {
    return 0;
  }
  dt -= lfo_delay * FrameTime;
  return mod * std::sin(2 * 3.141592 * (lfo_speed / 256.0) * dt * FrameFreq);
}

double Player::TrackStatus::GetPitchAdjust(double dt) const {
  double adjust = 4 * (tune + bend * bend_range);
  if (modt == 0) {
    adjust += 16 * modM(dt);
  }
  return keyshift + (adjust / 256.0);
}

PanVolume Player::TrackStatus::GetPannedVolume(double dt) const {
  double main_vol = vol;
  if (modt == 1) {
    main_vol *= (modM(dt) + 128.0) / 128.0;
  }

  double pan_vol = 2 * pan;
  if (modt == 2) {
    pan_vol += modM(dt);
  }
  pan_vol = std::clamp(pan_vol, -128.0, 127.0);
  
  return {
      .left  = (127.0 - pan_vol) * main_vol / 256.0,
      .right = (pan_vol + 128.0) * main_vol / 256.0,
  };
}

Sample Player::GetSample() const {
  if (paused) {
    return last_sample;
  }

  Sample total = {};
  for (const auto& status : statuses) {
    if (!status.enabled) continue;

    Sample superposition = {};
    for (const auto& note : status.current_notes) {
      const double time_since_started = global_time - note.time_started;
      const double time_since_release = global_time - note.time_released;

      const double amp     = note.voice.GetSample(time_since_release);
      const double vel_amp = amp * note.note->velocity / 128.0;
      const auto pan_vol   = status.GetPannedVolume(time_since_started);

      superposition.left  += vel_amp * pan_vol.left;
      superposition.right += vel_amp * pan_vol.right;
    }
    total.left  += superposition.left;
    total.right += superposition.right;
  }

  last_sample = total;
  return total;
}
