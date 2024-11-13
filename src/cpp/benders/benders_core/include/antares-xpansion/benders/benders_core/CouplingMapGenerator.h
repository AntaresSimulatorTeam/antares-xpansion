#pragma once
#include "common.h"

struct CouplingMapGenerator {
  static CouplingMap build_input(const std::filesystem::path &structure_path);
};
