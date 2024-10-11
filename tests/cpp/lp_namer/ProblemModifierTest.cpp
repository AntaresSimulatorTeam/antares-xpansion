
#include "antares-xpansion/lpnamer/model/ActiveLinks.h"
#include <ProblemModifier.h>
#include <multisolver_interface/SolverFactory.h>
#include <antares-xpansion/helpers/solver_utils.h>

#include <fstream>

#include "LoggerBuilder.h"
#include "antares-xpansion/lpnamer/model/Problem.h"
#include "gtest/gtest.h"

const std::string P_LINK = "p_link";
const std::string P_PLUS = "p_plus";
const std::string P_MINUS = "p_minus";
const double ZERO = 0.0;
const double PLUS_INF = 1e20;
const double MINUS_INF = -1e20;

class ProblemModifierTest : public ::testing::Test {
 public:
  std::shared_ptr<Problem> math_problem;
  int n_cols = -1;
  int n_rows = -1;
  int n_elems = -1;
  std::vector<char> coltypes;
  std::vector<char> rowtypes;
  std::vector<double> objectives;
  std::vector<double> upper_bounds;
  std::vector<double> lower_bounds;
  std::vector<double> rhs;
  std::vector<double> mat_val;
  std::vector<int> col_indexes;
  std::vector<int> start_indexes;
  std::vector<std::basic_string<char>> col_names;
  ProblemGenerationLog::ProblemGenerationLoggerSharedPointer logger =
      emptyLogger();

 protected:
  void SetUp() {
    std::string solver_name = "CBC";
    SolverFactory factory;
    math_problem =
        std::make_shared<Problem>(factory.create_solver(solver_name));

    // Add NTC variable column
    {
      std::vector<double> objectives(1, 1);
      std::vector<double> lb(1, -1000);
      std::vector<double> ub(1, 1000);
      std::vector<char> coltypes(1, 'C');
      std::vector<int> mstart(1, 0);
      std::vector<std::string> candidates_colnames(1, P_LINK);
      solver_addcols(*math_problem, objectives, mstart, {}, {}, lb, ub,
                     coltypes, candidates_colnames);
    }

    // Add direct cost variable column
    {
      std::vector<double> objectives(1, 1);
      std::vector<double> lb(1, 0);
      std::vector<double> ub(1, 1);
      std::vector<char> coltypes(1, 'C');
      std::vector<int> mstart(1, 0);
      std::vector<std::string> candidates_colnames(1, P_PLUS);
      solver_addcols(*math_problem, objectives, mstart, {}, {}, lb, ub,
                     coltypes, candidates_colnames);
    }

    // Add indirect cost variable column
    {
      std::vector<double> objectives(1, 1);
      std::vector<double> lb(1, 0);
      std::vector<double> ub(1, 1);
      std::vector<char> coltypes(1, 'C');
      std::vector<int> mstart(1, 0);
      std::vector<std::string> candidates_colnames(1, P_MINUS);
      solver_addcols(*math_problem, objectives, mstart, {}, {}, lb, ub,
                     coltypes, candidates_colnames);
    }
  }

  void TearDown() {
    n_cols = -1;
    n_rows = -1;
    n_elems = -1;
  }

  void update_n_cols() {
    if (n_cols == -1) {
      n_cols = math_problem->get_ncols();
    }
  }
  void update_n_rows() {
    if (n_rows == -1) {
      n_rows = math_problem->get_nrows();
    }
  }
  void update_n_elems() {
    if (n_elems == -1) {
      n_elems = math_problem->get_nelems();
    }
  }
  void update_col_names() {
    update_n_cols();
    col_names = math_problem->get_col_names(0, n_cols - 1);
  }
  void update_col_type() {
    update_n_cols();
    if (coltypes.size() != n_cols) {
      std::vector<char> buffer(n_cols, '0');
      coltypes.insert(coltypes.begin(), buffer.begin(), buffer.end());
      math_problem->get_col_type(coltypes.data(), 0, n_cols - 1);
    }
  }
  void update_row_type() {
    update_n_rows();
    if (rowtypes.size() != n_rows) {
      std::vector<char> buffer(n_rows, '0');
      rowtypes.insert(rowtypes.begin(), buffer.begin(), buffer.end());
      math_problem->get_row_type(rowtypes.data(), 0, n_rows - 1);
    }
  }
  void update_objectives() {
    update_n_cols();
    if (objectives.size() != n_cols) {
      std::vector<double> buffer(n_cols, -777);
      objectives.insert(objectives.begin(), buffer.begin(), buffer.end());
      math_problem->get_obj(objectives.data(), 0, n_cols - 1);
    }
  }
  void update_lower_bounds() {
    update_n_cols();
    if (lower_bounds.size() != n_cols) {
      std::vector<double> buffer(n_cols, -777);
      lower_bounds.insert(lower_bounds.begin(), buffer.begin(), buffer.end());
      math_problem->get_lb(lower_bounds.data(), 0, n_cols - 1);
    }
  }
  void update_upper_bounds() {
    update_n_cols();
    if (upper_bounds.size() != n_cols) {
      std::vector<double> buffer(n_cols, -777);
      upper_bounds.insert(upper_bounds.begin(), buffer.begin(), buffer.end());
      math_problem->get_ub(upper_bounds.data(), 0, n_cols - 1);
    }
  }
  void update_mat_val() {
    update_n_rows();
    update_n_elems();

    if (mat_val.size() != n_elems) {
      std::vector<double> buffer(n_elems, -777);
      mat_val.insert(mat_val.begin(), buffer.begin(), buffer.end());

      start_indexes.clear();
      start_indexes = std::vector<int>(n_rows + 1);
      col_indexes.clear();
      col_indexes = std::vector<int>(n_elems);
      int n_returned(0);
      math_problem->get_rows(start_indexes.data(), col_indexes.data(),
                             mat_val.data(), n_elems, &n_returned, 0,
                             n_rows - 1);
    }
  }

  std::vector<double> getRowCoefficients(int index_row) {
    update_mat_val();
    std::vector<double> row;
    row.insert(row.begin(), mat_val.begin() + start_indexes.at(index_row),
               mat_val.begin() + start_indexes.at(index_row + 1));
    return row;
  }

  std::vector<int> getRowColIndexes(int index_row) {
    update_mat_val();
    std::vector<int> index;
    index.insert(index.begin(),
                 col_indexes.begin() + start_indexes.at(index_row),
                 col_indexes.begin() + start_indexes.at(index_row + 1));
    return index;
  }

  void update_rhs_val() {
    update_n_rows();
    if (rhs.size() != n_rows) {
      std::vector<double> buffer(n_rows, -777);
      rhs.insert(rhs.begin(), buffer.begin(), buffer.end());
      math_problem->get_rhs(rhs.data(), 0, n_rows - 1);
    }
  }
  void verify_columns_are(const int expected_n_cols) {
    update_n_cols();
    ASSERT_EQ(n_cols, expected_n_cols);
  }
  void verify_rows_are(const int expected_n_rows) {
    update_n_rows();
    ASSERT_EQ(n_rows, expected_n_rows);
  }
  void verify_column_name_is(const int col_id, std::basic_string<char> name) {
    update_col_names();
    ASSERT_EQ(col_names.at(col_id), name);
  }
  void verify_column_is_of_type(const int col_id, char type) {
    update_col_type();
    ASSERT_EQ(coltypes.at(col_id), type);
  }
  void verify_column_objective_is(const int col_id, double obj_value) {
    update_objectives();
    ASSERT_DOUBLE_EQ(objectives.at(col_id), obj_value);
  }
  void verify_column_lower_bound_is(const int col_id, double lower_value) {
    update_lower_bounds();
    ASSERT_DOUBLE_EQ(lower_bounds.at(col_id), lower_value);
  }
  void verify_column_upper_bound_is(const int col_id, double upper_value) {
    update_upper_bounds();
    ASSERT_DOUBLE_EQ(upper_bounds.at(col_id), upper_value);
  }
  void verify_row_is_of_type(const int row_id, char type) {
    update_row_type();
    ASSERT_EQ(rowtypes.at(row_id), type);
  }
  void verify_rhs_is(const int rhs_id, double rhs_value) {
    update_rhs_val();
    ASSERT_DOUBLE_EQ(rhs.at(rhs_id), rhs_value);
  }
  void verify_column(const int col_id, std::basic_string<char> name, char type,
                     double obj_value, double lower_value, double upper_value) {
    verify_column_name_is(col_id, name);
    verify_column_is_of_type(col_id, type);
    verify_column_objective_is(col_id, obj_value);
    verify_column_lower_bound_is(col_id, lower_value);
    verify_column_upper_bound_is(col_id, upper_value);
  }
  void verify_row(int row, char type, const std::vector<double>& coeff,
                  const std::vector<int>& col_indexes, double rhs) {
    verify_row_is_of_type(row, type);
    ASSERT_EQ(getRowCoefficients(row), coeff);
    ASSERT_EQ(getRowColIndexes(row), col_indexes);
    verify_rhs_is(row, rhs);
  }
};

TEST_F(ProblemModifierTest, empty_test_the_multisolver_interface) {
  verify_columns_are(3);
  verify_rows_are(0);

  verify_column(0, P_LINK, 'C', 1, -1000, 1000);
  verify_column(1, P_PLUS, 'C', 1, 0, 1);
  verify_column(2, P_MINUS, 'C', 1, 0, 1);
}

TEST_F(ProblemModifierTest,
       One_link_no_candidates_link_boundaries_are_removed) {
  const int link_id = 0;
  const std::map<linkId, ColumnsToChange> p_var_columns = {{link_id, {{0, 0}}}};
  const std::map<linkId, ColumnsToChange> p_direct_cost_columns = {
      {link_id, {{{1, 0}}}}};
  const std::map<linkId, ColumnsToChange> p_indirect_cost_columns = {
      {link_id, {{{2, 0}}}}};
  ActiveLink link(link_id, "dummy_link", "from", "to", 0, logger);
  const std::vector<ActiveLink> active_links = {link};

  auto problem_modifier = ProblemModifier(logger);
  problem_modifier.changeProblem(math_problem.get(), active_links,
                                 p_var_columns, p_direct_cost_columns, p_indirect_cost_columns);

  verify_columns_are(3);

  verify_column(0, P_LINK, 'C', 1, MINUS_INF, PLUS_INF);
  verify_column(1, P_PLUS, 'C', 1, ZERO, PLUS_INF);
  verify_column(2, P_MINUS, 'C', 1, ZERO, PLUS_INF);

  try {
    problem_modifier.get_candidate_col_id("invalid_cand_name");
    FAIL();
  } catch (const ProblemModifier::CandidateWasNotAddedInProblem& expected) {
    ASSERT_STREQ(expected.ErrorMessage().c_str(),
                 "Candidate 'invalid_cand_name' not added in problem");
  }
}

TEST_F(ProblemModifierTest, One_link_two_candidates) {
  const int link_id = 0;
  std::vector<int> time_steps = {0};
  const std::map<linkId, ColumnsToChange> p_var_columns = {{link_id, {{0, 0}}}};
  const std::map<linkId, ColumnsToChange> p_direct_cost_columns = {
      {link_id, {{1, 0}}}};
  const std::map<linkId, ColumnsToChange> p_indirect_cost_columns = {
      {link_id, {{2, 0}}}};

  const double link_capacity = 1000.0;
  CandidateData cand1;
  cand1.link_id = link_id;
  cand1.name = "candy1";
  cand1.link_name = "dummy_link";
  cand1.already_installed_capacity = link_capacity;
  CandidateData cand2;
  cand2.link_id = link_id;
  cand2.name = "candy2";
  cand2.link_name = "dummy_link";
  cand2.already_installed_capacity = link_capacity;

  std::vector<CandidateData> cand_data_list = {cand1, cand2};
  std::map<std::string, std::vector<LinkProfile>> profile_map;

  ActiveLinksBuilder linkBuilder{cand_data_list, profile_map, logger};
  const std::vector<ActiveLink>& links = linkBuilder.getLinks();

  auto problem_modifier = ProblemModifier(logger);
  problem_modifier.changeProblem(math_problem.get(), links, p_var_columns,
                                 p_direct_cost_columns,
                                 p_indirect_cost_columns);

  verify_columns_are(5);

  const int P_LINK_id = 0;
  const int P_PLUS_id = 1;
  const int P_MINUS_id = 2;
  const int cand1_id = 3;
  const int cand2_id = 4;

  verify_column(P_LINK_id, P_LINK, 'C', 1, MINUS_INF, PLUS_INF);
  verify_column(P_PLUS_id, P_PLUS, 'C', 1, 0, PLUS_INF);
  verify_column(P_MINUS_id, P_MINUS, 'C', 1, 0, PLUS_INF);
  verify_column(cand1_id, cand1.name, 'C', 0, MINUS_INF, PLUS_INF);
  verify_column(cand2_id, cand2.name, 'C', 0, MINUS_INF, PLUS_INF);

  ASSERT_EQ(problem_modifier.get_candidate_col_id(cand1.name), cand1_id);
  ASSERT_EQ(problem_modifier.get_candidate_col_id(cand2.name), cand2_id);

  verify_rows_are(4);
  verify_row(0, 'L', {1, -1, -1}, {P_LINK_id, cand1_id, cand2_id},
             link_capacity);
  verify_row(1, 'G', {1, 1, 1}, {P_LINK_id, cand1_id, cand2_id},
             -link_capacity);
  verify_row(2, 'L', {1, -1, -1}, {P_PLUS_id, cand1_id, cand2_id},
             link_capacity);
  verify_row(3, 'L', {1, -1, -1}, {P_MINUS_id, cand1_id, cand2_id},
             link_capacity);
}

TEST_F(ProblemModifierTest, One_link_two_candidates_two_timestep_no_profile) {
  const int link_id = 0;
  const std::map<linkId, ColumnsToChange> p_var_columns = {
      {link_id, {{0, 0}, {0, 1}}}};
  const std::map<linkId, ColumnsToChange> p_direct_cost_columns = {
      {link_id, {{1, 0}, {1, 1}}}};
  const std::map<linkId, ColumnsToChange> p_indirect_cost_columns = {
      {link_id, {{{2, 0}, {2, 1}}}}};

  const double link_capacity = 1000.0;
  CandidateData cand1;
  cand1.link_id = link_id;
  cand1.name = "candy1";
  cand1.link_name = "dummy_link";
  cand1.already_installed_capacity = link_capacity;
  CandidateData cand2;
  cand2.link_id = link_id;
  cand2.name = "candy2";
  cand2.link_name = "dummy_link";
  cand2.already_installed_capacity = link_capacity;

  std::vector<CandidateData> cand_data_list = {cand1, cand2};
  std::map<std::string, std::vector<LinkProfile>> profile_map;

  ActiveLinksBuilder linkBuilder{cand_data_list, profile_map, logger};
  const std::vector<ActiveLink>& links = linkBuilder.getLinks();

  auto problem_modifier = ProblemModifier(logger);
  problem_modifier.changeProblem(math_problem.get(), links, p_var_columns,
                                 p_direct_cost_columns,
                                 p_indirect_cost_columns);
  verify_columns_are(5);

  const int P_LINK_id = 0;
  const int P_PLUS_id = 1;
  const int P_MINUS_id = 2;
  const int cand1_id = 3;
  const int cand2_id = 4;

  verify_column(P_LINK_id, P_LINK, 'C', 1, MINUS_INF, PLUS_INF);
  verify_column(P_PLUS_id, P_PLUS, 'C', 1, 0, PLUS_INF);
  verify_column(P_MINUS_id, P_MINUS, 'C', 1, 0, PLUS_INF);
  verify_column(cand1_id, cand1.name, 'C', 0, MINUS_INF, PLUS_INF);
  verify_column(cand2_id, cand2.name, 'C', 0, MINUS_INF, PLUS_INF);
  ASSERT_EQ(problem_modifier.get_candidate_col_id(cand1.name), cand1_id);
  ASSERT_EQ(problem_modifier.get_candidate_col_id(cand2.name), cand2_id);

  verify_rows_are(8);

  verify_row(0, 'L', {1, -1, -1}, {P_LINK_id, cand1_id, cand2_id},
             link_capacity);
  verify_row(1, 'G', {1, 1, 1}, {P_LINK_id, cand1_id, cand2_id},
             -link_capacity);
  verify_row(2, 'L', {1, -1, -1}, {P_LINK_id, cand1_id, cand2_id},
             link_capacity);
  verify_row(3, 'G', {1, 1, 1}, {P_LINK_id, cand1_id, cand2_id},
             -link_capacity);
  verify_row(4, 'L', {1, -1, -1}, {P_PLUS_id, cand1_id, cand2_id},
             link_capacity);
  verify_row(5, 'L', {1, -1, -1}, {P_PLUS_id, cand1_id, cand2_id},
             link_capacity);
  verify_row(6, 'L', {1, -1, -1}, {P_MINUS_id, cand1_id, cand2_id},
             link_capacity);
  verify_row(7, 'L', {1, -1, -1}, {P_MINUS_id, cand1_id, cand2_id},
             link_capacity);
}

TEST_F(ProblemModifierTest, One_link_two_candidates_two_timestep_profile) {
  const int link_id = 0;
  const std::map<linkId, ColumnsToChange> p_var_columns = {
      {link_id, {{0, 0}, {0, 1}}}};
  const std::map<linkId, ColumnsToChange> p_direct_cost_columns = {
      {link_id, {{1, 0}, {1, 1}}}};
  const std::map<linkId, ColumnsToChange> p_indirect_cost_columns = {
      {link_id, {{{2, 0}, {2, 1}}}}};

  const double link_capacity = 2000.0;
  CandidateData cand1;
  cand1.link_id = link_id;
  cand1.name = "candy1";
  cand1.link_name = "dummy_link";
  cand1.already_installed_capacity = link_capacity;
  cand1.installed_direct_link_profile_name = "install_link_profile";
  cand1.direct_link_profile = "profile_cand1";
  cand1.indirect_link_profile = "profile_cand1";
  CandidateData cand2;
  cand2.link_id = link_id;
  cand2.name = "candy2";
  cand2.link_name = "dummy_link";
  cand2.already_installed_capacity = link_capacity;
  cand2.installed_direct_link_profile_name = "install_link_profile";
  cand2.direct_link_profile = "profile_cand2";
  cand2.indirect_link_profile = "profile_cand2";

  std::vector<CandidateData> cand_data_list = {cand1, cand2};
  std::map<std::string, std::vector<LinkProfile>> profile_map;
  LinkProfile profile_link(logger);
  profile_link.direct_link_profile = {1, 2};
  profile_link.indirect_link_profile = {3, 4};
  profile_map["install_link_profile"] = {profile_link};
  LinkProfile profile_cand1(logger);
  profile_cand1.direct_link_profile = {0.5, 1};
  profile_cand1.indirect_link_profile = {0.8, 1.2};
  profile_map["profile_cand1"] = {profile_cand1};
  LinkProfile profile_cand2(logger);
  profile_cand2.direct_link_profile = {1.5, 1.7};
  profile_cand2.indirect_link_profile = {2.6, 2.8};
  profile_map["profile_cand2"] = {profile_cand2};

  ActiveLinksBuilder linkBuilder{cand_data_list, profile_map, logger};
  const std::vector<ActiveLink>& links = linkBuilder.getLinks();

  auto problem_modifier = ProblemModifier(logger);
  problem_modifier.changeProblem(math_problem.get(), links, p_var_columns,
                                 p_direct_cost_columns,
                                 p_indirect_cost_columns);

  const int P_LINK_id = 0;
  const int P_PLUS_id = 1;
  const int P_MINUS_id = 2;
  const int cand1_id = 3;
  const int cand2_id = 4;

  verify_column(P_LINK_id, P_LINK, 'C', 1, MINUS_INF, PLUS_INF);
  verify_column(P_PLUS_id, P_PLUS, 'C', 1, 0, PLUS_INF);
  verify_column(P_MINUS_id, P_MINUS, 'C', 1, 0, PLUS_INF);
  verify_column(cand1_id, cand1.name, 'C', 0, MINUS_INF, PLUS_INF);
  verify_column(cand2_id, cand2.name, 'C', 0, MINUS_INF, PLUS_INF);
  ASSERT_EQ(problem_modifier.get_candidate_col_id(cand1.name), cand1_id);
  ASSERT_EQ(problem_modifier.get_candidate_col_id(cand2.name), cand2_id);

  verify_rows_are(8);

  verify_row(0, 'L',
             {1, -profile_cand1.getDirectProfile(0),
              -profile_cand2.getDirectProfile(0)},
             {P_LINK_id, cand1_id, cand2_id},
             link_capacity * profile_link.getDirectProfile(0));

  verify_row(1, 'G',
             {1, profile_cand1.getIndirectProfile(0),
              profile_cand2.getIndirectProfile(0)},
             {P_LINK_id, cand1_id, cand2_id},
             -link_capacity * profile_link.getIndirectProfile(0));

  verify_row(2, 'L',
             {1, -profile_cand1.getDirectProfile(1),
              -profile_cand2.getDirectProfile(1)},
             {P_LINK_id, cand1_id, cand2_id},
             link_capacity * profile_link.getDirectProfile(1));

  verify_row(3, 'G',
             {1, profile_cand1.getIndirectProfile(1),
              profile_cand2.getIndirectProfile(1)},
             {P_LINK_id, cand1_id, cand2_id},
             -link_capacity * profile_link.getIndirectProfile(1));

  verify_row(4, 'L',
             {1, -profile_cand1.getDirectProfile(0),
              -profile_cand2.getDirectProfile(0)},
             {P_PLUS_id, cand1_id, cand2_id},
             link_capacity * profile_link.getDirectProfile(0));

  verify_row(5, 'L',
             {1, -profile_cand1.getDirectProfile(1),
              -profile_cand2.getDirectProfile(1)},
             {P_PLUS_id, cand1_id, cand2_id},
             link_capacity * profile_link.getDirectProfile(1));

  verify_row(6, 'L',
             {1, -profile_cand1.getIndirectProfile(0),
              -profile_cand2.getIndirectProfile(0)},
             {P_MINUS_id, cand1_id, cand2_id},
             link_capacity * profile_link.getIndirectProfile(0));

  verify_row(7, 'L',
             {1, -profile_cand1.getIndirectProfile(1),
              -profile_cand2.getIndirectProfile(1)},
             {P_MINUS_id, cand1_id, cand2_id},
             link_capacity * profile_link.getIndirectProfile(1));
}

TEST_F(ProblemModifierTest,
       One_link_two_candidates_one_candidate_with_empty_profile) {
  const int link_id = 0;
  const std::map<linkId, ColumnsToChange> p_var_columns = {
      {link_id, {{0, 0}, {0, 1}}}};
  const std::map<linkId, ColumnsToChange> p_direct_cost_columns = {
      {link_id, {{1, 0}, {1, 1}}}};
  const std::map<linkId, ColumnsToChange> p_indirect_cost_columns = {
      {link_id, {{{2, 0}, {2, 1}}}}};

  const double link_capacity = 2000.0;
  CandidateData cand1;
  cand1.link_id = link_id;
  cand1.name = "candy1";
  cand1.link_name = "dummy_link";
  cand1.already_installed_capacity = link_capacity;
  cand1.installed_direct_link_profile_name = "install_link_profile";
  cand1.direct_link_profile = "profile_cand1";
  cand1.indirect_link_profile = "profile_cand1";
  CandidateData cand2;
  cand2.link_id = link_id;
  cand2.name = "candy2";
  cand2.link_name = "dummy_link";
  cand2.already_installed_capacity = link_capacity;
  cand2.installed_direct_link_profile_name = "install_link_profile";
  cand2.direct_link_profile = "empty_profile";
  cand2.indirect_link_profile = "empty_profile";

  std::vector<CandidateData> cand_data_list = {cand1, cand2};
  std::map<std::string, std::vector<LinkProfile>> profile_map;
  LinkProfile profile_link(logger);
  profile_link.direct_link_profile = {1, 2};
  profile_link.indirect_link_profile = {3, 4};
  profile_map["install_link_profile"] = {profile_link};
  LinkProfile profile_cand1(logger);
  profile_cand1.direct_link_profile = {0.5, 1};
  profile_cand1.indirect_link_profile = {0.8, 1.2};
  profile_map["profile_cand1"] = {profile_cand1};
  LinkProfile empty_profile(logger);
  empty_profile.direct_link_profile = {0.0, 0.0};
  empty_profile.indirect_link_profile = {0.0, 0.0};
  profile_map["empty_profile"] = {empty_profile};

  ActiveLinksBuilder linkBuilder{cand_data_list, profile_map, logger};
  const std::vector<ActiveLink>& links = linkBuilder.getLinks();

  auto problem_modifier = ProblemModifier(logger);
  problem_modifier.changeProblem(math_problem.get(), links, p_var_columns,
                                 p_direct_cost_columns,
                                 p_indirect_cost_columns);

  const int P_LINK_id = 0;
  const int P_PLUS_id = 1;
  const int P_MINUS_id = 2;
  const int cand1_id = 3;

  verify_column(P_LINK_id, P_LINK, 'C', 1, MINUS_INF, PLUS_INF);
  verify_column(P_PLUS_id, P_PLUS, 'C', 1, 0, PLUS_INF);
  verify_column(P_MINUS_id, P_MINUS, 'C', 1, 0, PLUS_INF);
  verify_column(cand1_id, cand1.name, 'C', 0, MINUS_INF, PLUS_INF);
  ASSERT_EQ(problem_modifier.get_candidate_col_id(cand1.name), cand1_id);
  EXPECT_FALSE(problem_modifier.has_candidate_col_id(cand2.name));

  verify_rows_are(8);

  verify_row(0, 'L', {1, -profile_cand1.getDirectProfile(0)},
             {P_LINK_id, cand1_id},
             link_capacity * profile_link.getDirectProfile(0));

  verify_row(1, 'G', {1, profile_cand1.getIndirectProfile(0)},
             {P_LINK_id, cand1_id},
             -link_capacity * profile_link.getIndirectProfile(0));

  verify_row(2, 'L', {1, -profile_cand1.getDirectProfile(1)},
             {P_LINK_id, cand1_id},
             link_capacity * profile_link.getDirectProfile(1));

  verify_row(3, 'G', {1, profile_cand1.getIndirectProfile(1)},
             {P_LINK_id, cand1_id},
             -link_capacity * profile_link.getIndirectProfile(1));

  verify_row(4, 'L', {1, -profile_cand1.getDirectProfile(0)},
             {P_PLUS_id, cand1_id},
             link_capacity * profile_link.getDirectProfile(0));

  verify_row(5, 'L', {1, -profile_cand1.getDirectProfile(1)},
             {P_PLUS_id, cand1_id},
             link_capacity * profile_link.getDirectProfile(1));

  verify_row(6, 'L', {1, -profile_cand1.getIndirectProfile(0)},
             {P_MINUS_id, cand1_id},
             link_capacity * profile_link.getIndirectProfile(0));

  verify_row(7, 'L', {1, -profile_cand1.getIndirectProfile(1)},
             {P_MINUS_id, cand1_id},
             link_capacity * profile_link.getIndirectProfile(1));
}

TEST_F(ProblemModifierTest, candidateWithNotNullProfileExists) {
  const int link_id = 0;
  const std::map<linkId, ColumnsToChange> p_var_columns = {
      {link_id, {{0, 0}, {0, 1}}}};
  const std::map<linkId, ColumnsToChange> p_direct_cost_columns = {
      {link_id, {{1, 0}, {1, 1}}}};
  const std::map<linkId, ColumnsToChange> p_indirect_cost_columns = {
      {link_id, {{{2, 0}, {2, 1}}}}};

  const double link_capacity = 2000.0;
  CandidateData cand1;
  cand1.link_id = link_id;
  cand1.name = "candy1";
  cand1.link_name = "dummy_link";
  cand1.already_installed_capacity = link_capacity;
  cand1.installed_direct_link_profile_name = "install_link_profile";
  cand1.direct_link_profile = "profile_cand1";
  cand1.indirect_link_profile = "profile_cand1";

  std::vector<CandidateData> cand_data_list = {cand1};
  std::map<std::string, std::vector<LinkProfile>> profile_map;
  LinkProfile profile_link(logger);
  profile_link.direct_link_profile = {1, 2};
  profile_link.indirect_link_profile = {3, 4};
  profile_map["install_link_profile"] = {profile_link};
  LinkProfile profile_cand1(logger);
  profile_cand1.direct_link_profile = {0.5, 1};
  profile_cand1.indirect_link_profile = {0.8, 1.2};
  profile_map["profile_cand1"] = {profile_cand1};

  ActiveLinksBuilder linkBuilder{cand_data_list, profile_map, logger};
  const std::vector<ActiveLink>& links = linkBuilder.getLinks();

  auto problem_modifier = ProblemModifier(logger);
  problem_modifier.changeProblem(math_problem.get(), links, p_var_columns,
                                 p_direct_cost_columns,
                                 p_indirect_cost_columns);

  const int cand1_id = 3;

  ASSERT_EQ(problem_modifier.get_candidate_col_id(cand1.name), cand1_id);
}
TEST_F(ProblemModifierTest, candidateWithNullProfileIsRemoved) {
  const int link_id = 0;
  const std::map<linkId, ColumnsToChange> p_var_columns = {
      {link_id, {{0, 0}, {0, 1}}}};
  const std::map<linkId, ColumnsToChange> p_direct_cost_columns = {
      {link_id, {{1, 0}, {1, 1}}}};
  const std::map<linkId, ColumnsToChange> p_indirect_cost_columns = {
      {link_id, {{{2, 0}, {2, 1}}}}};

  const double link_capacity = 2000.0;
  CandidateData cand1;
  cand1.link_id = link_id;
  cand1.name = "candy1";
  cand1.link_name = "dummy_link";
  cand1.already_installed_capacity = link_capacity;
  cand1.installed_direct_link_profile_name = "install_link_profile";
  cand1.direct_link_profile = "profile_cand1";
  cand1.indirect_link_profile = "profile_cand1";

  std::vector<CandidateData> cand_data_list = {cand1};
  std::map<std::string, std::vector<LinkProfile>> profile_map;
  LinkProfile profile_link(logger);
  profile_link.direct_link_profile = {0, 0};
  profile_link.indirect_link_profile = {0, 0};
  profile_map["install_link_profile"] = {profile_link};
  LinkProfile profile_cand1(logger);
  profile_cand1.direct_link_profile = {0, 0};
  profile_cand1.indirect_link_profile = {0, 0};
  profile_map["profile_cand1"] = {profile_cand1};

  ActiveLinksBuilder linkBuilder{cand_data_list, profile_map, logger};
  const std::vector<ActiveLink>& links = linkBuilder.getLinks();

  auto problem_modifier = ProblemModifier(logger);
  problem_modifier.changeProblem(math_problem.get(), links, p_var_columns,
                                 p_direct_cost_columns,
                                 p_indirect_cost_columns);

  const int cand1_id = -1;

  ASSERT_THROW(problem_modifier.get_candidate_col_id(cand1.name),
               std::runtime_error);
}

class ProblemModifierTestMultiChronicle : public ProblemModifierTest {
 protected:
  void SetUp() override {
    ProblemModifierTest::SetUp();

    cand1.link_id = link_id;
    cand1.name = "candy1";
    cand1.link_name = "dummy_link";
    cand1.already_installed_capacity = link_capacity;
    cand1.installed_direct_link_profile_name = "install_link_profile";
    cand1.direct_link_profile = "profile_cand1";
    cand1.indirect_link_profile = "profile_cand1";
    cand1.linkor = "A";
    cand1.linkex = "B";

    cand2.link_id = link_id;
    cand2.name = "candy2";
    cand2.link_name = "dummy_link";
    cand2.already_installed_capacity = link_capacity;
    cand2.installed_direct_link_profile_name = "install_link_profile";
    cand2.direct_link_profile = "profile_cand2";
    cand2.indirect_link_profile = "profile_cand2";
    cand2.linkor = "C";
    cand2.linkex = "D";

    std::vector<CandidateData> cand_data_list = {cand1, cand2};
    std::map<std::string, std::vector<LinkProfile>> profile_map;
    profile_link.direct_link_profile = {1, 2};
    profile_link.indirect_link_profile = {3, 4};
    profile_map["install_link_profile"] = {profile_link};
    profile_cand1.direct_link_profile = {0.5, 1};
    profile_cand1.indirect_link_profile = {0.8, 1.2};
    profile_cand1_2.direct_link_profile = {5, 10};
    profile_cand1_2.indirect_link_profile = {8, 12};
    profile_map["profile_cand1"] = {profile_cand1, profile_cand1_2};
    profile_cand2.direct_link_profile = {1.5, 1.7};
    profile_cand2.indirect_link_profile = {2.6, 2.8};
    profile_cand2_2.direct_link_profile = {15, 17};
    profile_cand2_2.indirect_link_profile = {26, 28};
    profile_map["profile_cand2"] = {profile_cand2, profile_cand2_2};

    std::filesystem::path ts_info_root_ =
        std::filesystem::temp_directory_path() / std::tmpnam(nullptr);
    std::filesystem::create_directories(ts_info_root_ / "A");
    std::ofstream b_file(ts_info_root_ / "A" / "B.txt");
    b_file << "Garbage\n1\n2\n";  // Use link profile 1 for MCY1 and link
                                  // profile 2 for MCY2
    b_file.close();

    std::ofstream D_file(ts_info_root_ / "C" / "D.txt");
    D_file << "Garbage\n1\n1\n";
    D_file.close();

    ActiveLinksBuilder linkBuilder{
        cand_data_list, profile_map,
        DirectAccessScenarioToChronicleProvider(ts_info_root_, logger), logger};
    links = linkBuilder.getLinks();
  }

 protected:
  std::vector<ActiveLink> links;
  const unsigned int link_id = 0;
  const std::map<linkId, ColumnsToChange> p_var_columns = {
      {link_id, {{0, 0}, {0, 1}}}};
  const std::map<linkId, ColumnsToChange> p_direct_cost_columns = {
      {link_id, {{1, 0}, {1, 1}}}};
  const std::map<linkId, ColumnsToChange> p_indirect_cost_columns = {
      {link_id, {{{2, 0}, {2, 1}}}}};
  CandidateData cand1, cand2;
  const double link_capacity = 2000.0;
  LinkProfile profile_link = LinkProfile(logger),
              profile_cand1 = LinkProfile(logger),
              profile_cand1_2 = LinkProfile(logger),
              profile_cand2 = LinkProfile(logger),
              profile_cand2_2 = LinkProfile(logger);
};

TEST_F(
    ProblemModifierTestMultiChronicle,
    One_link_two_candidates_two_timestep_profile_multiple_chronicle_chooseNothing) {
  auto problem_modifier = ProblemModifier(logger);
  problem_modifier.changeProblem(math_problem.get(), links, p_var_columns,
                                 p_direct_cost_columns,
                                 p_indirect_cost_columns);

  const int P_LINK_id = 0;
  const int P_PLUS_id = 1;
  const int P_MINUS_id = 2;
  const int cand1_id = 3;
  const int cand2_id = 4;

  verify_column(P_LINK_id, P_LINK, 'C', 1, MINUS_INF, PLUS_INF);
  verify_column(P_PLUS_id, P_PLUS, 'C', 1, 0, PLUS_INF);
  verify_column(P_MINUS_id, P_MINUS, 'C', 1, 0, PLUS_INF);
  verify_column(cand1_id, cand1.name, 'C', 0, MINUS_INF, PLUS_INF);
  verify_column(cand2_id, cand2.name, 'C', 0, MINUS_INF, PLUS_INF);
  ASSERT_EQ(problem_modifier.get_candidate_col_id(cand1.name), cand1_id);
  ASSERT_EQ(problem_modifier.get_candidate_col_id(cand2.name), cand2_id);

  verify_rows_are(8);

  verify_row(0, 'L',
             {1, -profile_cand1.getDirectProfile(0),
              -profile_cand2.getDirectProfile(0)},
             {P_LINK_id, cand1_id, cand2_id},
             link_capacity * profile_link.getDirectProfile(0));

  verify_row(1, 'G',
             {1, profile_cand1.getIndirectProfile(0),
              profile_cand2.getIndirectProfile(0)},
             {P_LINK_id, cand1_id, cand2_id},
             -link_capacity * profile_link.getIndirectProfile(0));

  verify_row(2, 'L',
             {1, -profile_cand1.getDirectProfile(1),
              -profile_cand2.getDirectProfile(1)},
             {P_LINK_id, cand1_id, cand2_id},
             link_capacity * profile_link.getDirectProfile(1));

  verify_row(3, 'G',
             {1, profile_cand1.getIndirectProfile(1),
              profile_cand2.getIndirectProfile(1)},
             {P_LINK_id, cand1_id, cand2_id},
             -link_capacity * profile_link.getIndirectProfile(1));

  verify_row(4, 'L',
             {1, -profile_cand1.getDirectProfile(0),
              -profile_cand2.getDirectProfile(0)},
             {P_PLUS_id, cand1_id, cand2_id},
             link_capacity * profile_link.getDirectProfile(0));

  verify_row(5, 'L',
             {1, -profile_cand1.getDirectProfile(1),
              -profile_cand2.getDirectProfile(1)},
             {P_PLUS_id, cand1_id, cand2_id},
             link_capacity * profile_link.getDirectProfile(1));

  verify_row(6, 'L',
             {1, -profile_cand1.getIndirectProfile(0),
              -profile_cand2.getIndirectProfile(0)},
             {P_MINUS_id, cand1_id, cand2_id},
             link_capacity * profile_link.getIndirectProfile(0));

  verify_row(7, 'L',
             {1, -profile_cand1.getIndirectProfile(1),
              -profile_cand2.getIndirectProfile(1)},
             {P_MINUS_id, cand1_id, cand2_id},
             link_capacity * profile_link.getIndirectProfile(1));
}
TEST_F(
    ProblemModifierTestMultiChronicle,
    One_link_two_candidates_two_timestep_profile_multiple_chronicle_chooseChronicle2forYear2) {
  auto problem_modifier = ProblemModifier(logger);
  math_problem->mc_year = 2;
  problem_modifier.changeProblem(math_problem.get(), links, p_var_columns,
                                 p_direct_cost_columns,
                                 p_indirect_cost_columns);

  const int P_LINK_id = 0;
  const int P_PLUS_id = 1;
  const int P_MINUS_id = 2;
  const int cand1_id = 3;
  const int cand2_id = 4;

  verify_column(P_LINK_id, P_LINK, 'C', 1, MINUS_INF, PLUS_INF);
  verify_column(P_PLUS_id, P_PLUS, 'C', 1, 0, PLUS_INF);
  verify_column(P_MINUS_id, P_MINUS, 'C', 1, 0, PLUS_INF);
  verify_column(cand1_id, cand1.name, 'C', 0, MINUS_INF, PLUS_INF);
  verify_column(cand2_id, cand2.name, 'C', 0, MINUS_INF, PLUS_INF);
  ASSERT_EQ(problem_modifier.get_candidate_col_id(cand1.name), cand1_id);
  ASSERT_EQ(problem_modifier.get_candidate_col_id(cand2.name), cand2_id);

  verify_rows_are(8);

  verify_row(0, 'L',
             {1, -profile_cand1_2.getDirectProfile(0),
              -profile_cand2_2.getDirectProfile(0)},
             {P_LINK_id, cand1_id, cand2_id},
             link_capacity * profile_link.getDirectProfile(0));

  verify_row(1, 'G',
             {1, profile_cand1_2.getIndirectProfile(0),
              profile_cand2_2.getIndirectProfile(0)},
             {P_LINK_id, cand1_id, cand2_id},
             -link_capacity * profile_link.getIndirectProfile(0));

  verify_row(2, 'L',
             {1, -profile_cand1_2.getDirectProfile(1),
              -profile_cand2_2.getDirectProfile(1)},
             {P_LINK_id, cand1_id, cand2_id},
             link_capacity * profile_link.getDirectProfile(1));

  verify_row(3, 'G',
             {1, profile_cand1_2.getIndirectProfile(1),
              profile_cand2_2.getIndirectProfile(1)},
             {P_LINK_id, cand1_id, cand2_id},
             -link_capacity * profile_link.getIndirectProfile(1));

  verify_row(4, 'L',
             {1, -profile_cand1_2.getDirectProfile(0),
              -profile_cand2_2.getDirectProfile(0)},
             {P_PLUS_id, cand1_id, cand2_id},
             link_capacity * profile_link.getDirectProfile(0));

  verify_row(5, 'L',
             {1, -profile_cand1_2.getDirectProfile(1),
              -profile_cand2_2.getDirectProfile(1)},
             {P_PLUS_id, cand1_id, cand2_id},
             link_capacity * profile_link.getDirectProfile(1));

  verify_row(6, 'L',
             {1, -profile_cand1_2.getIndirectProfile(0),
              -profile_cand2_2.getIndirectProfile(0)},
             {P_MINUS_id, cand1_id, cand2_id},
             link_capacity * profile_link.getIndirectProfile(0));

  verify_row(7, 'L',
             {1, -profile_cand1_2.getIndirectProfile(1),
              -profile_cand2_2.getIndirectProfile(1)},
             {P_MINUS_id, cand1_id, cand2_id},
             link_capacity * profile_link.getIndirectProfile(1));
}
TEST_F(ProblemModifierTestMultiChronicle, candidateWithNotNullProfileExists) {
  auto problem_modifier = ProblemModifier(logger);
  math_problem->mc_year = 2;
  problem_modifier.changeProblem(math_problem.get(), links, p_var_columns,
                                 p_direct_cost_columns,
                                 p_indirect_cost_columns);

  const int cand1_id = 3;

  ASSERT_TRUE(problem_modifier.has_candidate_col_id(cand1.name));
}

class ProblemModifierTestWithProfile : public ProblemModifierTest {
 protected:
  void SetUp() override {
    ProblemModifierTest::SetUp();

    const double link_capacity = 2000.0;

    cand1.link_id = link_id;
    cand1.name = "candidate";
    cand1.link_name = "dummy_link";
    cand1.already_installed_capacity = link_capacity;
    cand1.direct_link_profile = "chronicle_1";
    cand1.indirect_link_profile = "chronicle_1";
    cand1.linkor = "A";
    cand1.linkex = "B";

    ActiveLinksBuilder linkBuilder{
        cand_data_list, profile_map,
        DirectAccessScenarioToChronicleProvider(ts_info_root_, logger), logger};
    links = linkBuilder.getLinks();

    math_problem->mc_year = 2;
  }

  ProblemModifier problem_modifier = ProblemModifier(logger);
  std::vector<ActiveLink> links;
  CandidateData cand1;

  const unsigned int link_id = 0;
  const std::map<linkId, ColumnsToChange> p_var_columns = {
      {link_id, {{0, 0}, {0, 1}}}};
  const std::map<linkId, ColumnsToChange> p_direct_cost_columns = {
      {link_id, {{1, 0}, {1, 1}}}};
  const std::map<linkId, ColumnsToChange> p_indirect_cost_columns = {
      {link_id, {{{2, 0}, {2, 1}}}}};

  LinkProfile installed_profile = LinkProfile(logger),
              chronicle_1 = LinkProfile(logger),
              chronicle_2 = LinkProfile(logger);
  std::map<std::string, std::vector<LinkProfile>> profile_map;
  std::vector<CandidateData> cand_data_list = {cand1};
  std::filesystem::path ts_info_root_ =
      std::filesystem::temp_directory_path() / std::tmpnam(nullptr);
};

TEST_F(ProblemModifierTestWithProfile,
       candidateWithNullProfileIsRemovedMultipleChronicle) {
  cand1.installed_direct_link_profile_name = "install_link_profile";
  installed_profile.direct_link_profile = {0, 0};
  installed_profile.indirect_link_profile = {0, 0};
  profile_map["install_link_profile"] = {installed_profile};
  chronicle_1.direct_link_profile = {0, 0};
  chronicle_1.indirect_link_profile = {0, 0};
  chronicle_2.direct_link_profile = {0, 0};
  chronicle_2.indirect_link_profile = {0, 0};
  profile_map["chronicle_1"] = {chronicle_1, chronicle_2};

  std::filesystem::create_directories(ts_info_root_ / "A");
  std::ofstream b_file(ts_info_root_ / "A" / "B.txt");
  b_file << "Garbage\n1\n2\n";  // Use link profile 1 for MCY1 and link profile
                                // 2 for MCY2
  b_file.close();

  cand_data_list = {cand1};
  ActiveLinksBuilder linkBuilder{
      cand_data_list, profile_map,
      DirectAccessScenarioToChronicleProvider(ts_info_root_, logger), logger};
  links = linkBuilder.getLinks();

  problem_modifier.changeProblem(math_problem.get(), links, p_var_columns,
                                 p_direct_cost_columns,
                                 p_indirect_cost_columns);

  ASSERT_THROW(problem_modifier.get_candidate_col_id(cand1.name),
               std::runtime_error);
}
TEST_F(ProblemModifierTestWithProfile,
       CandidateWithNotNullChronicleButOTherwiseNullProfileExists) {
  std::map<std::string, std::vector<LinkProfile>> profile_map;
  chronicle_1.direct_link_profile = {0, 0};
  chronicle_1.indirect_link_profile = {0, 0};
  chronicle_2.direct_link_profile = {0, 1};
  chronicle_2.indirect_link_profile = {0, 1};
  profile_map["chronicle_1"] = {chronicle_1, chronicle_2};

  std::filesystem::path ts_info_root_ =
      std::filesystem::temp_directory_path() / std::tmpnam(nullptr);
  std::filesystem::create_directories(ts_info_root_ / "A");
  std::ofstream b_file(ts_info_root_ / "A" / "B.txt");
  b_file << "Garbage\n1\n2\n";  // Use link profile 1 for MCY1 and link profile
                                // 2 for MCY2
  b_file.close();

  cand_data_list = {cand1};
  ActiveLinksBuilder linkBuilder{
      cand_data_list, profile_map,
      DirectAccessScenarioToChronicleProvider(ts_info_root_, logger), logger};
  links = linkBuilder.getLinks();

  problem_modifier.changeProblem(math_problem.get(), links, p_var_columns,
                                 p_direct_cost_columns,
                                 p_indirect_cost_columns);

  ASSERT_TRUE(problem_modifier.has_candidate_col_id(cand1.name));
}
TEST_F(ProblemModifierTestWithProfile,
       CandidatesWithNullSelectedChronicleIsRemoved) {
  cand_data_list = {cand1};
  std::map<std::string, std::vector<LinkProfile>> profile_map;
  chronicle_1.direct_link_profile = {1, 1};
  chronicle_1.indirect_link_profile = {1, 1};
  chronicle_2.direct_link_profile = {0, 0};
  chronicle_2.indirect_link_profile = {0, 0};
  profile_map["chronicle_1"] = {chronicle_1, chronicle_2};

  std::filesystem::path ts_info_root_ =
      std::filesystem::temp_directory_path() / std::tmpnam(nullptr);
  std::filesystem::create_directories(ts_info_root_ / "A");
  std::ofstream b_file(ts_info_root_ / "A" / "B.txt");
  b_file << "Garbage\n2\n2\n";  // Use chronicle 2. Chronicle 1 is never used.
                                // Chronicle 2 is empty
  b_file.close();

  ActiveLinksBuilder linkBuilder{
      cand_data_list, profile_map,
      DirectAccessScenarioToChronicleProvider(ts_info_root_, logger), logger};
  links = linkBuilder.getLinks();

  problem_modifier.changeProblem(math_problem.get(), links, p_var_columns,
                                 p_direct_cost_columns,
                                 p_indirect_cost_columns);

  ASSERT_THROW(problem_modifier.get_candidate_col_id(cand1.name),
               std::runtime_error);
}