//
// Created by marechaljas on 08/11/22.
//

#include "ArchiveProblemWriter.h"

#include <utility>

#include "IProblemWriter.h"
#include "LinkProblemsGenerator.h"

void ArchiveProblemWriter::Write_problem(std::shared_ptr<Problem> &in_prblm) {
  auto const lp_mps_name = lp_dir_ / in_prblm->_name;
  in_prblm->write_prob_mps(lp_mps_name);
  writer_->AddFileInArchive(lp_mps_name);
  std::filesystem::remove(lp_mps_name);
}

ArchiveProblemWriter::ArchiveProblemWriter(
    std::filesystem::path lp_dir, std::shared_ptr<ArchiveWriter> writer)
    : lp_dir_(std::move(lp_dir)), writer_(std::move(writer)) {}
