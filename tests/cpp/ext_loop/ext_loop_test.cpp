
// #include "MasterUpdate.h"
// #include "ext_loop_test.h"

#include "LoggerFactories.h"
#include "OuterLoopCriterion.h"
#include "WriterFactories.h"
#include "gtest/gtest.h"
#include "multisolver_interface/environment.h"

int my_argc;
char** my_argv;

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
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

    // BaseOptions base_options;

    // base_options.OUTPUTROOT = ".";
    // base_options.INPUTROOT = ".";
    // base_options.STRUCTURE_FILE = "structure.txt";
    // base_options.LAST_ITERATION_JSON_FILE = "last_iteration_json_file.json";
    // base_options.MASTER_NAME = "master.mps";
    // base_options.SOLVER_NAME = "COIN";
    // base_options.SLAVE_WEIGHT = "CONSTANT";
    // base_options.LOG_LEVEL = 0;
    // base_options.SLAVE_WEIGHT_VALUE = 0;
    // base_options.RESUME = false;
    // base_options.weights = {};

    // BendersBaseOptions options(base_options);

    // options.MAX_ITERATIONS = -1;
    // options.ABSOLUTE_GAP = 0;
    // options.RELATIVE_GAP = 0;
    // options.RELAXED_GAP = 0;
    // options.TIME_LIMIT = 0;
    // options.SEPARATION_PARAM = 1;
    // options.AGGREGATION = false;
    // options.TRACE = false;
    // options.BOUND_ALPHA = false;
    // options.MASTER_FORMULATION = MasterFormulation::INTEGER;
    // options.CSV_NAME = "benders_output_trace";
    // options.LAST_MASTER_MPS = "last_master_mps.mps";
    // options.LAST_MASTER_BASIS = "last_master.bss";
    // options.BATCH_SIZE = 0;
    // return options;
  }
};

TEST_F(MasterUpdateBaseTest, ConstraintIsAddedBendersMPI) {
  // skipping if xpress is not available
  if (!LoadXpress::XpressIsCorrectlyInstalled()) {
    GTEST_SKIP();
  }
  mpi::environment env(my_argc, my_argv);
  mpi::communicator world;

  BendersBaseOptions benders_options = BuildBendersOptions();
  CouplingMap coupling_map = build_input(benders_options.STRUCTURE_FILE);

  benders = std::make_shared<BendersMpi>(benders_options, logger, writer, env,
                                         world, math_log_driver);
  benders->set_input_map(coupling_map);
  // benders->
}