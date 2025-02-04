#include <fstream>
#include <memory>

#include "antares-xpansion/lpnamer/input_reader/WeightsFileWriter.h"
#include "gtest/gtest.h"

class MockSolverAbstract : public SolverAbstract {
 public:
  int get_number_of_instances() override { return 1; }
  std::string get_solver_name() const override { return "MockSolver"; }
  void init() override {}
  void free() override {}
  void write_prob_mps(const std::filesystem::path &filename) override {}
  void write_prob_lp(const std::filesystem::path &filename) override {}
  void read_prob_mps(const std::filesystem::path &filename) override {}
  void read_prob_lp(const std::filesystem::path &filename) override {}
  void copy_prob(Ptr fictif_solv) override {}
  int get_ncols() const override { return 0; }
  int get_nrows() const override { return 0; }
  int get_nelems() const override { return 0; }
  int get_n_integer_vars() const override { return 0; }
  void get_obj(double *obj, int first, int last) const override {}
  void set_obj_to_zero() override {}
  void set_obj(const double *obj, int first, int last) override {}
  void get_rows(int *mstart, int *mclind, double *dmatval, int size, int *nels,
                int first, int last) const override {}
  void get_row_type(char *qrtype, int first, int last) const override {}
  void get_rhs(double *rhs, int first, int last) const override {}
  void get_rhs_range(double *range, int first, int last) const override {}
  void get_col_type(char *coltype, int first, int last) const override {}
  void get_lb(double *lb, int first, int last) const override {}
  void get_ub(double *ub, int first, int last) const override {}
  int get_row_index(const std::string &name) override { return 0; }
  int get_col_index(const std::string &name) override { return 0; }
  std::vector<std::string> get_row_names(int first, int last) override {
    return {};
  }
  std::vector<std::string> get_row_names() override { return {}; }
  std::vector<std::string> get_col_names(int first, int last) override {
    return {};
  }
  std::vector<std::string> get_col_names() override { return {}; }
  void del_rows(int first, int last) override {}
  void add_rows(int newrows, int newnz, const char *qrtype, const double *rhs,
                const double *range, const int *mstart, const int *mclind,
                const double *dmatval,
                const std::vector<std::string> &names = {}) override {}
  void add_cols(int newcol, int newnz, const double *objx, const int *mstart,
                const int *mrwind, const double *dmatval, const double *bdl,
                const double *bdu) override {}
  void add_name(int type, const char *cnames, int indice) override {}
  void add_names(int type, const std::vector<std::string> &cnames, int first,
                 int end) override {}
  void chg_obj(const std::vector<int> &mindex,
               const std::vector<double> &obj) override {}
  void chg_obj_direction(const bool minimize) override {}
  void chg_bounds(const std::vector<int> &mindex,
                  const std::vector<char> &qbtype,
                  const std::vector<double> &bnd) override {}
  void chg_col_type(const std::vector<int> &mindex,
                    const std::vector<char> &qctype) override {}
  void chg_rhs(int id_row, double val) override {}
  void chg_coef(int id_row, int id_col, double val) override {}
  void chg_row_name(int id_row, const std::string &name) override {}
  void chg_col_name(int id_col, const std::string &name) override {}
  int solve_lp() override { return 0; }
  int solve_mip() override { return 0; }
  void get_basis(int *rstatus, int *cstatus) const override {}
  double get_mip_value() const override { return 0.0; }
  double get_lp_value() const override { return 0.0; }
  int get_splex_num_of_ite_last() const override { return 0; }
  void get_lp_sol(double *primals, double *duals,
                  double *reduced_costs) override {}
  void get_mip_sol(double *primals) override {}
  void set_output_log_level(int loglevel) override {}
  void set_algorithm(const std::string &algo) override {}
  void set_threads(int n_threads) override {}
  void set_optimality_gap(double gap) override {}
  void set_simplex_iter(int iter) override {}
  void write_basis(const std::filesystem::path &filename) override {}
  void read_basis(const std::filesystem::path &filename) override {}
  void save_prob(const std::filesystem::path &filename) override {}
  void restore_prob(const std::filesystem::path &filename) override {}
};

class MockProblem : public Problem {
 public:
  MockProblem(int year) : Problem(std::make_shared<MockSolverAbstract>()) {
    Problem::mc_year = year;
  }
};

class WeightsFileWriterTest : public ::testing::Test {
 protected:
  void SetUp() override {
    const ::testing::TestInfo *test_info =
        ::testing::UnitTest::GetInstance()->current_test_info();

    tempDir = std::filesystem::temp_directory_path() / test_info->name();
    tempDirLp = tempDir / "lp";
    if (!std::filesystem::is_directory(tempDirLp)) {
      std::filesystem::create_directories(tempDirLp);
    }

    logger = std::make_shared<ProblemGenerationLog::ProblemGenerationLogger>(
        LogUtils::LOGLEVEL::NONE);

    problems_and_data = {
        {std::make_shared<MockProblem>(1),
         ProblemData("problem-1-1--optim-nb-1.mps", "variables.txt")},
        {std::make_shared<MockProblem>(1),
         ProblemData("problem-1-50--optim-nb-1.mps", "variables.txt")},
        {std::make_shared<MockProblem>(2),
         ProblemData("problem-2-10--optim-nb-1.mps", "variables.txt")},
        {std::make_shared<MockProblem>(2),
         ProblemData("problem-2-11--optim-nb-1.mps", "variables.txt")},
        {std::make_shared<MockProblem>(2),
         ProblemData("problem-2-30--optim-nb-1.mps", "variables.txt")},
        {std::make_shared<MockProblem>(3),
         ProblemData("problem-3-20--optim-nb-1.mps", "variables.txt")},
    };
  }

  void RunWeightsFileWriterTest(const std::string &solver_name,
                                const std::string &expected) {
    auto yearly_weight_writer = YearlyWeightsWriter(
        tempDir, {3, 5, 7}, "weights_123.txt", {1, 2, 3}, solver_name, logger);

    yearly_weight_writer.CreateWeightFile(problems_and_data);

    std::ifstream reader(tempDirLp / "weights_123.txt");
    std::string actual((std::istreambuf_iterator<char>(reader)),
                       std::istreambuf_iterator<char>());

    EXPECT_EQ(expected, actual);
  }

  std::filesystem::path tempDir;
  std::filesystem::path tempDirLp;
  std::shared_ptr<ProblemGenerationLog::ProblemGenerationLogger> logger;
  std::vector<std::pair<std::shared_ptr<Problem>, ProblemData>>
      problems_and_data;
};

TEST_F(WeightsFileWriterTest, CorrectlyWriteWeightsFileWithXpress) {
  std::string expected = R"xxx(problem-1-1--optim-nb-1.svf 3
problem-1-50--optim-nb-1.svf 3
problem-2-10--optim-nb-1.svf 5
problem-2-11--optim-nb-1.svf 5
problem-2-30--optim-nb-1.svf 5
problem-3-20--optim-nb-1.svf 7
WEIGHT_SUM 15
)xxx";
  RunWeightsFileWriterTest("xpress", expected);
}

TEST_F(WeightsFileWriterTest, CorrectlyWriteWeightsFileWithCoin) {
  std::string expected = R"xxx(problem-1-1--optim-nb-1.mps 3
problem-1-50--optim-nb-1.mps 3
problem-2-10--optim-nb-1.mps 5
problem-2-11--optim-nb-1.mps 5
problem-2-30--optim-nb-1.mps 5
problem-3-20--optim-nb-1.mps 7
WEIGHT_SUM 15
)xxx";
  RunWeightsFileWriterTest("coin", expected);
}
