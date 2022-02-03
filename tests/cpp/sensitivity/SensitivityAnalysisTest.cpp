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

    void verify_output_data(const SensitivityOutputData &output_data, double expec_epsilon, double expec_best_ub, const std::vector<SinglePbData> &expec_pbs_data)
    {
        ASSERT_DOUBLE_EQ(output_data.epsilon, expec_epsilon);
        ASSERT_DOUBLE_EQ(output_data.best_benders_cost, expec_best_ub);
        ASSERT_EQ(output_data.pbs_data.size(), expec_pbs_data.size());

        for (int pb_id(0); pb_id < output_data.pbs_data.size(); pb_id++)
        {
            verify_single_pb_data(output_data.pbs_data[pb_id], expec_pbs_data[pb_id]);
        }
    }

    void verify_single_pb_data(const SinglePbData &single_pb_data, const SinglePbData &expec_single_pb_data)
    {
        ASSERT_EQ(single_pb_data.pb_type, expec_single_pb_data.pb_type);
        ASSERT_EQ(single_pb_data.opt_dir, expec_single_pb_data.opt_dir);
        ASSERT_NEAR(single_pb_data.objective, expec_single_pb_data.objective, 1e-8);
        ASSERT_NEAR(single_pb_data.system_cost, expec_single_pb_data.system_cost, 1e-8);
        ASSERT_EQ(single_pb_data.candidates, expec_single_pb_data.candidates);
        ASSERT_EQ(single_pb_data.status, expec_single_pb_data.status);
    }
};

TEST_F(SensitivityAnalysisTest, OutputDataInit)
{
    auto output_data = sensitivity_analysis.get_output_data();

    verify_output_data(output_data, epsilon, best_ub, {});
}

// TEST_F(SensitivityAnalysisTest, GetCapexSolutions)
// {
//     sensitivity_analysis.get_capex_solutions();
//     auto output_data = sensitivity_analysis.get_output_data();

//     ASSERT_NEAR(output_data.pb_objective, 32.142857, 1e-6);
//     ASSERT_NEAR(output_data.system_cost, 32.142857 + 45, 1e-6);

//     ASSERT_DOUBLE_EQ(output_data.candidates["candidate_0"], 0);
//     ASSERT_NEAR(output_data.candidates["candidate_1"], 0.357142, 1e-6);

//     ASSERT_EQ(output_data.pb_status, SOLVER_STATUS::OPTIMAL);
// }