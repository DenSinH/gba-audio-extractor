#include "extractor.h"
#include "song.h"
#include "file.h"
#include "error.h"
#include "gba.h"
#include "bin.h"

#define HEADER_NUM_TRACKS_OFFSET 0
#define HEADER_REVERB_OFFSET 3
#define HEADER_VOICEGROUP_PTR_OFFSET 4
#define HEADER_TRACK_PTR_OFFSET 8


void Extractor::Extract(u32 start, u32 size, u32 header_offset) {
  auto header = util::GetPointer(start + header_offset);

  const u8 num_tracks      = header[HEADER_NUM_TRACKS_OFFSET];
  const u8 reverb          = header[HEADER_REVERB_OFFSET];
  const u32 voicegroup_ptr = util::Read<u32>(&header[HEADER_VOICEGROUP_PTR_OFFSET]);

  Debug("Voicegroups at %08x", voicegroup_ptr);

  Song song = Song(num_tracks, reverb, util::GetPointer(voicegroup_ptr));

  for (int i = 0; i < num_tracks; i++) {
    Debug("Extracting track %d", i);
    auto* track_ptr = util::GetPointer(util::Read<u32>(&header[HEADER_TRACK_PTR_OFFSET + sizeof(u32) * i]));
    song.AddTrack(track_ptr);
  }
}