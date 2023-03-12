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
  u32 start = 14509348;
  u32 header_offset = 1752;

  auto song   = Song::Extract(util::GetPointer(start + header_offset));
  auto player = Player(&song);

  player.SkipToTick(690);
  player.paused = true;
//  player.ToggleTrackEnable(0);
  player.ToggleTrackEnable(1);
  player.ToggleTrackEnable(2);
  player.ToggleTrackEnable(3);
  player.ToggleTrackEnable(4);
  player.ToggleTrackEnable(5);
  player.ToggleTrackEnable(6);
//  player.ToggleTrackEnable(7);

  frontend::Run(&player);

  return 0;
}
