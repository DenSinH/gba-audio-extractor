#pragma once

#include "song.h"

#include <vector>
#include <list>


struct Sample {
  float left;
  float right;
};

struct PanVolume {
  double left;
  double right;
};

struct Player {
  const Song* song;

  explicit Player(const Song* song) : song{song} {
    InitTracks();
  }

  void TickTime(double dt);
  Sample GetSample() const;
  void SetTrackEnable(i32 track, bool value);
  int GetCurrentTick() const;
  void SkipToTick(i32 tick);

private:
  double time_per_tick = 0;
  double global_time = 0;
  double fine_time = 0;  // time since last beat

  struct PlayingNote {
    double time_started;
    i32 tick_started;
    double time_released;
    const Note* note;
    VoiceState voice;
  };

  struct TrackStatus {
    // external control
    bool enabled = true;

    // control variables
    i32 keyshift   = 0;
    double vol    = 0;
    i32 pan        = 0;
    i32 bend       = 0;
    u32 bend_range = 2;
    u32 lfo_speed  = 22;
    u32 lfo_delay  = 0;
    u32 mod        = 0;
    u32 modt       = 0;
    i32 tune       = 0;

    // internal state
    i32 tick = 0;
    const Event* current_event = nullptr;
    const Voice* voice         = nullptr;
    std::list<PlayingNote> current_notes{};

    double GetPitchAdjust(double dt) const;
    PanVolume GetPannedVolume(double dt) const;
  private:
    double modM(double dt) const;
  };

  std::vector<TrackStatus> statuses{};

  void Reset();
  void InitTracks();
  void TickTracks();
  void TickTrack(const Track& track, TrackStatus& status);
  void TickEvents(const Track& track, TrackStatus& status);
  void TickNotes(const Track& track, TrackStatus& status);
};