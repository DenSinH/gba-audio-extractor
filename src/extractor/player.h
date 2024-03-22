#pragma once

#include "song.h"

#include <vector>
#include <list>
#include <deque>


struct Sample {
  float left;
  float right;
};

struct PanVolume {
  double left;
  double right;
};

struct Player {
  const Song song;

  bool paused = true;

  explicit Player(Song song) : song{std::move(song)} {
    InitTracks();
  }

  Sample GetNextSample();
  void ToggleTrackEnable(i32 track);
  bool GetTrackEnable(i32 track);
  int GetCurrentTick() const;
  void SkipToTick(i32 tick);
  const Note* GetTrackNote(i32 track);

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
    struct ExternalControl {
      bool enabled = true;
    } external;

    // control variables
    i32 keyshift   = 0;
    double vol    = 0;
    i32 pan        = 0;
    i32 bend       = 0;
    i32 bend_range = 2;
    u32 lfo_speed  = 22;
    u32 lfo_delay  = 0;
    u32 mod        = 0;
    u32 modt       = 0;
    i32 tune       = 0;

    // internal state
    i32 tick = 0;
    bool track_ended = false;

    const Event* current_event = nullptr;
    const Voice* voice         = nullptr;
    std::list<PlayingNote> current_notes{};

    double GetPitchAdjust(double dt) const;
    PanVolume GetPannedVolume(double dt) const;
  private:
    double modM(double dt) const;
  };

  std::vector<TrackStatus> statuses{};

  Sample last_sample = {};

  static constexpr double ReverbFrames = 7;
  std::deque<Sample> reverb_buffer = {};

  void Reset();
  void InitTracks();
  void TickTime();
  void TickTracks();
  void TickTrack(const Track& track, TrackStatus& status);
  void TickEvents(const Track& track, TrackStatus& status);
  void TickNotes(const Track& track, TrackStatus& status);

  void AddReverbSample(Sample& sample);
  double GetReverb() const;
};