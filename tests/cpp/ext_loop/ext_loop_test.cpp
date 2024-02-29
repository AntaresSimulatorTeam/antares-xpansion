
// #include "MasterUpdate.h"
// #include "ext_loop_test.h"

#include "LoggerFactories.h"
#include "MasterUpdate.h"
#include "OuterLoopCriterion.h"
#include "WriterFactories.h"
#include "gtest/gtest.h"
#include "multisolver_interface/environment.h"

int my_argc;
char** my_argv;

boost::mpi::environment* penv = nullptr;
boost::mpi::communicator* pworld = nullptr;
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

//-------------------- OuterLoopCriterionTest -------------------------

class OuterLoopCriterionTest : public ::testing::Test {};

TEST_F(OuterLoopCriterionTest, IsCriterionHigh) {
  double threshold = 1.4;
  double epsilon = 1e-1;
  double max_unsup_energy = 0.1;
  const ExternalLoopOptions options = {threshold, epsilon, max_unsup_energy};
  PlainData::Variables variables = {
      {"PositiveUnsuppliedEnergy::1", "PositiveUnsuppliedEnergy::2", "var3"},
      {0.2, 0.3, 68}};
  double criterion_value = 2.0;  // two vars named ^PositiveUnsuppliedEnergy
                                 // with value > max_unsup_energy

  PlainData::SubProblemData subProblemData;
  subProblemData.variables = variables;
  SubProblemDataMap cut_trace = {
      std::make_pair(std::string("P1"), subProblemData)};

  WorkerMasterData worker_master_data;
  worker_master_data._cut_trace = cut_trace;

  OuterloopCriterionLossOfLoad criterion(options);

  EXPECT_EQ(criterion.IsCriterionSatisfied(worker_master_data),
            CRITERION::HIGH);
  EXPECT_EQ(criterion.CriterionValue(), criterion_value);
}

TEST_F(OuterLoopCriterionTest, IsCriterionLow) {
  double threshold = 4;
  double epsilon = 1e-1;
  double max_unsup_energy = 0.1;
  const ExternalLoopOptions options = {threshold, epsilon, max_unsup_energy};
  PlainData::Variables variables = {
      {"PositiveUnsuppliedEnergy::1", "PositiveUnsuppliedEnergy::2", "var3"},
      {0.2, 0.3, 68}};
  double criterion_value = 2.0;  // two vars named PositiveUnsuppliedEnergy with
                                 // value > max_unsup_energy

  PlainData::SubProblemData subProblemData;
  subProblemData.variables = variables;
  SubProblemDataMap cut_trace = {
      std::make_pair(std::string("P1"), subProblemData)};

  WorkerMasterData worker_master_data;
  worker_master_data._cut_trace = cut_trace;

  OuterloopCriterionLossOfLoad criterion(options);

  EXPECT_EQ(criterion.IsCriterionSatisfied(worker_master_data), CRITERION::LOW);
  EXPECT_EQ(criterion.CriterionValue(), criterion_value);
}

TEST_F(OuterLoopCriterionTest, IsMet) {
  double threshold = 2.0;
  double epsilon = 1e-1;
  double max_unsup_energy = 0.1;
  const ExternalLoopOptions options = {threshold, epsilon, max_unsup_energy};
  PlainData::Variables variables = {
      {"PositiveUnsuppliedEnergy::1", "PositiveUnsuppliedEnergy::2", "var3"},
      {0.2, 0.3, 68}};
  double criterion_value = 2.0;  // two vars named PositiveUnsuppliedEnergy with
                                 // value > max_unsup_energy

  PlainData::SubProblemData subProblemData;
  subProblemData.variables = variables;
  SubProblemDataMap cut_trace = {
      std::make_pair(std::string("P1"), subProblemData)};

  WorkerMasterData worker_master_data;
  worker_master_data._cut_trace = cut_trace;

  OuterloopCriterionLossOfLoad criterion(options);

  EXPECT_EQ(criterion.IsCriterionSatisfied(worker_master_data),
            CRITERION::IS_MET);
  EXPECT_EQ(criterion.CriterionValue(), criterion_value);
}

//-------------------- MasterUpdateBaseTest -------------------------
const auto STUDY_PATH =
    std::filesystem::path("data_test") / "external_loop_test";
const auto OPTIONS_FILE = STUDY_PATH / "lp" / "options.json";

class MasterUpdateBaseTest : public ::testing::Test {
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
    return options.get_benders_options();
  }
};

double LambdaMax(pBendersBase benders) {
  const auto& obj = benders->ObjectiveFunctionCoeffs();
  const auto max_invest = benders->BestIterationWorkerMaster().get_max_invest();
  double lambda_max = 0;
  for (const auto& [var_name, var_id] : benders->MasterVariables()) {
    lambda_max += obj[var_id] * max_invest.at(var_name);
  }
  return lambda_max;
}

TEST_F(MasterUpdateBaseTest, ConstraintIsAddedBendersMPI) {
  // skipping if xpress is not available
  if (!LoadXpress::XpressIsCorrectlyInstalled()) {
    GTEST_SKIP();
  }

  BendersBaseOptions benders_options = BuildBendersOptions();
  CouplingMap coupling_map = build_input(benders_options.STRUCTURE_FILE);

  benders = std::make_shared<BendersMpi>(benders_options, logger, writer, *penv,
                                         *pworld, math_log_driver);
  benders->set_input_map(coupling_map);
  benders->DoFreeProblems(false);
  benders->InitializeProblems();
  benders->launch();

  MasterUpdateBase master_updater(benders, 0.5);
  // update lambda_max
  master_updater.Init();
  benders->ResetData(3.0);
  benders->launch();
  auto num_constraints_master_before = benders->MasterGetnrows();
  master_updater.Update(CRITERION::LOW);
  auto num_constraints_master_after = benders->MasterGetnrows();

  ASSERT_EQ(num_constraints_master_after, num_constraints_master_before + 1);
  benders->free();
}

TEST_F(MasterUpdateBaseTest, InitialRhs) {
  // skipping if xpress is not available
  if (!LoadXpress::XpressIsCorrectlyInstalled()) {
    GTEST_SKIP();
  }

  BendersBaseOptions benders_options = BuildBendersOptions();
  CouplingMap coupling_map = build_input(benders_options.STRUCTURE_FILE);

  benders = std::make_shared<BendersMpi>(benders_options, logger, writer, *penv,
                                         *pworld, math_log_driver);
  benders->set_input_map(coupling_map);
  benders->DoFreeProblems(false);
  benders->InitializeProblems();

  benders->launch();

  MasterUpdateBase master_updater(benders, 0.5);
  // update lambda_max
  master_updater.Init();
  auto lambda_max = LambdaMax(benders);
  benders->ResetData(3.0);
  benders->launch();
  master_updater.Update(CRITERION::HIGH);
  auto expected_initial_rhs = lambda_max * 0.5;

  auto added_row_index = benders->MasterGetnrows() - 1;
  double rhs;
  benders->MasterGetRhs(rhs, added_row_index);
  EXPECT_EQ(expected_initial_rhs, rhs);
  benders->free();
}