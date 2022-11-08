//
// Created by marechaljas on 27/04/2022.
//

#include "Problem.h"

#include "ProblemNameParser.h"

void Problem::read_prob_mps(const std::filesystem::path& filename) {
  mc_year = MCYear(filename);
  solver_abstract_->read_prob_mps(filename);
  _name = filename.filename();
}
