#include "extractor/extractor.h"
#include "file.h"

#include <string>
#include <memory>

const u8* file;


int main() {
  const std::string file_name = "D:\\Projects\\CProjects\\MusicExtractor\\files\\zelda.gba";
  const auto file_data = util::LoadBin(file_name);
  file = file_data.data();

  auto extractor = Extractor();

  // bgmBossTheme
  u32 start = 14524276;
  u32 size = 1196;
  u32 header_offset = 1156;

  extractor.Extract(start, size, header_offset);


  return 0;
}
