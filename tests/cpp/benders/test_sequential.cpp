//
// Created by marechaljas on 19/07/22.
//
#include <gtest/gtest.h>

#include <utility>
#include "BendersSequential.h"

class NOOPLogger: public ILogger {
 public:
  void display_message(const std::string& str) override {}
  void log_at_initialization(const int it_number) override {}
  void log_iteration_candidates(const LogData& d) override {}
  void log_master_solving_duration(double durationInSeconds) override {}
  void log_subproblems_solving_duration(double durationInSeconds) override {}
  void log_at_iteration_end(const LogData& d) override {}
  void log_at_ending(const LogData& d) override {}
  void log_total_duration(double durationInSeconds) override {}
  void log_stop_criterion_reached(
      const StoppingCriterion stopping_criterion) override {}
  void display_restart_message() override {}
  void restart_elapsed_time(const double elapsed_time) override {}
  void number_of_iterations_before_restart(const int num_iterations) override {}
  void restart_best_iteration(const int best_iterations) override {}
  void restart_best_iterations_infos(
      const LogData& best_iterations_data) override {}
};

class NOOPWriter : public Output::OutputWriter {
 public:
  void update_solution(const Output::SolutionData& solution_data) override {}
  void dump() override {}
  void initialize() override {}
  void end_writing(const Output::IterationsData& iterations_data) override {}
  void write_solver_name(const std::string& solver_name) override {}
  void write_master_name(const std::string& master_name) override {}
  void write_log_level(const int log_level) override {}
  void write_solution(const Output::SolutionData& solution) override {}
  void write_iteration(const Output::Iteration& iteration_data,
                       const size_t iteration_num) override {}
  void updateBeginTime() override {}
  void updateEndTime() override {}
  void write_nbweeks(const int nb_weeks) override {}
  void write_duration(const double duration) override {}
  std::string solution_status() const override { return std::string(); }
};

class StubMPSUtils: public MPSUtils {
 public:
  CouplingMap build_input(
      const std::filesystem::path& structure_path) const override {
        return {
            {"master", {{"Variable1", 1}}},
            {"subproblem", {{"x2", 2}}}
           };

  }
};

/**
 * We need some adjustments to test properly
 * - Need to make public some function
 * - Need to override dome factory methods
 * - Behave like a mock in some cases
 */
class BendersSequentialSUT : public BendersSequential {
 public:
  BendersSequentialSUT(BendersBaseOptions const& options,
                          Logger logger,
                          Writer writer,
                          std::shared_ptr<MPSUtils> mps_utils)
      : BendersSequential(options, logger, std::move(writer), std::move(mps_utils))
  {}

  void build_input_map() override { BendersBase::build_input_map(); }
  void getSubproblemCut(SubproblemCutPackage& subproblem_cut_package) override {
    BendersBase::getSubproblemCut(subproblem_cut_package);
  }

  mutable unsigned int subproblem_construction_count = 0;

 protected:
  std::shared_ptr<SubproblemWorker> makeSubproblemWorker(
      const std::pair<std::string, VariableMap>& kvp) const override {
    subproblem_construction_count++;
    return BendersBase::makeSubproblemWorker(kvp);
  }
};

class BendersSequentialTest : public ::testing::Test {
 public:
  Logger logger_ = std::make_shared<NOOPLogger>();
  Writer writer_ = std::make_shared<NOOPWriter>();
  std::shared_ptr<MPSUtils> mps_utils_ = std::make_shared<StubMPSUtils>();
  const std::filesystem::path data_test_dir = "data_test";
  BendersBaseOptions benders_base_options_ = init_benders_options();

 protected:
  void SetUp() override {
    std::cout << "Working dir " << std::filesystem::current_path() << std::endl;
    benders_base_options_ = init_benders_options();
  }

  BaseOptions init_base_options() const {
    BaseOptions base_options;

    base_options.LOG_LEVEL = 0;
    base_options.SLAVE_WEIGHT_VALUE = 1;
    base_options.OUTPUTROOT = "my_output";
    base_options.SLAVE_WEIGHT = "CONSTANT";
    base_options.MASTER_NAME = "master";
    base_options.STRUCTURE_FILE = "my_structure.txt";
    base_options.INPUTROOT = (data_test_dir / "unit_tests").string();
    base_options.SOLVER_NAME = "COIN";
    base_options.weights = {};

    return base_options;
  }

  BendersBaseOptions init_benders_options() const {
    BaseOptions base_options(init_base_options());
    BendersBaseOptions options(base_options);

    options.MAX_ITERATIONS = 1;
    options.ABSOLUTE_GAP = 1e-4;
    options.RELATIVE_GAP = 1e-4;
    options.TIME_LIMIT = 10;

    options.AGGREGATION = false;
    options.TRACE = true;
    options.BOUND_ALPHA = true;

    options.CSV_NAME = "my_trace";
    options.LAST_MASTER_MPS = "my_last_iteration";
    options.LAST_MASTER_BASIS = "my_last_basis";
    options.CONSTRUCT_ALL_PROBLEMS = true;

    return options;
  }
};

TEST_F(BendersSequentialTest, problem_initialization_contains_all_problems) {
  BendersSequentialSUT benders_sequential(benders_base_options_, logger_, writer_, mps_utils_);
  benders_sequential.build_input_map();

  benders_sequential.initialize_problems();

  ASSERT_EQ(benders_sequential.getSubproblemMap().size(), benders_sequential.getSubproblems().size());
  ASSERT_EQ(benders_sequential.getSubproblemMap().size(), 1);
}

TEST_F(BendersSequentialTest, produce_cut_for_problem) {
  BendersSequentialSUT benders_sequential(benders_base_options_, logger_, writer_, mps_utils_);
  benders_sequential.build_input_map();

  benders_sequential.initialize_problems();
  ASSERT_EQ(benders_sequential.subproblem_construction_count, 1);

  SubproblemCutPackage subproblem_cut_package;
  benders_sequential.getSubproblemCut(subproblem_cut_package);

  ASSERT_EQ(benders_sequential.subproblem_construction_count, 1);
  ASSERT_EQ(subproblem_cut_package.size(), 1);
  ASSERT_NE(subproblem_cut_package.find("subproblem"), subproblem_cut_package.end());
}

TEST_F(BendersSequentialTest, Option_to_limit_memory_doesnt_produce_problems) {
  benders_base_options_.CONSTRUCT_ALL_PROBLEMS = false;
  BendersSequentialSUT benders_sequential(benders_base_options_, logger_, writer_, mps_utils_);
  benders_sequential.build_input_map();

  benders_sequential.initialize_problems();

  ASSERT_FALSE(benders_sequential.Options().CONSTRUCT_ALL_PROBLEMS);
  ASSERT_EQ(benders_sequential.getSubproblemMap().size(), benders_sequential.getSubproblems().size());
  ASSERT_EQ(benders_sequential.getSubproblemMap().size(), 0);
}

TEST_F(BendersSequentialTest, produce_cut_for_problem_even_without_all_problems_constructed) {
  benders_base_options_.CONSTRUCT_ALL_PROBLEMS = false;
  BendersSequentialSUT benders_sequential(benders_base_options_, logger_, writer_, mps_utils_);
  benders_sequential.build_input_map();

  benders_sequential.initialize_problems();
  ASSERT_EQ(benders_sequential.subproblem_construction_count, 0);

  SubproblemCutPackage subproblem_cut_package;
  benders_sequential.getSubproblemCut(subproblem_cut_package);

  ASSERT_EQ(benders_sequential.subproblem_construction_count, 1);
  ASSERT_FALSE(benders_sequential.Options().CONSTRUCT_ALL_PROBLEMS);
  ASSERT_EQ(subproblem_cut_package.size(), 1);
  ASSERT_NE(subproblem_cut_package.find("subproblem"), subproblem_cut_package.end());
}