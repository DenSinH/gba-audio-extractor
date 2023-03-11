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
  u32 start = 14533836;
  u32 header_offset = 2652;

  auto song   = Song::Extract(util::GetPointer(start + header_offset));
  auto player = Player(&song);

//  player.SkipToTick(2400);
//  player.SetTrackEnable(0, false);
//  player.SetTrackEnable(1, false);
//  player.SetTrackEnable(2, false);
//  player.SetTrackEnable(3, false);
//  player.SetTrackEnable(4, false);
//  player.SetTrackEnable(5, false);
//  player.SetTrackEnable(6, false);
//  player.SetTrackEnable(7, false);

  frontend::Run(&player);

  return 0;
}
