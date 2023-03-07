
#include "MPSFileWriter.h"

#include <utility>

#include "IProblemWriter.h"
#include "LinkProblemsGenerator.h"

void MPSFileWriter::Write_problem(std::shared_ptr<Problem> &in_prblm) {
  auto const lp_mps_name = lp_dir_ / in_prblm->_name;
  in_prblm->write_prob_mps(lp_mps_name);
}

MPSFileWriter::MPSFileWriter(std::filesystem::path lp_dir)
    : lp_dir_(std::move(lp_dir)) {}
