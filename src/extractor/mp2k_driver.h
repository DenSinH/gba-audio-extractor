// adapted from:
// Saptapper: Automated GSF ripper for MusicPlayer2000.
// at https://github.com/loveemu/saptapper/blob/2da643b718ec6308c4064f5aae769c035c496368/src/saptapper/mp2k_driver.hpp

#ifndef SAPTAPPER_MP2K_DRIVER_HPP_
#define SAPTAPPER_MP2K_DRIVER_HPP_

#include <string>
#include <string_view>
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

#endif