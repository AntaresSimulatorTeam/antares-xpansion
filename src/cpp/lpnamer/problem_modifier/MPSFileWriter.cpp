
#include "antares-xpansion/lpnamer/problem_modifier/MPSFileWriter.h"

#include <utility>

#include "antares-xpansion/lpnamer/problem_modifier/IProblemWriter.h"
#include "antares-xpansion/lpnamer/problem_modifier/LinkProblemsGenerator.h"

void MPSFileWriter::Write_problem(Problem *in_prblm, const std::filesystem::path &output_file) {
    in_prblm->write_prob_mps(output_file);
}

MPSFileWriter::MPSFileWriter(std::filesystem::path lp_dir)
    : lp_dir_(std::move(lp_dir)) {}
