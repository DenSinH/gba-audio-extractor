#include "extractor/player.h"
#include "frontend/frontend.h"
#include "util/file.h"
#include "util/gba.h"

#include <string>

const u8* file;


int main() {
  const std::string file_name = "D:\\Projects\\CProjects\\MusicExtractor\\files\\zelda.gba";
  const auto file_data = util::LoadBin(file_name);
  file = file_data.data();

  // bgmBossTheme
//  u32 start = 14524276;
//  u32 header_offset = 1156;
  // bgmMinishCap
//  u32 start = 14533836;
//  u32 header_offset = 2652;
  // bgmCloudTops
//  u32 start = 14506168;
//  u32 header_offset = 3140;
  // bgmDarkHyruleCastle
//  u32 start = 14509348;
//  u32 header_offset = 1752;
  // bgmStory
//  u32 start = 14485172;
//  u32 header_offset = 992;
  // todo: cursor goes off screen
  // bgmCaveOfFlames
//  u32 start = 14512520;
//  u32 header_offset = 1256;
  // bgmPicoriFestival
//  u32 start = 14538036;
//  u32 header_offset = 1308;
  // bgmFileSelect (high reverb (60))
  u32 start = 14471404;
  u32 header_offset = 960;
  // sfxRupeeBounce
//  u32 start = 14545056;
//  u32 header_offset = 28;

  auto song   = Song::Extract(util::GetPointer(start + header_offset));
  static constexpr double SampleRate = 44100;
  auto player = Player(&song, 44100);

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
