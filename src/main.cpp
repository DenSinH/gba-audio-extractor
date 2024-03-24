#include "extractor/player.h"
#include "frontend/frontend.h"
#include "util/file.h"
#include "util/gba.h"
#include "util/bin.h"
#include "extractor/mp2k_driver.h"

#include <string>


int main() {
  const std::string file_name = "D:\\Projects\\CProjects\\gba-audio-extractor\\files\\zelda.gba";
  auto driver = Mp2kDriver();
  // driver.Init(file_name);
  // driver.SelectSong(34);  // cloud tops

  // agbplay: https://github.com/ipatix/agbplay/tree/master
  // song data structure:
  // https://github.com/zeldaret/tmc/blob/cd2b8d4b531ee5b4b78d04964a3f895edc31db73/include/gba/m4a.h
  // gSongTable references in 
  // https://github.com/zeldaret/tmc/blob/master/src/sound.c
  // https://github.com/zeldaret/tmc/blob/master/src/gba/m4a.c
  // which are both "default" libgba files
  // find song table from saptapper
  // https://github.com/loveemu/saptapper/blob/2da643b718ec6308c4064f5aae769c035c496368/src/saptapper/mp2k_driver.cpp#L165

  // commands: https://loveemu.github.io/vgmdocs/Summary_of_GBA_Standard_Sound_Driver_MusicPlayer2000.html
  // and gba2mid / mid2gba

  // offsets, see https://github.com/zeldaret/tmc/blob/cd2b8d4b531ee5b4b78d04964a3f895edc31db73/assets/sounds.json#L665

  frontend::Run(&driver);

  return 0;
}
