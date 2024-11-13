#pragma once
#include "common.h"

class CouplingMapGenerator {
 public:
  static CouplingMap BuildInput(const std::filesystem::path &structure_path);
};
