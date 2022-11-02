//
// Created by marechaljas on 08/11/22.
//

#pragma once

#include "IProblemWriter.h"
#include "LinkProblemsGenerator.h"

class ArchiveProblemWriter : public IProblemWriter {
  void Write_problem(std::shared_ptr<Problem>& in_prblm) override;

 public:
  ArchiveProblemWriter(std::filesystem::path lp_dir,
                       std::shared_ptr<ArchiveWriter> writer);
  std::filesystem::path lp_dir_;
  std::shared_ptr<ArchiveWriter> writer_;
};
