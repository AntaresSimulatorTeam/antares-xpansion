#include <algorithm>

#include "BendersSequential.h"
#include "JsonWriter.h"
#include "LoggerStub.h"
#include "gtest/gtest.h"

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
  int parametrized_it = 0;
  int parametrized_nsubproblem = 0;
  double parametrized_lb = -1e20;
  double parametrized_ub = +1e20;
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
    _data.it = parametrized_it;
  };

  [[nodiscard]] WorkerMasterPtr get_master() const {
    return BendersSequential::get_master();
  };

  void get_master_value() override{};
  void build_cut() override{};
  void compute_ub() override { _data.ub = parametrized_ub; };
  BendersData get_data() const { return _data; }
  void build_input_map() override{};
  void write_basis() const override{};
  void update_trace() override{};
  void post_run_actions() const override{};
  void SaveCurrentBendersData() override{};
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
  void set_bestx(Point x_out, Point x_in) {
    _data.x_out = x_out;
    _data.x_in = x_in;
  }
  void set_ub(double ub) { parametrized_ub = ub; }
  void set_it(int it) { parametrized_it = it; }
};

class BendersSequentialTest : public ::testing::Test {
 public:
  Logger logger;
  Writer writer;
  const std::filesystem::path data_test_dir = "data_test";

 protected:
  void SetUp() override {
    logger = std::make_shared<LoggerNOOPStub>();
    writer = std::make_shared<Output::JsonWriter>(std::make_shared<Clock>(),
                                                  std::tmpnam(nullptr));
  }

  BaseOptions init_base_options() const {
    BaseOptions base_options;

    base_options.LOG_LEVEL = 0;
    base_options.SLAVE_WEIGHT_VALUE = 1;
    base_options.OUTPUTROOT = "my_output";
    base_options.SLAVE_WEIGHT = "CONSTANT";
    base_options.MASTER_NAME = "mip_toy_prob";
    base_options.STRUCTURE_FILE = "my_structure.txt";
    base_options.INPUTROOT = (data_test_dir / "mps").string();
    base_options.SOLVER_NAME = "COIN";
    base_options.weights = {};
    base_options.RESUME = false;

    return base_options;
  }

  BendersBaseOptions init_benders_options(MasterFormulation master_formulation,
                                          int max_iter,
                                          double sep_param) const {
    BaseOptions base_options(init_base_options());
    BendersBaseOptions options(base_options);

    options.MAX_ITERATIONS = max_iter;
    options.ABSOLUTE_GAP = 1e-4;
    options.RELATIVE_GAP = 1e-4;
    options.RELAXED_GAP = 1e-2;
    options.TIME_LIMIT = 10;
    options.SEPARATION_PARAM = sep_param;

    options.MASTER_FORMULATION = master_formulation;

    options.INITIAL_MASTER_RELAXATION = true;
    options.AGGREGATION = false;
    options.TRACE = false;
    options.BOUND_ALPHA = true;

    options.CSV_NAME = "my_trace";
    options.LAST_MASTER_MPS = "my_last_iteration";
    options.LAST_MASTER_BASIS = "my_last_basis";

    return options;
  }

  BendersSequentialDouble init_benders_sequential(
      MasterFormulation master_formulation, int max_iter, double sep_param) {
    BendersBaseOptions options =
        init_benders_options(master_formulation, max_iter, sep_param);
    return BendersSequentialDouble(options, logger, writer);
  }

  std::vector<char> get_nb_units_col_types(
      const BendersSequentialDouble &benders) const {
    char col_type;
    std::vector<char> nb_units_col_types;
    for (auto col_id : benders.get_master()->get_id_nb_units()) {
      benders.get_master()->solver()->get_col_type(&col_type, col_id, col_id);
      nb_units_col_types.push_back(col_type);
    }
    return nb_units_col_types;
  }
};

TEST_F(BendersSequentialTest, MasterRelaxedAtBeginning) {
  MasterFormulation master_formulation = MasterFormulation::INTEGER;
  int max_iter = 1;
  double sep_param = 1;
  BendersSequentialDouble benders =
      init_benders_sequential(master_formulation, max_iter, sep_param);

  benders.set_data(true, 0);
  benders.launch();

  std::vector<char> nb_units_col_types = get_nb_units_col_types(benders);

  ASSERT_TRUE(std::all_of(nb_units_col_types.begin(), nb_units_col_types.end(),
                          [](char element) { return element == 'C'; }));
}

TEST_F(BendersSequentialTest, CheckDataPreRelaxation) {
  MasterFormulation master_formulation = MasterFormulation::INTEGER;
  int max_iter = 1;
  double sep_param = 1;
  BendersSequentialDouble benders =
      init_benders_sequential(master_formulation, max_iter, sep_param);

  benders.set_data(true, 0);
  benders.launch();

  ASSERT_EQ(benders.get_data().is_in_initial_relaxation, true);
}

TEST_F(BendersSequentialTest, ReactivateIntegrityConstraint) {
  MasterFormulation master_formulation = MasterFormulation::INTEGER;
  int max_iter = 1;
  double sep_param = 1;
  BendersSequentialDouble benders =
      init_benders_sequential(master_formulation, max_iter, sep_param);

  benders.set_data(false, 0);
  benders.set_bounds(1000, 1001);
  benders.launch();

  std::vector<char> nb_units_col_types = get_nb_units_col_types(benders);

  ASSERT_TRUE(std::all_of(nb_units_col_types.begin(), nb_units_col_types.end(),
                          [](char element) { return element == 'I'; }));
}

TEST_F(BendersSequentialTest, CheckDataPostRelaxation) {
  MasterFormulation master_formulation = MasterFormulation::INTEGER;
  int max_iter = 1;
  double sep_param = 1;
  BendersSequentialDouble benders =
      init_benders_sequential(master_formulation, max_iter, sep_param);

  benders.set_data(false, 0);
  benders.set_bounds(1000, 1001);
  benders.launch();

  ASSERT_EQ(benders.get_data().is_in_initial_relaxation, false);
  ASSERT_EQ(benders.get_data().best_ub, 1e+20);
  ASSERT_EQ(benders.get_data().best_it, 0);
}

TEST_F(BendersSequentialTest, CheckInOutDataWhithoutImprovement) {
  MasterFormulation master_formulation = MasterFormulation::RELAXED;
  double sep_param = 0.8;
  int current_it = 4;
  int max_iter = current_it + 1;

  double init_lb = 1000;
  double init_ub = 1001;
  double current_ub = 2000;

  Point x_out = {{"x1", 1}, {"x2", 2}};
  Point x_in = {{"x1", 3}, {"x2", 6}};

  BendersSequentialDouble benders =
      init_benders_sequential(master_formulation, max_iter, sep_param);

  benders.set_data(false, 0);
  benders.set_bounds(init_lb, init_ub);
  benders.set_it(current_it);
  benders.set_bestx(x_out, x_in);
  benders.set_ub(current_ub);

  Point expec_x_cut;
  for (const auto &[coord, val] : x_out) {
    expec_x_cut[coord] =
        sep_param * x_out[coord] + (1 - sep_param) * x_in[coord];
  }

  benders.launch();

  EXPECT_EQ(benders.get_data().x_out, x_out);
  EXPECT_EQ(benders.get_data().x_cut, expec_x_cut);
  EXPECT_EQ(benders.get_data().x_in, x_in);
  EXPECT_EQ(benders.get_data().best_ub, init_ub);
  EXPECT_EQ(benders.get_data().best_it, 0);
}

TEST_F(BendersSequentialTest, CheckInOutDataWhenImprovement) {
  MasterFormulation master_formulation = MasterFormulation::RELAXED;
  double sep_param = 0.8;
  int current_it = 4;
  int max_iter = current_it + 1;

  double init_lb = 1000;
  double init_ub = 1001;
  double current_ub = 1000.5;

  Point x_out = {{"x1", 1}, {"x2", 2}};
  Point x_in = {{"x1", 3}, {"x2", 6}};

  BendersSequentialDouble benders =
      init_benders_sequential(master_formulation, max_iter, sep_param);

  benders.set_data(false, 0);
  benders.set_bounds(init_lb, init_ub);
  benders.set_it(current_it);
  benders.set_bestx(x_out, x_in);
  benders.set_ub(current_ub);

  Point expec_x_cut;
  for (const auto &[coord, val] : x_out) {
    expec_x_cut[coord] =
        sep_param * x_out[coord] + (1 - sep_param) * x_in[coord];
  }

  benders.launch();

  EXPECT_EQ(benders.get_data().x_out, x_out);
  EXPECT_EQ(benders.get_data().x_cut, expec_x_cut);
  EXPECT_EQ(benders.get_data().x_in, expec_x_cut);
  EXPECT_EQ(benders.get_data().best_ub, current_ub);
  EXPECT_EQ(benders.get_data().best_it, current_it + 1);
}