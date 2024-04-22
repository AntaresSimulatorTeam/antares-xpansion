
// #include "MasterUpdate.h"
// #include "ext_loop_test.h"

#include "LoggerFactories.h"
#include "MasterUpdate.h"
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
const auto OPTIONS_FILE = STUDY_PATH / "lp" / "options.json";

// class MasterUpdateBaseTest : public ::testing::TestWithParam<std::string> {
//  public:
//   pBendersBase benders;
//   std::shared_ptr<MathLoggerDriver> math_log_driver;
//   Logger logger;
//   Writer writer;

//   void SetUp() override {
//     math_log_driver = MathLoggerFactory::get_void_logger();
//     logger = build_void_logger();
//     writer = build_void_writer();
//   }
//   BendersBaseOptions BuildBendersOptions() {
//     SimulationOptions options(OPTIONS_FILE);
//     return options.get_benders_options();
//   }
// };

// auto solvers() {
//   std::vector<std::string> solvers_name;
//   solvers_name.push_back("COIN");
//   if (LoadXpress::XpressIsCorrectlyInstalled()) {
//     solvers_name.push_back("XPRESS");
//   }
//   return solvers_name;
// }

// INSTANTIATE_TEST_SUITE_P(availsolvers, MasterUpdateBaseTest,
//                          ::testing::ValuesIn(solvers()));
// double LambdaMax(pBendersBase benders) {
//   const auto& obj = benders->MasterObjectiveFunctionCoeffs();
//   const auto max_invest =
//   benders->BestIterationWorkerMaster().get_max_invest(); double lambda_max =
//   0; for (const auto& [var_name, var_id] : benders->MasterVariables()) {
//     lambda_max += obj[var_id] * max_invest.at(var_name);
//   }
//   return lambda_max;
// }

// void CheckMinInvestmentConstraint(const VariableMap& master_variables,
//                                   const std::vector<double>& expected_coeffs,
//                                   const double expected_rhs, char
//                                   expected_sign, const std::vector<double>&
//                                   coeffs, const double rhs, char sign) {
//   for (auto const& [name, var_id] : master_variables) {
//     ASSERT_EQ(expected_coeffs[var_id], coeffs[var_id]);
//   }

//   ASSERT_EQ(expected_rhs, rhs);
//   ASSERT_EQ(expected_sign, sign);
// }

// TEST_P(MasterUpdateBaseTest, ConstraintIsAddedBendersMPI) {
//   BendersBaseOptions benders_options = BuildBendersOptions();
//   CouplingMap coupling_map = build_input(benders_options.STRUCTURE_FILE);
//   // override solver
//   benders_options.SOLVER_NAME = GetParam();
//   benders = std::make_shared<BendersMpi>(benders_options, logger, writer,
//   *penv,
//                                          *pworld, math_log_driver);
//   benders->set_input_map(coupling_map);
//   benders->DoFreeProblems(false);
//   benders->InitializeProblems();
//   benders->launch();

//   MasterUpdateBase master_updater(benders, 0.5, 0.1);
//   // update lambda_max
//   master_updater.Init();
//   benders->init_data();
//   benders->launch();
//   auto num_constraints_master_before = benders->MasterGetnrows();
//   master_updater.Update(true);
//   auto num_constraints_master_after = benders->MasterGetnrows();

//   auto master_variables = benders->MasterVariables();
//   auto expected_coeffs = benders->MasterObjectiveFunctionCoeffs();

//   // criterion is low <=> lambda_max = min(lambda_max, invest_cost)
//   auto lambda_max = LambdaMax(benders);
//   auto expected_rhs = 0.5 * lambda_max;

//   //
//   ASSERT_EQ(num_constraints_master_after, num_constraints_master_before + 1);

//   std::vector<int> mstart(1 + 1);
//   auto n_elems = benders->MasterGetNElems();
//   auto nnz = master_variables.size();
//   std::vector<int> mclind(n_elems);
//   std::vector<double> matval(n_elems);
//   std::vector<int> p_nels(1, 0);

//   auto added_row_index = num_constraints_master_after - 1;
//   benders->MasterRowsCoeffs(mstart, mclind, matval, n_elems, p_nels,
//                             added_row_index, added_row_index);
//   std::vector<double> coeffs(benders->MasterGetncols());

//   for (auto ind = mstart[0]; ind < mstart[1]; ++ind) {
//     coeffs[mclind[ind]] = matval[ind];
//   }
//   double rhs;
//   benders->MasterGetRhs(rhs, added_row_index);
//   std::vector<char> qrtype(1);
//   benders->MasterGetRowType(qrtype, added_row_index, added_row_index);
//   CheckMinInvestmentConstraint(master_variables, expected_coeffs,
//   expected_rhs,
//                                'G', coeffs, rhs, qrtype[0]);
//   benders->free();
// }

// TEST_P(MasterUpdateBaseTest, InitialRhs) {
//   BendersBaseOptions benders_options = BuildBendersOptions();
//   benders_options.SOLVER_NAME = GetParam();
//   CouplingMap coupling_map = build_input(benders_options.STRUCTURE_FILE);

//   benders = std::make_shared<BendersMpi>(benders_options, logger, writer,
//   *penv,
//                                          *pworld, math_log_driver);
//   benders->set_input_map(coupling_map);
//   benders->DoFreeProblems(false);
//   benders->InitializeProblems();

//   benders->launch();

//   MasterUpdateBase master_updater(benders, 0.5, 0.1);
//   // update lambda_max
//   master_updater.Init();
//   auto lambda_max = LambdaMax(benders);
//   benders->init_data();
//   benders->launch();
//   master_updater.Update(true);
//   auto expected_initial_rhs = lambda_max * 0.5;

//   auto added_row_index = benders->MasterGetnrows() - 1;
//   double rhs;
//   benders->MasterGetRhs(rhs, added_row_index);
//   EXPECT_EQ(expected_initial_rhs, rhs);
//   benders->free();
// }

class OuterLoopPatternTest : public ::testing::Test {};

TEST_F(OuterLoopPatternTest, RegexGivenPrefixAndBody) {
  const std::string prefix = "prefix";
  const std::string body = "body";
  OuterLoopPattern o(prefix, body);

  auto ret_regex = o.MakeRegex();

  ASSERT_EQ(std::regex_search(prefix + body, ret_regex), true);
  ASSERT_EQ(std::regex_search(prefix + "::" + body + "::suffix", ret_regex),
            true);
  ASSERT_EQ(std::regex_search(body + prefix, ret_regex), false);
  ASSERT_EQ(std::regex_search(prefix + "::", ret_regex), false);
  ASSERT_EQ(std::regex_search(body, ret_regex), false);
}