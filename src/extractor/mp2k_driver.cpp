#include "mp2k_driver.h"

#include <array>
#include <vector>
#include <string>
#include <string_view>

#include "song.h"
#include "types.h"
#include "util/bin.h"
#include "util/gba.h"
#include "util/file.h"
#include "error.h"

extern std::vector<u8> file;

// adapted from
// Saptapper: Automated GSF ripper for MusicPlayer2000.
// at
// https://github.com/loveemu/saptapper/blob/2da643b718ec6308c4064f5aae769c035c496368/src/saptapper/mp2k_driver.cpp#L165

static bool memcmp_loose(const u8* buf1, const u8* buf2, size_t n,
                         unsigned int max_diff) {
  unsigned int diff = 0;
  for (size_t pos = 0; pos < n; pos++) {
    if (buf1[pos] != buf2[pos]) {
      if (++diff >= max_diff) return false;
    }
  }
  return true;
}

static const u8* find_loose(std::string_view pattern, unsigned int max_diff, const u8* pos = nullptr) {
  if (file.size() < pattern.size()) return nullptr;
  if (!pos) pos = file.data();

  constexpr u32 align = 4;
  for (const u8* searchpos = pos; searchpos < file.data() + file.size(); searchpos += align) {
    if (memcmp_loose(searchpos, reinterpret_cast<const u8*>(pattern.data()), pattern.size(), max_diff))
      return searchpos;
  }
  return nullptr;
}

void Mp2kDriver::Init(const std::string& filename) {
  player = nullptr;
  util::LoadFile(filename);
  song_table = FindSongTable();
  song_count = ReadSongCount(song_table);
}

void Mp2kDriver::SelectSong(u32 index) {
  if (index >= song_count) {
    Error("Out of bounds song requested");
  }
  player = std::make_unique<Player>(Song::Extract(GetSongPtr(index)));
}

const u8* Mp2kDriver::FindSelectSongFn() {
  using namespace std::literals::string_view_literals;
  const std::string_view pattern{
      "\x00\xb5\x00\x04\x07\x4a\x08\x49\x40\x0b"sv
      "\x40\x18\x83\x88\x59\x00\xc9\x18\x89\x00"sv
      "\x89\x18\x0a\x68\x01\x68\x10\x1c\x00\xf0"sv};
  return find_loose(pattern, 8);
}

const u8* Mp2kDriver::FindSongTable() {
  auto select_song_fn = FindSelectSongFn();
  if (select_song_fn == nullptr) return nullptr;

  const u32 song_table = util::Read<u32>(select_song_fn + 40);
  if (!util::ValidPointer(song_table)) return nullptr;
  return util::GetPointer(song_table);
}

int Mp2kDriver::ReadSongCount(const u8* song_table) {
  if (song_table == nullptr) return 0;

  int song_count = 0;
  for (const u8* searchpos = song_table; searchpos <= file.data() + file.size(); searchpos += 8) {
    u32 song = util::Read<u32>(searchpos);
    if (!util::ValidPointer(song)) break;
    const u8* songptr = util::GetPointer(song);
    if (!Song::Validate(songptr)) break;
    song_count++;
  }
  return song_count;
}

const u8* Mp2kDriver::GetSongPtr(u32 index) {
  if (index >= song_count) {
    Error("Out of bounds song requested");
  }
  return util::GetPointer(util::Read<u32>(song_table + 8 * index));
}