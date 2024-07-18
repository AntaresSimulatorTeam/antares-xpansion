
// #include "MasterUpdate.h"

#include "LoggerFactories.h"
#include "MasterUpdate.h"
#include "OuterLoopBenders.h"
#include "OuterLoopInputDataReader.h"
#include "WriterFactories.h"
#include "gtest/gtest.h"
#include "multisolver_interface/environment.h"

int my_argc;
char** my_argv;

boost::mpi::environment* penv = nullptr;
boost::mpi::communicator* pworld = nullptr;

using namespace Outerloop;

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);

  mpi::environment env(my_argc, my_argv);
  mpi::communicator world;
  penv = &env;
  pworld = &world;
  my_argc = argc;
  my_argv = argv;
  return RUN_ALL_TESTS();
}

//-------------------- MasterUpdateBaseTest -------------------------
const auto STUDY_PATH =
    std::filesystem::path("data_test") / "external_loop_test";
const auto LP_DIR = STUDY_PATH / "lp";
const auto OPTIONS_FILE = LP_DIR / "options.json";
const auto OUTER_OPTIONS_FILE = LP_DIR / "adequacy_criterion.yml";

class MasterUpdateBaseTest : public ::testing::TestWithParam<std::string> {
 public:
  pBendersBase benders;
  std::shared_ptr<MathLoggerDriver> math_log_driver;
  Logger logger;
  Writer writer;

  void SetUp() override {
    math_log_driver = MathLoggerFactory::get_void_logger();
    logger = build_void_logger();
    writer = build_void_writer();
  }
  BendersBaseOptions BuildBendersOptions() {
    SimulationOptions options(OPTIONS_FILE);
    return options.get_bendersoptions();
  }
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

// TODOs
static void OuterLoopCheckFeasibility(pBendersBase benders) {
  std::vector<double> obj_coeff;
  obj_coeff = benders->MasterObjectiveFunctionCoeffs();

  // /!\ partially
  benders->SetMasterObjectiveFunctionCoeffsToZeros();

  benders->launch();
  benders->SetMasterObjectiveFunction(obj_coeff.data(), 0,
                                      obj_coeff.size() - 1);
  benders->UpdateOverallCosts();
  benders->OuterLoopBilevelChecks();
  // de-comment for general case
  //  cuts_manager_->Save(benders->AllCuts());
  // auto cuts = cuts_manager_->Load();
  // High
  if (!benders->ExternalLoopFoundFeasible()) {
    std::ostringstream err_msg;
    err_msg << PrefixMessage(LogUtils::LOGLEVEL::FATAL, "Outer Loop")
            << "Criterion cannot be satisfied for your study\n";
    throw Outerloop::CriterionCouldNotBeSatisfied(err_msg.str(), LOGLOCATION);
  }

  benders->InitExternalValues(false, 0.0);
}

TEST_P(MasterUpdateBaseTest, ConstraintIsAddedBendersMPI) {
  BendersBaseOptions bendersoptions = BuildBendersOptions();
  CouplingMap coupling_map =
      build_input(std::filesystem::path(bendersoptions.INPUTROOT) /
                  bendersoptions.STRUCTURE_FILE);
  // override solver
  bendersoptions.SOLVER_NAME = GetParam();
  bendersoptions.EXTERNAL_LOOP_OPTIONS.DO_OUTER_LOOP = true;
  bendersoptions.EXTERNAL_LOOP_OPTIONS.OUTER_LOOP_OPTION_FILE =
      OUTER_OPTIONS_FILE.string();
  benders = std::make_shared<BendersMpi>(bendersoptions, logger, writer, *penv,
                                         *pworld, math_log_driver);
  benders->set_input_map(coupling_map);
  benders->DoFreeProblems(false);
  benders->InitializeProblems();
  OuterLoopCheckFeasibility(benders);

  auto num_constraints_master_before = benders->MasterGetnrows();
  auto lambda_min = benders->OuterLoopLambdaMin();
  auto lambda_max = benders->OuterLoopLambdaMax();
  auto expected_lambda_max = LambdaMax(benders);
  //--------
  ASSERT_EQ(lambda_max, expected_lambda_max);
  //--------
  MasterUpdateBase master_updater(benders, 0.5);

  master_updater.Update(lambda_min, lambda_max);
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
  OuterLoopPattern o(prefix, body);

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
    OuterLoopInputFromYaml parser;
    parser.Read(empty);
  } catch (const OuterLoopInputFileError& e) {
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
    OuterLoopInputFromYaml parser;
    parser.Read(empty);
  } catch (const OuterLoopInputFileIsEmpty& e) {
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
    OuterLoopInputFromYaml parser;
    parser.Read(empty_patterns);
  } catch (const OuterLoopInputFileNoPatternFound& e) {
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
    OuterLoopInputFromYaml parser;
    parser.Read(patterns_not_array);
  } catch (const OuterLoopInputPatternsShouldBeArray& e) {
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
  auto data = OuterLoopInputFromYaml().Read(valid_file);

  ASSERT_EQ(data.StoppingThreshold(), 1e-4);
  ASSERT_EQ(data.CriterionCountThreshold(), 1e-1);

  auto patterns = data.OuterLoopData();
  ASSERT_EQ(patterns.size(), 4);
  auto pattern1 = patterns[0];
  ASSERT_EQ(pattern1.Criterion(), 185.0);
  ASSERT_EQ(pattern1.Pattern().GetBody(), "N0");
}
