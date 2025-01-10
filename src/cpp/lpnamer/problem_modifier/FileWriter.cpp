
#include <utility>

#include "antares-xpansion/lpnamer/problem_modifier/FileWriter.h"
#include "antares-xpansion/lpnamer/problem_modifier/IProblemWriter.h"
#include "antares-xpansion/lpnamer/problem_modifier/LinkProblemsGenerator.h"

void FileWriter::Write_problem(Problem *in_prblm, const std::filesystem::path &output_file) {
  if (output_file.extension() == ".mps") {
    in_prblm->write_prob_mps(output_file);
  } else {
    in_prblm->save_prob(output_file);
  }
}

FileWriter::FileWriter(std::filesystem::path lp_dir)
    : lp_dir_(std::move(lp_dir)) {}
