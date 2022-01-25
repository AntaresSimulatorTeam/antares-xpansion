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

protected:
    void SetUp() override
    {
        epsilon = 1e3;
        best_ub = 1e5;

        id_to_name = {{0, "candidate_0"}, {1, "candidate_1"}};

        json_filename = std::tmpnam(nullptr);
        writer = std::make_shared<SensitivityWriter>(json_filename);

        std::string last_master_mps_path = "../data_test/mps/master_last_iteration.mps";
        std::string solver_name = "CBC";
        SolverFactory factory;
        math_problem = factory.create_solver(solver_name);
        math_problem->init();
        math_problem->read_prob_mps(last_master_mps_path);
    }
};

TEST_F(SensitivityAnalysisTest, OutputDataInit)
{
    auto sensitivity_analysis = SensitivityAnalysis(epsilon, best_ub, math_problem, id_to_name, writer);

    auto outputData = sensitivity_analysis.get_output_data();

    ASSERT_DOUBLE_EQ(outputData.epsilon, epsilon);
    ASSERT_DOUBLE_EQ(outputData.best_benders_cost, best_ub);
    ASSERT_DOUBLE_EQ(outputData.sensitivity_solution_overall_cost, 1e+20);
    ASSERT_DOUBLE_EQ(outputData.sensitivity_pb_objective, 1e+20);
    ASSERT_EQ(outputData.sensitivity_pb_status, SOLVER_STATUS::UNKNOWN);

    ASSERT_EQ(outputData.sensitivity_candidates.size(), 2);

}