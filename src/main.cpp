#include "extractor/song.h"
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
  u32 start = 14524276;
  u32 header_offset = 1156;

  auto song = Song::Extract(util::GetPointer(start + header_offset));

  frontend::Run();

  return 0;
}
