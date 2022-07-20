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

class NOOPWritter: public Output::OutputWriter {
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
            {"master", {{"Variable1", 0}}},
            {"SubProblem", {{"Variable2", 0}}}
           };

  }
};

class PublicBendersSequential: public BendersSequential {
 public:
  PublicBendersSequential(BendersBaseOptions const& options,
                          Logger logger,
                          Writer writer,
                          std::shared_ptr<MPSUtils> mps_utils)
      : BendersSequential(options, logger, std::move(writer), std::move(mps_utils))
  {}

  void build_input_map() override { BendersBase::build_input_map(); }
};

class BendersSequentialTest : public ::testing::Test {
 public:
  Logger logger_ = std::make_shared<NOOPLogger>();
  Writer writer_ = std::make_shared<NOOPWritter>();
  std::shared_ptr<MPSUtils> mps_utils_ = std::make_shared<StubMPSUtils>();
  const std::filesystem::path data_test_dir = "data_test";
  BendersBaseOptions benders_base_options_ = init_benders_options();

 protected:
  void SetUp() override {
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

    return options;
  }
};

TEST_F(BendersSequentialTest, problem_initialization_contains_all_problems) {
  PublicBendersSequential bendersSequential(benders_base_options_, logger_, writer_, mps_utils_);
  bendersSequential.build_input_map();

  bendersSequential.initialize_problems();

  ASSERT_EQ(bendersSequential.getSubproblemMap().size(), bendersSequential.getSubproblems().size());
  ASSERT_EQ(bendersSequential.getSubproblemMap().size(), 1);
}