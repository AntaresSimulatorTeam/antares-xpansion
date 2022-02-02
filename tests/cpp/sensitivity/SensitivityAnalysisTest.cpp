#include "gtest/gtest.h"
#include "SensitivityAnalysis.h"

class SensitivityAnalysisTest : public ::testing::Test
{
public:
    double epsilon;
    double best_ub;
    std::map<int, std::string> id_to_name;
    std::string json_filename;
    std::shared_ptr<SensitivityWriter> writer;

    SolverAbstract::Ptr math_problem;

    SensitivityAnalysis sensitivity_analysis;

protected:
    void SetUp() override
    {
#if defined(WIN32) || defined(_WIN32)
        std::string data_test_dir = "../../data_test";
#else
        std::string data_test_dir = "../data_test";
#endif
        epsilon = 1e3;
        best_ub = 1e5;

        id_to_name = {{0, "candidate_0"}, {1, "candidate_1"}};

        json_filename = std::tmpnam(nullptr);
        writer = std::make_shared<SensitivityWriter>(json_filename);

        std::string last_master_mps_path = data_test_dir + "/mps/master_last_iteration.mps";
        std::string solver_name = "CBC";
        SolverFactory factory;
        math_problem = factory.create_solver(solver_name);
        math_problem->init();
        math_problem->read_prob_mps(last_master_mps_path);

        sensitivity_analysis = SensitivityAnalysis(epsilon, best_ub, id_to_name, math_problem, writer);
    }
};

TEST_F(SensitivityAnalysisTest, OutputDataInit)
{
    auto outputData = sensitivity_analysis.get_output_data();

    ASSERT_DOUBLE_EQ(outputData.epsilon, epsilon);
    ASSERT_DOUBLE_EQ(outputData.best_benders_cost, best_ub);
    ASSERT_DOUBLE_EQ(outputData.system_cost, 1e+20);
    ASSERT_DOUBLE_EQ(outputData.pb_objective, 1e+20);
    ASSERT_EQ(outputData.pb_status, SOLVER_STATUS::UNKNOWN);

    ASSERT_EQ(outputData.candidates.size(), 2);
}

TEST_F(SensitivityAnalysisTest, GetCapexMinSolution)
{
    sensitivity_analysis.launch();
    auto output_data = sensitivity_analysis.get_output_data();

    ASSERT_NEAR(output_data.pb_objective, 32.142857, 1e-6);
    ASSERT_NEAR(output_data.system_cost, 32.142857 + 45, 1e-6);

    ASSERT_DOUBLE_EQ(output_data.candidates["candidate_0"], 0);
    ASSERT_NEAR(output_data.candidates["candidate_1"], 0.357142, 1e-6);

    ASSERT_EQ(output_data.pb_status, SOLVER_STATUS::OPTIMAL);
}