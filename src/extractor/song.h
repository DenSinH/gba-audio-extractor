#pragma once

#include "track.h"
#include "voice.h"

#include <vector>

struct Song {
  Song(u8 num_tracks, u8 reverb, const u8* voicegroup_ptr) :
      tracks{}, reverb{reverb}, voicegroup{voicegroup_ptr} {
    tracks.reserve(num_tracks);
  }

  void AddTrack(const u8* pointer) {
    tracks.emplace_back(Track(pointer));
  }

  // todo: blocks?
  u8 reverb;
  VoiceGroup voicegroup;
  std::vector<Track> tracks;
};