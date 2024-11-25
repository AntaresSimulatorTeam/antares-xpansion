
#include "antares-xpansion/benders/benders_core/CouplingMapGenerator.h"
#include "antares-xpansion/benders/benders_core/CriterionInputDataReader.h"
#include "antares-xpansion/benders/benders_core/MasterUpdate.h"
#include "antares-xpansion/benders/benders_core/VariablesGroup.h"
#include "antares-xpansion/benders/benders_mpi/OuterLoopBenders.h"
#include "antares-xpansion/benders/factories/LoggerFactories.h"
#include "antares-xpansion/benders/factories/WriterFactories.h"
#include "antares-xpansion/benders/outer_loop/OuterLoopBiLevel.h"
#include "antares-xpansion/multisolver_interface/environment.h"
#include "gtest/gtest.h"

boost::mpi::environment* penv = nullptr;
boost::mpi::communicator* pworld = nullptr;

using namespace Outerloop;
using namespace Benders::Criterion;

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  mpi::environment env(argc, argv);
  mpi::communicator world;
  penv = &env;
  pworld = &world;

  return RUN_ALL_TESTS();
}

//-------------------- MasterUpdateBaseTest -------------------------
const auto STUDY_PATH =
    std::filesystem::path("data_test") / "external_loop_test";
const auto LP_DIR = std::filesystem::absolute((STUDY_PATH / "lp"));
const auto OPTIONS_FILE = LP_DIR / "options.json";
const auto OUTER_OPTIONS_FILE = "adequacy_criterion.yml";

class MasterUpdateBaseTest : public ::testing::TestWithParam<std::string> {
 public:
  pBendersBase benders;
  std::shared_ptr<MathLoggerDriver> math_log_driver;
  Logger logger;
  Writer writer;

  void SetUp() override {
    // Save the current working directory
    original_working_dir_ = std::filesystem::current_path();

    // Change to the desired working directory
    std::filesystem::current_path(LP_DIR);
    math_log_driver = MathLoggerFactory::get_void_logger();
    logger = build_void_logger();
    writer = build_void_writer();
  }
  BendersBaseOptions BuildBendersOptions() {
    SimulationOptions options(OPTIONS_FILE);
    return options.get_benders_options();
  }

  void TearDown() override {
    // Restore the original working directory after the test
    std::filesystem::current_path(original_working_dir_);
  }
  std::filesystem::path original_working_dir_;
};

auto solvers() {
  std::vector<std::string> solvers_name;
  solvers_name.push_back("COIN");
  if (LoadXpress::XpressLoader xpressLoader;
      xpressLoader.XpressIsCorrectlyInstalled()) {
    solvers_name.push_back("XPRESS");
  }
  return solvers_name;
}

INSTANTIATE_TEST_SUITE_P(availsolvers, MasterUpdateBaseTest,
                         ::testing::ValuesIn(solvers()));
double LambdaMax(pBendersBase benders) {
  const auto& obj = benders->MasterObjectiveFunctionCoeffs();
  const auto max_invest = benders->BestIterationWorkerMaster().get_max_invest();
  double lambda_max = 0;
  for (const auto& [var_name, var_id] : benders->MasterVariables()) {
    lambda_max += obj[var_id] * max_invest.at(var_name);
  }
  return lambda_max;
}

void CheckMinInvestmentConstraint(const VariableMap& master_variables,
                                  const std::vector<double>& expected_coeffs,
                                  const double expected_rhs, char expected_sign,
                                  const std::vector<double>& coeffs,
                                  const double rhs, char sign) {
  for (auto const& [name, var_id] : master_variables) {
    ASSERT_EQ(expected_coeffs[var_id], coeffs[var_id]);
  }

  ASSERT_EQ(expected_rhs, rhs);
  ASSERT_EQ(expected_sign, sign);
}



TEST_P(MasterUpdateBaseTest, ConstraintIsAddedBendersMPI) {
  BendersBaseOptions bendersoptions = BuildBendersOptions();
  CouplingMap coupling_map = CouplingMapGenerator::BuildInput(
      std::filesystem::path(bendersoptions.INPUTROOT) /
          bendersoptions.STRUCTURE_FILE,
      logger, ::testing::UnitTest::GetInstance()->current_test_info()->name());
  // override solver
  bendersoptions.SOLVER_NAME = GetParam();
  bendersoptions.EXTERNAL_LOOP_OPTIONS.DO_OUTER_LOOP = true;
  bendersoptions.EXTERNAL_LOOP_OPTIONS.OUTER_LOOP_OPTION_FILE =
      OUTER_OPTIONS_FILE;
  benders = std::make_shared<BendersMpi>(bendersoptions, logger, writer, *penv,
                                         *pworld, math_log_driver);
  benders->set_input_map(coupling_map);

  auto outer_loop_input_data =
      Benders::Criterion::CriterionInputFromYaml().Read(
          std::filesystem::path(bendersoptions.INPUTROOT) / OUTER_OPTIONS_FILE);

  benders->setCriterionComputationInputs(outer_loop_input_data);

  auto master_updater = std::make_shared<MasterUpdateBase>(
      benders, 0.5, outer_loop_input_data.StoppingThreshold());
  auto cut_manager = std::make_shared<Outerloop::CutsManagerRunTime>();
  Outerloop::OuterLoopBenders out_loop(outer_loop_input_data.Criteria(),
                                       master_updater, cut_manager, benders, *pworld);
  out_loop.OuterLoopCheckFeasibility();

  auto num_constraints_master_before = benders->MasterGetnrows();
  auto lambda_min = out_loop.OuterLoopLambdaMin();
  auto lambda_max = out_loop.OuterLoopLambdaMax();
  auto expected_lambda_max = LambdaMax(benders);
  //--------
  ASSERT_EQ(lambda_max, expected_lambda_max);
  //--------

  master_updater->Update(lambda_min, lambda_max);
  auto num_constraints_master_after = benders->MasterGetnrows();

  //------
  ASSERT_EQ(num_constraints_master_after, num_constraints_master_before + 1);
  //------

  auto master_variables = benders->MasterVariables();
  auto expected_coeffs = benders->MasterObjectiveFunctionCoeffs();

  // criterion is low <=> lambda_max = min(lambda_max, invest_cost)
  auto expected_rhs = 0.5 * lambda_max;

  // get added constraint infos (coeff, sign & rhs)
  std::vector<int> mstart(1 + 1);
  auto n_elems = benders->MasterGetNElems();
  auto nnz = master_variables.size();
  std::vector<int> mclind(n_elems);
  std::vector<double> matval(n_elems);
  std::vector<int> p_nels(1, 0);

  auto added_row_index = num_constraints_master_after - 1;
  benders->MasterRowsCoeffs(mstart, mclind, matval, n_elems, p_nels,
                            added_row_index, added_row_index);
  std::vector<double> coeffs(benders->MasterGetncols());

  for (auto ind = mstart[0]; ind < mstart[1]; ++ind) {
    coeffs[mclind[ind]] = matval[ind];
  }
  double rhs;
  benders->MasterGetRhs(rhs, added_row_index);
  std::vector<char> qrtype(1);
  benders->MasterGetRowType(qrtype, added_row_index, added_row_index);
  CheckMinInvestmentConstraint(master_variables, expected_coeffs, expected_rhs,
                               'G', coeffs, rhs, qrtype[0]);
  benders->free();
}

class OuterLoopPatternTest : public ::testing::Test {};

TEST_F(OuterLoopPatternTest, RegexGivenPrefixAndBody) {
  const std::string prefix = "prefix";
  const std::string body = "body";
  CriterionPattern o(prefix, body);

  auto ret_regex = o.MakeRegex();

  ASSERT_EQ(std::regex_search(prefix + body, ret_regex), false);
  ASSERT_EQ(std::regex_search(prefix + "::" + body + "::suffix", ret_regex),
            false);
  ASSERT_EQ(std::regex_search(body + prefix, ret_regex), false);
  ASSERT_EQ(std::regex_search(prefix + "::", ret_regex), false);
  ASSERT_EQ(std::regex_search(body, ret_regex), false);
  ASSERT_EQ(std::regex_search(prefix + "area<" + body + ">", ret_regex), true);
  ASSERT_EQ(std::regex_search(prefix + "area<" + body + ">::suffix", ret_regex), true);
  ASSERT_EQ(std::regex_search(prefix + "area<" + body + "_other_area>::suffix", ret_regex), false);
}

class OuterLoopInputFromYamlTest : public ::testing::Test {};

TEST_F(OuterLoopInputFromYamlTest, YamlFileDoesNotExist) {
  std::filesystem::path empty("");
  std::ostringstream expected_msg;
  expected_msg << "Could not read outer loop input file: " << empty << "\n"
               << "bad file: " << empty.string();
  try {
    CriterionInputFromYaml parser;
    parser.Read(empty);
  } catch (const CriterionInputFileError& e) {
    ASSERT_EQ(expected_msg.str(), e.ErrorMessage());
  }
}

TEST_F(OuterLoopInputFromYamlTest, YamlFileIsEmpty) {
  std::filesystem::path empty(std::filesystem::temp_directory_path() /
                              "empty.yml");
  std::ofstream of(empty);
  of.close();

  std::ostringstream expected_msg;
  expected_msg << "outer loop input file is empty: " << empty << "\n";
  try {
    CriterionInputFromYaml parser;
    parser.Read(empty);
  } catch (const CriterionInputFileIsEmpty& e) {
    ASSERT_EQ(expected_msg.str(), e.ErrorMessage());
  }
}

TEST_F(OuterLoopInputFromYamlTest, YamlFileShouldContainsAtLeast1Pattern) {
  std::filesystem::path empty_patterns(std::filesystem::temp_directory_path() /
                                       "empty_patterns.yml");
  std::ofstream of(empty_patterns);
  of << "stopping_threshold: " << 17.07;
  of.close();

  std::ostringstream expected_msg;
  expected_msg << "outer loop input file must contains at least one pattern."
               << "\n";
  try {
    CriterionInputFromYaml parser;
    parser.Read(empty_patterns);
  } catch (const CriterionInputFileNoPatternFound& e) {
    ASSERT_EQ(expected_msg.str(), e.ErrorMessage());
  }
}

TEST_F(OuterLoopInputFromYamlTest, YamlFilePatternsShouldBeAnArray) {
  std::filesystem::path patterns_not_array(
      std::filesystem::temp_directory_path() / "patterns_not_array.yml");
  std::ofstream of(patterns_not_array);
  of << "stopping_threshold: " << 17.07 << "\n"
     << "patterns: " << 786;
  of.close();

  std::ostringstream expected_msg;
  expected_msg << "In outer loop input file 'patterns' should be an array."
               << "\n";
  try {
    CriterionInputFromYaml parser;
    parser.Read(patterns_not_array);
  } catch (const CriterionInputPatternsShouldBeArray& e) {
    ASSERT_EQ(expected_msg.str(), e.ErrorMessage());
  }
}

TEST_F(OuterLoopInputFromYamlTest, ReadValidFile) {
  std::filesystem::path valid_file(std::filesystem::temp_directory_path() /
                                   "valid_file.yml");

  std::ofstream of(valid_file);
  auto my_yaml = R"(stopping_threshold: 1e-4
# seuil
criterion_count_threshold: 1e-1
patterns:
  - area: "N0"
    criterion: 185
  - area: "N1"
    criterion: 1
  - area: "N2"
    criterion: 1
  - area: "N3"
    criterion: 1)";
  of << my_yaml;
  of.close();
  auto data = CriterionInputFromYaml().Read(valid_file);

  ASSERT_EQ(data.StoppingThreshold(), 1e-4);
  ASSERT_EQ(data.CriterionCountThreshold(), 1e-1);

  auto patterns = data.Criteria();
  ASSERT_EQ(patterns.size(), 4);
  auto pattern1 = patterns[0];
  ASSERT_EQ(pattern1.Criterion(), 185.0);
  ASSERT_EQ(pattern1.Pattern().GetBody(), "N0");
}

class VariablesGroupTest : public ::testing::Test {};

TEST_F(VariablesGroupTest, EmptyVariablesListGivesEmptyIndices) {
  std::vector<std::string> variables;
  std::vector<Benders::Criterion::CriterionSingleInputData> data;

  Benders::Criterion::VariablesGroup var_grp(variables, data);
  ASSERT_TRUE(var_grp.Indices().empty());
}

TEST_F(VariablesGroupTest, EmptyPatternsListGivesEmptyIndices) {
  std::vector<std::string> variables{
      "PositiveUnsuppliedEnergy::area<test>::hour<125>"};
  std::vector<Benders::Criterion::CriterionSingleInputData> data;

  Benders::Criterion::VariablesGroup var_grp(variables, data);
  ASSERT_TRUE(var_grp.Indices().empty());
}

TEST_F(VariablesGroupTest, SingleDataWithInvalidPrefixAndBody) {
  std::vector<std::string> variables{
      "PositiveUnsuppliedEnergy::area<test>::hour<125>"};
  std::vector<Benders::Criterion::CriterionSingleInputData> data{
      Benders::Criterion::CriterionSingleInputData("Pref", "Body", 1534.0)};

  Benders::Criterion::VariablesGroup var_grp(variables, data);
  const auto& vect_indices = var_grp.Indices();
  ASSERT_EQ(vect_indices.size(), 1);
  ASSERT_TRUE(vect_indices[0].empty());
}

TEST_F(VariablesGroupTest, SingleDataWithUnMatchedPrefix) {
  std::vector<std::string> variables{
      "PositiveUnsuppliedEnergy::area<test>::hour<125>"};
  std::vector<Benders::Criterion::CriterionSingleInputData> data{
      Benders::Criterion::CriterionSingleInputData("UnsuppliedEnergy::", "test",
                                                   1534.0)};

  Benders::Criterion::VariablesGroup var_grp(variables, data);
  const auto& vect_indices = var_grp.Indices();
  ASSERT_EQ(vect_indices.size(), 1);
  ASSERT_TRUE(vect_indices[0].empty());
}

TEST_F(VariablesGroupTest, SingleDataWithUnMatchedBody) {
  std::vector<std::string> variables{
      "PositiveUnsuppliedEnergy::area<test>::hour<125>"};
  std::vector<Benders::Criterion::CriterionSingleInputData> data{
      Benders::Criterion::CriterionSingleInputData(
          "PositiveUnsuppliedEnergy::", "Body", 1534.0)};

  Benders::Criterion::VariablesGroup var_grp(variables, data);
  const auto& vect_indices = var_grp.Indices();
  ASSERT_EQ(vect_indices.size(), 1);
  ASSERT_TRUE(vect_indices[0].empty());
}

static const std::vector<Benders::Criterion::CriterionSingleInputData> data{
    {"Blue::", "Earth", 1534.0}, {"Red::", "Mars", 65.0}};

TEST_F(VariablesGroupTest, With2ValidPatterns) {
  std::vector<std::string> variables{
      "Gold::area<Sun>::hour<9999>", "Blue::area<Earth>::hour<125>",
      "Red::area<Mars>::hour<1546>", "Blue::area<Earth>::hour<3336>"};

  Benders::Criterion::VariablesGroup var_grp(variables, data);
  const auto& vect_indices = var_grp.Indices();
  ASSERT_EQ(vect_indices.size(), 2);
  // 2 vars for the 1st pattern
  const auto& first_pattern_vars = vect_indices[0];
  ASSERT_EQ(first_pattern_vars.size(), 2);
  // variable indices
  ASSERT_EQ(first_pattern_vars[0], 1);
  ASSERT_EQ(first_pattern_vars[1], 3);
  // 1 var for the 2nd pattern
  const auto& second_pattern_vars = vect_indices[1];
  ASSERT_EQ(second_pattern_vars.size(), 1);
  ASSERT_EQ(second_pattern_vars[0], 2);
}

class OuterLoopBiLevelTest : public ::testing::Test {};

TEST_F(OuterLoopBiLevelTest, BiLevelBestUbInitialization) {
  Outerloop::OuterLoopBiLevel outerLoopBiLevel(data);
  ASSERT_EQ(outerLoopBiLevel.BilevelBestub(), 1e20);
}

TEST_F(OuterLoopBiLevelTest, Unfeasibility_With_High_Criteria) {
  Outerloop::OuterLoopBiLevel outerLoopBiLevel(data);
  const std::vector<double> criteria = {
    data[0].Criterion() + 12,
    data[1].Criterion() + 2024
  };
  const auto lambda = 1205;
  ASSERT_FALSE(outerLoopBiLevel.Update_bilevel_data_if_feasible(
    {}, criteria, 0., 0., lambda));
  ASSERT_EQ(outerLoopBiLevel.LambdaMin(), lambda);
}

TEST_F(OuterLoopBiLevelTest, UnfeasibilityWithInfiniteOverAllCosts) {
  Outerloop::OuterLoopBiLevel outerLoopBiLevel(data);
  const std::vector<double> criteria = {
    data[0].Criterion() - 12,
    data[1].Criterion() - 24
  };
  const auto lambda = 1205;
  const auto overall_cost = 1e20;
  ASSERT_FALSE(outerLoopBiLevel.Update_bilevel_data_if_feasible(
    {}, criteria, overall_cost, 0., lambda));
  ASSERT_EQ(outerLoopBiLevel.LambdaMin(), lambda);
}

TEST_F(OuterLoopBiLevelTest, FeasibleScenario) {
  const auto lambda = 1205;
  const auto overall_cost = 105;
  const auto invest_cost_at_x = 10;
  const auto cand = "candidate";
  const Point max_x = {{cand, 30}};
  Outerloop::OuterLoopBiLevel outerLoopBiLevel(data);
  const auto cand_coeff = 2.0;
  outerLoopBiLevel.Init({cand_coeff}, max_x, {{cand, 0}});
  ASSERT_EQ(outerLoopBiLevel.LambdaMax(), cand_coeff * max_x.at(cand));
  const std::vector<double> criteria = {
    data[0].Criterion() - 12,
    data[1].Criterion() - 24
  };
  const Point x = {{cand, max_x.at(cand) / 2}};

  ASSERT_TRUE(outerLoopBiLevel.Update_bilevel_data_if_feasible(
    x, criteria, overall_cost, invest_cost_at_x, lambda));
  ASSERT_EQ(outerLoopBiLevel.LambdaMin(), 0.);
  ASSERT_EQ(outerLoopBiLevel.LambdaMax(), invest_cost_at_x);
  ASSERT_EQ(outerLoopBiLevel.BilevelBestub(), overall_cost);
  ASSERT_EQ(outerLoopBiLevel.BilevelBestX(), x);
}

TEST_F(OuterLoopBiLevelTest,
       FeasibleScenario_InvestCostAtX_Higher_Than_Initial_LambdaMax) {
  const auto lambda = 1205;
  const auto overall_cost = 105;
  const auto cand = "candidate";
  const Point max_x = {{cand, 30}};
  Outerloop::OuterLoopBiLevel outerLoopBiLevel(data);
  const auto cand_coeff = 2.0;
  outerLoopBiLevel.Init({cand_coeff}, max_x, {{cand, 0}});
  const auto initial_lambda_max = cand_coeff * max_x.at(cand);
  // higher than lambda_max initial value
  const auto invest_cost_at_x = initial_lambda_max * 30;
  ASSERT_EQ(outerLoopBiLevel.LambdaMax(), initial_lambda_max);
  const std::vector<double> criteria = {
    data[0].Criterion() - 12,
    data[1].Criterion() - 24
  };
  const Point x = {{cand, max_x.at(cand) / 2}};

  ASSERT_TRUE(outerLoopBiLevel.Update_bilevel_data_if_feasible(
    x, criteria, overall_cost, invest_cost_at_x, lambda));
  ASSERT_EQ(outerLoopBiLevel.LambdaMin(), 0.);
  ASSERT_EQ(outerLoopBiLevel.LambdaMax(), initial_lambda_max);
  ASSERT_EQ(outerLoopBiLevel.BilevelBestub(), overall_cost);
  ASSERT_EQ(outerLoopBiLevel.BilevelBestX(), x);
}
