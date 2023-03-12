#pragma once

#include "track.h"
#include "voice.h"

#include <vector>

struct Song {
  Song(u8 num_tracks, u8 _reverb, const u8* voicegroup_ptr) :
      tracks{}, voicegroup{voicegroup_ptr} {
    reverb    = _reverb & 0x7f;
    do_reverb = (_reverb & 0x80) != 0;
    tracks.reserve(num_tracks);
  }

  void AddTrack(const u8* pointer) {
    tracks.emplace_back(Track(pointer));
  }

  // todo: blocks?
  u8 reverb;
  bool do_reverb;
  VoiceGroup voicegroup;
  std::vector<Track> tracks;

  static Song Extract(const u8* header);
};