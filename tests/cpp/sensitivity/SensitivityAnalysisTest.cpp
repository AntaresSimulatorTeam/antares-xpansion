#include "gtest/gtest.h"
#include "SensitivityAnalysis.h"

const std::string peak_name = "peak";
const std::string semibase_name = "semibase";

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
        epsilon = 100;
        best_ub = 1390;

        id_to_name = {{0, peak_name}, {1, semibase_name}};

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

    void verify_output_data(const SensitivityOutputData &output_data, const SensitivityOutputData &expec_output_data)
    {
        ASSERT_DOUBLE_EQ(output_data.epsilon, expec_output_data.epsilon);
        ASSERT_DOUBLE_EQ(output_data.best_benders_cost, expec_output_data.best_benders_cost);
        ASSERT_EQ(output_data.pbs_data.size(), expec_output_data.pbs_data.size());

        for (int pb_id(0); pb_id < output_data.pbs_data.size(); pb_id++)
        {
            verify_single_pb_data(output_data.pbs_data[pb_id], expec_output_data.pbs_data[pb_id]);
        }
    }

    void verify_single_pb_data(const SinglePbData &single_pb_data, const SinglePbData &expec_single_pb_data)
    {
        ASSERT_EQ(single_pb_data.pb_type, expec_single_pb_data.pb_type);
        ASSERT_EQ(single_pb_data.opt_dir, expec_single_pb_data.opt_dir);
        ASSERT_NEAR(single_pb_data.objective, expec_single_pb_data.objective, 1e-6);
        ASSERT_NEAR(single_pb_data.system_cost, expec_single_pb_data.system_cost, 1e-6);
        ASSERT_EQ(single_pb_data.status, expec_single_pb_data.status);

        verify_candidates(single_pb_data.candidates, expec_single_pb_data.candidates);
    }

    void verify_candidates(const Point &candidates, const Point &expec_candidates)
    {
        ASSERT_EQ(candidates.size(), expec_candidates.size());

        for (auto candidate_it = candidates.begin(), expec_candidate_it = expec_candidates.begin(); candidate_it != candidates.end(), expec_candidate_it != expec_candidates.end(); candidate_it++, expec_candidate_it++)
        {
            ASSERT_EQ(candidate_it->first, expec_candidate_it->first);
            ASSERT_NEAR(candidate_it->second, expec_candidate_it->second, 1e-6);
        }
    }
};

TEST_F(SensitivityAnalysisTest, OutputDataInit)
{
    SensitivityOutputData expec_output_data;
    expec_output_data.epsilon = epsilon;
    expec_output_data.best_benders_cost = best_ub;
    expec_output_data.pbs_data = {};

    auto output_data = sensitivity_analysis.get_output_data();

    verify_output_data(output_data, expec_output_data);
}

TEST_F(SensitivityAnalysisTest, GetCapexSolutions)
{
    sensitivity_analysis.get_capex_solutions();
    auto output_data = sensitivity_analysis.get_output_data();

    SensitivityOutputData expec_output_data;
    expec_output_data.epsilon = epsilon;
    expec_output_data.best_benders_cost = best_ub;

    SinglePbData capex_min_data;
    capex_min_data.pb_type = CAPEX_C;
    capex_min_data.opt_dir = MIN_C;
    capex_min_data.objective = 1040;
    capex_min_data.system_cost = 1390;
    capex_min_data.candidates = {{peak_name, 14}, {semibase_name, 10}};
    capex_min_data.status = SOLVER_STATUS::OPTIMAL;

    SinglePbData capex_max_data;
    capex_max_data.pb_type = CAPEX_C;
    capex_max_data.opt_dir = MAX_C;
    capex_max_data.objective = 1224.745762711;
    capex_max_data.system_cost = 1490;
    capex_max_data.candidates = {{peak_name, 17.22033898}, {semibase_name, 11.69491525}};
    capex_max_data.status = SOLVER_STATUS::OPTIMAL;

    expec_output_data.pbs_data.push_back(capex_min_data);
    expec_output_data.pbs_data.push_back(capex_max_data);

    verify_output_data(output_data, expec_output_data);
}

TEST_F(SensitivityAnalysisTest, FullSensitivityAnalysis)
{
    double epsilon = 10000;
    double best_ub = 1440683382.537683;

    std::map<int, std::string> id_to_name = {{3, "semibase"}, {1, "peak"}, {2, "pv"}, {0, "battery"}, {4, "transmission_line"}};

    std::string data_test_dir = "../data_test";
    std::string json_filename = data_test_dir + "/mps/sensitivity_out.json";
    std::shared_ptr<SensitivityWriter> writer = std::make_shared<SensitivityWriter>(json_filename);

    std::string last_master_mps_path = data_test_dir + "/mps/sensitivity.mps";
    std::string solver_name = "CBC";
    SolverFactory factory;
    SolverAbstract::Ptr math_problem = factory.create_solver(solver_name);
    math_problem->init();
    math_problem->read_prob_mps(last_master_mps_path);

    SensitivityAnalysis sensitivity_analysis = SensitivityAnalysis(epsilon, best_ub, id_to_name, math_problem, writer);
    sensitivity_analysis.launch();
}