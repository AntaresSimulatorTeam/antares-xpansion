#include "gtest/gtest.h"
#include "SensitivityAnalysis.h"

class SensitivityAnalysisTest : public ::testing::Test
{
public:
    double epsilon;
    double best_ub;

    std::string peak_name;
    std::string semibase_name;

    std::shared_ptr<SensitivityWriter> writer;

    SensitivityInputData input_data;

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

        peak_name = "peak";
        semibase_name = "semibase";

        std::map<std::string, int> name_to_id = {{peak_name, 0}, {semibase_name, 1}};

        bool capex = true;
        std::vector<std::string> projection = {peak_name, semibase_name};

        std::string json_filename = std::tmpnam(nullptr);
        writer = std::make_shared<SensitivityWriter>(json_filename);

        std::string last_master_mps_path = data_test_dir + "/mps/master_last_iteration.mps";
        std::string solver_name = "CBC";

        SolverFactory factory;
        SolverAbstract::Ptr math_problem = factory.create_solver(solver_name);
        math_problem->init();
        math_problem->read_prob_mps(last_master_mps_path);

        input_data = {epsilon, best_ub, name_to_id, math_problem, capex, projection};
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
    auto sensitivity_analysis = SensitivityAnalysis(input_data, writer);

    auto expec_output_data = SensitivityOutputData(epsilon, best_ub);

    auto output_data = sensitivity_analysis.get_output_data();

    verify_output_data(output_data, expec_output_data);
}

TEST_F(SensitivityAnalysisTest, GetCapexSolutions)
{
    input_data.capex = true;
    input_data.projection = {};
    auto sensitivity_analysis = SensitivityAnalysis(input_data, writer);
    sensitivity_analysis.launch();

    auto output_data = sensitivity_analysis.get_output_data();

    auto capex_min_data = SinglePbData(CAPEX_C, MIN_C, 1040, 1390, {{peak_name, 14}, {semibase_name, 10}}, SOLVER_STATUS::OPTIMAL);

    auto capex_max_data = SinglePbData(CAPEX_C, MAX_C, 1224.745762711, 1490, {{peak_name, 17.22033898}, {semibase_name, 11.69491525}}, SOLVER_STATUS::OPTIMAL);

    std::vector<SinglePbData> pbs_data = {capex_min_data, capex_max_data};

    auto expec_output_data = SensitivityOutputData(epsilon, best_ub, pbs_data);

    verify_output_data(output_data, expec_output_data);
}

TEST_F(SensitivityAnalysisTest, GetCandidatesProjection)
{
    input_data.capex = false;
    input_data.projection = {peak_name, semibase_name};
    auto sensitivity_analysis = SensitivityAnalysis(input_data, writer);
    sensitivity_analysis.launch();

    auto output_data = sensitivity_analysis.get_output_data();

    auto projection_min_peak = SinglePbData(PROJECTION_C + " " + peak_name, MIN_C, 13.83050847, 1490, {{peak_name, 13.83050847}, {semibase_name, 11.694915254}}, SOLVER_STATUS::OPTIMAL);

    auto projection_max_peak = SinglePbData(PROJECTION_C + " " + peak_name, MAX_C, 24, 1490, {{peak_name, 24}, {semibase_name, 10}}, SOLVER_STATUS::OPTIMAL);

    auto projection_min_semibase = SinglePbData(PROJECTION_C + " " + semibase_name, MIN_C, 10, 1390, {{peak_name, 14}, {semibase_name, 10}}, SOLVER_STATUS::OPTIMAL);

    auto projection_max_semibase = SinglePbData(PROJECTION_C + " " + semibase_name, MAX_C, 11.694915254, 1490, {{peak_name, 13.83050847}, {semibase_name, 11.694915254}}, SOLVER_STATUS::OPTIMAL);

    std::vector<SinglePbData> pbs_data = {projection_min_peak, projection_max_peak, projection_min_semibase, projection_max_semibase};

    auto expec_output_data = SensitivityOutputData(epsilon, best_ub, pbs_data);

    verify_output_data(output_data, expec_output_data);
}

TEST_F(SensitivityAnalysisTest, InvalidCandidateNameInProjection)
{
    // TODO
}

TEST_F(SensitivityAnalysisTest, FullSensitivityAnalysis)
{
    double epsilon = 10000;
    double best_ub = 1440683382.537683;

    std::map<std::string, int> name_to_id = {{"semibase", 3}, {"peak", 1}, {"pv", 2}, {"battery", 0}, {"transmission_line", 4}};

    std::string data_test_dir = "../data_test";
    std::string json_filename = data_test_dir + "/mps/sensitivity_out_2.json";
    std::shared_ptr<SensitivityWriter> writer = std::make_shared<SensitivityWriter>(json_filename);

    std::string last_master_mps_path = data_test_dir + "/mps/sensitivity.mps";
    std::string solver_name = "CBC";
    SolverFactory factory;
    SolverAbstract::Ptr math_problem = factory.create_solver(solver_name);
    math_problem->init();
    math_problem->read_prob_mps(last_master_mps_path);

    SensitivityInputData input_data = {epsilon, best_ub, name_to_id, math_problem, true, {"semibase", "peak", "pv", "battery", "transmission_line"}};

    SensitivityAnalysis sensitivity_analysis = SensitivityAnalysis(input_data, writer);
    sensitivity_analysis.launch();
}