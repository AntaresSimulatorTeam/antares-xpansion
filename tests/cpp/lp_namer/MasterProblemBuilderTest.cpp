//
// Created by s90365 on 23/08/2021.
//
#include <antares-xpansion/helpers/solver_utils.h>

#include "LoggerBuilder.h"
#include "antares-xpansion/lpnamer/problem_modifier/MasterProblemBuilder.h"
#include "gtest/gtest.h"

TEST(MasterProblemBuilderTest, test_one_candidate_not_integer) {
  std::string solver_name = "CBC";
  std::string master_formulation = "integer";

  CandidateData cand1;
  cand1.link_id = 1;
  cand1.name = "transmission_line_1";
  cand1.link_name = "area1 - area2";
  cand1.already_installed_capacity = 0;

  std::vector<CandidateData> cand_data_list = {cand1};

  std::map<std::string, std::vector<LinkProfile>> profile_map;

  auto logger = emptyLogger();
  ActiveLinksBuilder linkBuilder{cand_data_list, profile_map, logger};
  const std::vector<ActiveLink>& links = linkBuilder.getLinks();

  std::vector<Candidate> candidates;

  for (const auto& link : links) {
    const auto& candidateFromLink = link.getCandidates();
    candidates.insert(candidates.end(), candidateFromLink.begin(),
                      candidateFromLink.end());
  }
  auto solver_log_manager = SolverLogManager("");
  auto master_problem = MasterProblemBuilder(master_formulation)
                            .build(solver_name, candidates, solver_log_manager);
  ASSERT_EQ(master_problem->get_ncols(), 1);
  ASSERT_EQ(
      master_problem->get_col_names(0, master_problem->get_ncols() - 1)[0],
      cand1.name);

  std::vector<char> colTypeArray(master_problem->get_ncols());
  master_problem->get_col_type(colTypeArray.data(), 0,
                               master_problem->get_ncols() - 1);

  ASSERT_EQ(colTypeArray[0], 'C');

  std::vector<double> varLbArray(master_problem->get_ncols());
  master_problem->get_lb(varLbArray.data(), 0, master_problem->get_ncols() - 1);

  ASSERT_EQ(varLbArray[0], 0);

  std::vector<double> varUbArray(master_problem->get_ncols());
  master_problem->get_ub(varUbArray.data(), 0, master_problem->get_ncols() - 1);

  ASSERT_EQ(varUbArray[0], 0);

  ASSERT_EQ(master_problem->get_nrows(), 0);
}

TEST(MasterProblemBuilderTest, test_one_candidate_integer_problem_integer) {
  std::string solver_name = "CBC";
  std::string master_formulation = "integer";

  CandidateData cand1;
  cand1.link_id = 1;
  cand1.name = "transmission_line_1";
  cand1.link_name = "area1 - area2";
  cand1.already_installed_capacity = 0;
  cand1.unit_size = 4;
  cand1.max_units = 17;

  std::vector<CandidateData> cand_data_list = {cand1};

  std::map<std::string, std::vector<LinkProfile>> profile_map;

  auto logger = emptyLogger();
  ActiveLinksBuilder linkBuilder{cand_data_list, profile_map, logger};
  const std::vector<ActiveLink>& links = linkBuilder.getLinks();

  std::vector<Candidate> candidates;

  for (const auto& link : links) {
    const auto& candidateFromLink = link.getCandidates();
    candidates.insert(candidates.end(), candidateFromLink.begin(),
                      candidateFromLink.end());
  }

  auto solver_log_manager = SolverLogManager("");
  auto master_problem = MasterProblemBuilder(master_formulation)
                            .build(solver_name, candidates, solver_log_manager);
  ASSERT_EQ(master_problem->get_ncols(), 2);

  std::vector<char> colTypeArray(master_problem->get_ncols());
  master_problem->get_col_type(colTypeArray.data(), 0,
                               master_problem->get_ncols() - 1);

  ASSERT_EQ(colTypeArray[1], 'I');

  std::vector<std::string> colNameArray(master_problem->get_ncols());
  colNameArray =
      master_problem->get_col_names(0, master_problem->get_ncols() - 1);
  ASSERT_EQ(colNameArray[1], NB_UNITS_VAR_PREFIX + "transmission_line_1");

  std::vector<double> varLbArray(master_problem->get_ncols());
  master_problem->get_lb(varLbArray.data(), 0, master_problem->get_ncols() - 1);

  ASSERT_EQ(varLbArray[1], 0);

  std::vector<double> varUbArray(master_problem->get_ncols());
  master_problem->get_ub(varUbArray.data(), 0, master_problem->get_ncols() - 1);

  ASSERT_EQ(varUbArray[1], 17);

  ASSERT_EQ(master_problem->get_nrows(), 1);

  std::vector<char> rowTypeArray(master_problem->get_nrows());
  master_problem->get_row_type(rowTypeArray.data(), 0,
                               master_problem->get_nrows() - 1);

  ASSERT_EQ(rowTypeArray[0], 'E');
}

TEST(MasterProblemBuilderTest, test_one_candidate_integer_problem_relaxed) {
  std::string solver_name = "CBC";
  std::string master_formulation = "relaxed";

  CandidateData cand1;
  cand1.link_id = 1;
  cand1.name = "transmission_line_1";
  cand1.link_name = "area1 - area2";
  cand1.already_installed_capacity = 0;
  cand1.unit_size = 4;
  cand1.max_units = 17;

  std::vector<CandidateData> cand_data_list = {cand1};

  std::map<std::string, std::vector<LinkProfile>> profile_map;

  auto logger = emptyLogger();
  ActiveLinksBuilder linkBuilder{cand_data_list, profile_map, logger};
  const std::vector<ActiveLink>& links = linkBuilder.getLinks();

  std::vector<Candidate> candidates;

  for (const auto& link : links) {
    const auto& candidateFromLink = link.getCandidates();
    candidates.insert(candidates.end(), candidateFromLink.begin(),
                      candidateFromLink.end());
  }

  auto solver_log_manager = SolverLogManager("");
  auto master_problem = MasterProblemBuilder(master_formulation)
                            .build(solver_name, candidates, solver_log_manager);
  ASSERT_EQ(master_problem->get_ncols(), 1);
  ASSERT_EQ(
      master_problem->get_col_names(0, master_problem->get_ncols() - 1)[0],
      cand1.name);

  std::vector<char> colTypeArray(master_problem->get_ncols());
  master_problem->get_col_type(colTypeArray.data(), 0,
                               master_problem->get_ncols() - 1);

  ASSERT_EQ(colTypeArray[0], 'C');

  std::vector<double> varLbArray(master_problem->get_ncols());
  master_problem->get_lb(varLbArray.data(), 0, master_problem->get_ncols() - 1);

  ASSERT_EQ(varLbArray[0], 0);

  std::vector<double> varUbArray(master_problem->get_ncols());
  master_problem->get_ub(varUbArray.data(), 0, master_problem->get_ncols() - 1);

  ASSERT_EQ(varUbArray[0], 68);

  ASSERT_EQ(master_problem->get_nrows(), 0);
}
