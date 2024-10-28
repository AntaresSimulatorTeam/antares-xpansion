
#include "antares-xpansion/lpnamer/problem_modifier/MPSFileWriter.h"

#include <utility>

#include "antares-xpansion/lpnamer/problem_modifier/IProblemWriter.h"
#include "antares-xpansion/lpnamer/problem_modifier/LinkProblemsGenerator.h"

void MPSFileWriter::Write_problem(Problem *in_prblm) {
  auto const lp_mps_name = lp_dir_ / in_prblm->_name;
  in_prblm->write_prob_mps(lp_mps_name);
}

MPSFileWriter::MPSFileWriter(std::filesystem::path lp_dir)
    : lp_dir_(std::move(lp_dir)) {}
