#pragma once

#include "types.h"
#include "error.h"

#include <iostream>
#include <iterator>
#include <fstream>
#include <vector>
#include <string>

namespace util {

static std::vector<u8> LoadBin(const std::string& path) {
  std::basic_ifstream<u8> file(path, std::ios::binary);
  if (!file.good()) {
    Error("Failed to open file: %s", path.c_str());
  }

  std::vector<u8> bin;
  bin.reserve(file.tellg());
  bin.assign(std::istreambuf_iterator<u8>{file}, {});
  Debug("Loaded %llx bytes from %s", bin.size(), path.c_str());
  return std::move(bin);
}

}