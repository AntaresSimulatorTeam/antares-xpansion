#include <algorithm>

#include "BendersSequential.h"
#include "JsonWriter.h"
#include "gtest/gtest.h"
#include "logger/UserFile.h"

class FakeWorkerMaster : public WorkerMaster {
 public:
  WorkerMasterPtr worker_master;

  FakeWorkerMaster(WorkerMasterPtr worker_master)
      : worker_master(worker_master){};
  std::vector<int> get_id_nb_units() const override {
    return worker_master->get_id_nb_units();
  };

  void deactivate_integrity_constraints() const override {
    worker_master->deactivate_integrity_constraints();
  };

  void activate_integrity_constraints() const override {
    worker_master->activate_integrity_constraints();
  };

  SolverAbstract::Ptr solver() const override { return worker_master->_solver; }
};

class BendersSequentialDouble : public BendersSequential {
 public:
  bool parametrized_stop = false;
  int parametrized_nsubproblem = 0;
  double parametrized_lb = -1e20;
  double parametrized_best_ub = +1e20;

  explicit BendersSequentialDouble(BendersBaseOptions const &options,
                                   Logger &logger, Writer writer)
      : BendersSequential(options, logger, writer){};

  void init_data() override {
    BendersBase::init_data();
    _data.stop = parametrized_stop;
    _data.nsubproblem = parametrized_nsubproblem;
    _data.lb = parametrized_lb;
    _data.best_ub = parametrized_best_ub;
  };

  [[nodiscard]] WorkerMasterPtr get_master() const {
    return BendersSequential::get_master();
  };

  void get_master_value() override {};
  BendersData get_data() const { return _data; }
  void build_input_map() override{};
  void write_basis() const override{};
  void post_run_actions() const override{};
  void reset_master(WorkerMaster *worker_master) override {
    WorkerMasterPtr var;
    var.reset(worker_master);
    BendersBase::reset_master(new FakeWorkerMaster(var));
  };
  void free() override{};

  void set_data(bool stop, int nsubproblem) {
    parametrized_stop = stop;
    parametrized_nsubproblem = nsubproblem;
    _data.nsubproblem = nsubproblem;
  }

  void set_bounds(double lb, double best_ub) {
    parametrized_lb = lb;
    parametrized_best_ub = best_ub;
  }
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
    base_options.MASTER_NAME = "mip_toy_prob";
    base_options.STRUCTURE_FILE = "my_structure.txt";
    base_options.INPUTROOT = data_test_dir / "mps";
    base_options.SOLVER_NAME = "COIN";
    base_options.weights = {};

    return base_options;
  }

  BendersBaseOptions init_benders_options() {
    BaseOptions base_options(init_base_options());
    BendersBaseOptions options(base_options);

    options.MAX_ITERATIONS = 1;
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

TEST_F(BendersSequentialTest, ChangeToRelaxedAtBeginning) {
  BendersBaseOptions options = init_benders_options();
  BendersSequentialDouble benders(options, logger, writer);
  benders.set_data(true, 0);
  benders.launch();

  char col_type;
  std::vector<char> master_col_types;
  for (auto col_id : benders.get_master()->get_id_nb_units()) {
    benders.get_master()->solver()->get_col_type(&col_type, col_id, col_id);
    master_col_types.push_back(col_type);
  }

  ASSERT_TRUE(std::all_of(master_col_types.begin(), master_col_types.end(),
                          [](char element) { return element == 'C'; }));
  ASSERT_EQ(benders.get_data().is_in_initial_relaxation, true);
}

TEST_F(BendersSequentialTest, ReactivateIntegrityConstraint) {
  BendersBaseOptions options = init_benders_options();
  BendersSequentialDouble benders(options, logger, writer);
  benders.set_data(false, 0);
  benders.set_bounds(1000, 1001);
  benders.launch();

  char col_type;
  std::vector<char> master_col_types;
  for (auto col_id : benders.get_master()->get_id_nb_units()) {
    benders.get_master()->solver()->get_col_type(&col_type, col_id, col_id);
    master_col_types.push_back(col_type);
  }

  ASSERT_TRUE(std::all_of(master_col_types.begin(), master_col_types.end(),
                          [](char element) { return element == 'I'; }));
}