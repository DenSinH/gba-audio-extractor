#include "extractor/player.h"
#include "frontend/frontend.h"
#include "util/file.h"
#include "util/gba.h"
#include "util/bin.h"
#include "extractor/mp2k_driver.h"

#include <string>


int main() {
  const std::string file_name = "D:\\Projects\\CProjects\\gba-audio-extractor\\files\\zelda.gba";
  util::LoadFile(file_name);
  auto driver = Mp2kDriver();
  driver.Init();

  // agbplay: https://github.com/ipatix/agbplay/tree/master
  // song data structure:
  // https://github.com/zeldaret/tmc/blob/cd2b8d4b531ee5b4b78d04964a3f895edc31db73/include/gba/m4a.h
  // gSongTable references in 
  // https://github.com/zeldaret/tmc/blob/master/src/sound.c
  // https://github.com/zeldaret/tmc/blob/master/src/gba/m4a.c
  // which are both "default" libgba files
  // find song table from saptapper
  // https://github.com/loveemu/saptapper/blob/2da643b718ec6308c4064f5aae769c035c496368/src/saptapper/mp2k_driver.cpp#L165

  // offsets, see https://github.com/zeldaret/tmc/blob/cd2b8d4b531ee5b4b78d04964a3f895edc31db73/assets/sounds.json#L665
  // bgmBossTheme
//  u32 start = 14524276;
//  u32 header_offset = 1156;
  // bgmMinishCap
//  u32 start = 14533836;
//  u32 header_offset = 2652;
  // bgmCloudTops
  u32 start = 14506168;
  u32 header_offset = 3140;
  // bgmDarkHyruleCastle
//  u32 start = 14509348;
//  u32 header_offset = 1752;
  // bgmStory
//  u32 start = 14485172;
//  u32 header_offset = 992;
  // bgmCaveOfFlames
//  u32 start = 14512520;
//  u32 header_offset = 1256;
  // bgmPicoriFestival
//  u32 start = 14538036;
//  u32 header_offset = 1308;
  // bgmFileSelect (high reverb (60))
//  u32 start = 14471404;
//  u32 header_offset = 960;
  // sfxRupeeBounce
//  u32 start = 14545056;
//  u32 header_offset = 28;

  // auto song   = Song::Extract(util::GetPointer(start + header_offset));
  auto song = Song::Extract(driver.GetSongPtr(34));
  static constexpr double SampleRate = 44100;
  auto player = Player(&song, SampleRate);

//  player.SkipToTick(690);
//  player.paused = true;
//  player.ToggleTrackEnable(0);
//  player.ToggleTrackEnable(1);
//  player.ToggleTrackEnable(2);
//  player.ToggleTrackEnable(3);
//  player.ToggleTrackEnable(4);
//  player.ToggleTrackEnable(5);
//  player.ToggleTrackEnable(6);
//  player.ToggleTrackEnable(7);

  frontend::Run(&player);

  return 0;
}
