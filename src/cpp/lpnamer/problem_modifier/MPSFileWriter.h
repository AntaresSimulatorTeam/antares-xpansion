#pragma once

#include "IProblemWriter.h"
#include "LinkProblemsGenerator.h"

class MPSFileWriter : public IProblemWriter {
  void Write_problem(std::shared_ptr<Problem>& in_prblm) override;

 public:
  MPSFileWriter(std::filesystem::path lp_dir);
  std::filesystem::path lp_dir_;
};
