#include "BendersSequential.h"
#include "JsonWriter.h"
#include "gtest/gtest.h"
#include "logger/UserFile.h"

class BendersSequentialDouble : public BendersSequential {
 public:
  explicit BendersSequentialDouble(BendersBaseOptions const &options,
                                   Logger &logger, Writer writer)
      : BendersSequential(options, logger, writer){};
};

class BendersSequentialTest : public ::testing::Test {
 public:
  Logger logger;
  Writer writer;
  const std::filesystem::path data_test_dir = "data_test";

 protected:
  void SetUp() override {
    logger = std::make_shared<xpansion::logger::UserFile>(std::tmpnam(nullptr));
    writer =
        std::make_shared<Output::JsonWriter>(nullptr, std::tmpnam(nullptr));
  }

  BaseOptions init_base_options() {
    BaseOptions base_options;

    base_options.LOG_LEVEL = 0;
    base_options.SLAVE_WEIGHT_VALUE = 1;
    base_options.OUTPUTROOT = "my_output";
    base_options.SLAVE_WEIGHT = "CONSTANT";
    base_options.MASTER_NAME = "lp_toy_prob";
    base_options.STRUCTURE_FILE = "my_structure.txt";
    base_options.INPUTROOT = data_test_dir / "mps";
    base_options.SOLVER_NAME = "COIN";
    base_options.weights = {};

    return base_options;
  }

  BendersBaseOptions init_benders_options() {
    BaseOptions base_options(init_base_options());
    BendersBaseOptions options(base_options);

    options.MAX_ITERATIONS = 5;
    options.ABSOLUTE_GAP = 1e-4;
    options.RELATIVE_GAP = 1e-4;
    options.RELAXED_GAP = 1e-2;
    options.TIME_LIMIT = 10;

    options.MASTER_FORMULATION = MasterFormulation::INTEGER;

    options.INITIAL_MASTER_RELAXATION = true;
    options.AGGREGATION = false;
    options.TRACE = true;
    options.BOUND_ALPHA = true;

    options.CSV_NAME = "my_trace";
    options.LAST_MASTER_MPS = "my_last_iteration";
    options.LAST_MASTER_BASIS = "my_last_basis";

    return options;
  }
};

TEST_F(BendersSequentialTest, my_test) {
  BendersBaseOptions options = init_benders_options();
  BendersSequentialDouble benders(options, logger, writer);
  benders.initialize_problems();
}