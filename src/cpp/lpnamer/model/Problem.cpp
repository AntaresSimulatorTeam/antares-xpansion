//
// Created by marechaljas on 27/04/2022.
//

#include "antares-xpansion/lpnamer/model/Problem.h"

#include "antares-xpansion/lpnamer/model/ProblemNameParser.h"

void Problem::read_prob_mps(const std::filesystem::path& filename) {
  std::tie(mc_year, week) = McYearAndWeek(filename);
  solver_abstract_->read_prob_mps(filename);
}
void Problem::save_prob(const std::filesystem::path& filename) {
  solver_abstract_->save_prob(filename);
}
void Problem::restore_prob(const std::filesystem::path& filename) {
  solver_abstract_->restore_prob(filename);
}
