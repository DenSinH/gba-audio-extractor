#pragma once

#include "song.h"
#include "sample.h"

#include <vector>
#include <list>


struct Player {
  const Song* song;

  explicit Player(const Song* song) : song{song} {
    InitTracks();
  }

  void TickTime(double dt);
  Sample GetSample() const;

private:
  double time_per_tick = 0;
  double global_time = 0;
  double fine_time = 0;  // time since last beat

  struct PlayingNote {
    double time_started;
    i32 tick_started;
    const Note* note;
  };

  struct TrackStatus {
    i32 tick = 0;
    double volume = 0;
    const Event* current_event;
    const Voice* voice = nullptr;
    std::list<PlayingNote> current_notes{};
  };

  std::vector<TrackStatus> statuses{};

  void InitTracks();
  void TickTrack(const Track& track, TrackStatus& status);
  void HandleEvents(const Track& track, TrackStatus& status);
  void HandleNotes(const Track& track, TrackStatus& status);
};