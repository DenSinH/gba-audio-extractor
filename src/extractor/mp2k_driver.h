#pragma once

#include <string>
#include <memory>
#include "types.h"
#include "player.h"


struct Mp2kDriver {
  
  const u8* song_table = nullptr;
  int song_count = 0;
  std::unique_ptr<Player> player = nullptr;

  void Init(const std::string& filename);
  void SelectSong(u32 index);

private:
  const u8* GetSongPtr(u32 index);
  static const u8* FindSelectSongFn();
  static const u8* FindSongTable();
  static int ReadSongCount(const u8* song_table);
};
