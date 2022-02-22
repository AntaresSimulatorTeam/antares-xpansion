#include "gtest/gtest.h"
#include "SensitivityAnalysis.h"

class SensitivityAnalysisTest : public ::testing::Test
{
public:
    std::string data_test_dir;

    double epsilon;
    double best_ub;

    const std::string semibase_name = "semibase";
    const std::string peak_name = "peak";
    const std::string pv_name = "pv";
    const std::string battery_name = "battery";
    const std::string transmission_name = "transmission_line";

    std::vector<std::string> candidate_names;

    SolverAbstract::Ptr math_problem;

    std::shared_ptr<SensitivityWriter> writer;

    SensitivityInputData input_data;

protected:
    void SetUp() override
    {
#if defined(WIN32) || defined(_WIN32)
        data_test_dir = "../../data_test";
#else
        data_test_dir = "../data_test";
#endif
        data_test_dir += "/sensitivity";

        std::string json_filename = std::tmpnam(nullptr);
        writer = std::make_shared<SensitivityWriter>(json_filename);

        std::string solver_name = "CBC";
        SolverFactory factory;
        math_problem = factory.create_solver(solver_name);
        math_problem->init();
    }

    void prepare_toy_sensitivity_pb()
    {
        epsilon = 100;
        best_ub = 1390;

        candidate_names = {peak_name, semibase_name};

        std::map<std::string, int> name_to_id = {{peak_name, 0}, {semibase_name, 1}};

        std::string last_master_mps_path = data_test_dir + "/toy_last_iteration.mps";
        math_problem->read_prob_mps(last_master_mps_path);

        bool capex = true;

        input_data = {epsilon, best_ub, name_to_id, math_problem, capex, candidate_names};
    }

    void prepare_real_sensitivity_pb()
    {
        epsilon = 10000;
        best_ub = 1440683382.5376825;

        candidate_names = {semibase_name, peak_name, pv_name, battery_name, transmission_name};

        std::map<std::string, int> name_to_id = {{semibase_name, 3}, {peak_name, 1}, {pv_name, 2}, {battery_name, 0}, {transmission_name, 4}};

        std::string last_master_mps_path = data_test_dir + "/real_last_iteration.mps";
        math_problem->read_prob_mps(last_master_mps_path);

        bool capex = true;

        input_data = {epsilon, best_ub, name_to_id, math_problem, capex, candidate_names};
    }

    std::string get_projection_pb_type(const std::string & candidate_name)
    {
        return PROJECTION_C + " " + candidate_name;
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
    prepare_toy_sensitivity_pb();

    auto sensitivity_analysis = SensitivityAnalysis(input_data, writer);
    auto expec_output_data = SensitivityOutputData(epsilon, best_ub);
    auto output_data = sensitivity_analysis.get_output_data();

    verify_output_data(output_data, expec_output_data);
}

TEST_F(SensitivityAnalysisTest, GetCapexSolutions)
{
    prepare_toy_sensitivity_pb();

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
    prepare_toy_sensitivity_pb();

    input_data.capex = false;
    input_data.projection = candidate_names;
    auto sensitivity_analysis = SensitivityAnalysis(input_data, writer);
    sensitivity_analysis.launch();

    auto output_data = sensitivity_analysis.get_output_data();

    auto projection_min_peak = SinglePbData(get_projection_pb_type(peak_name), MIN_C, 13.83050847, 1490, {{peak_name, 13.83050847}, {semibase_name, 11.694915254}}, SOLVER_STATUS::OPTIMAL);

    auto projection_max_peak = SinglePbData(get_projection_pb_type(peak_name), MAX_C, 24, 1490, {{peak_name, 24}, {semibase_name, 10}}, SOLVER_STATUS::OPTIMAL);

    auto projection_min_semibase = SinglePbData(get_projection_pb_type(semibase_name), MIN_C, 10, 1390, {{peak_name, 14}, {semibase_name, 10}}, SOLVER_STATUS::OPTIMAL);

    auto projection_max_semibase = SinglePbData(get_projection_pb_type(semibase_name), MAX_C, 11.694915254, 1490, {{peak_name, 13.83050847}, {semibase_name, 11.694915254}}, SOLVER_STATUS::OPTIMAL);

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
    prepare_real_sensitivity_pb();

    input_data.capex = true;
    input_data.projection = candidate_names;
    SensitivityAnalysis sensitivity_analysis = SensitivityAnalysis(input_data, writer);
    sensitivity_analysis.launch();

    auto output_data = sensitivity_analysis.get_output_data();

    auto capex_min_data = SinglePbData(CAPEX_C, MIN_C, 299860860.09420657, 1440693382.5376828, {{battery_name, 511.01433490373716}, {peak_name, 1500}, {pv_name, 0}, {semibase_name, 1200}, {transmission_name, 2800}}, SOLVER_STATUS::OPTIMAL);

    auto capex_max_data = SinglePbData(CAPEX_C, MAX_C, 300394980.47320199, 1440693382.5376825, {{battery_name, 519.91634122019002}, {peak_name, 1500}, {pv_name, 0}, {semibase_name, 1200}, {transmission_name, 2800}}, SOLVER_STATUS::OPTIMAL);

    auto projection_min_semibase = SinglePbData(get_projection_pb_type(semibase_name), MIN_C, 1200, 1440693382.5376825, {{battery_name, 519.91634122019059}, {peak_name, 1500}, {pv_name, 0}, {semibase_name, 1200}, {transmission_name, 2800}}, SOLVER_STATUS::OPTIMAL);

    auto projection_max_semibase = SinglePbData(get_projection_pb_type(semibase_name), MAX_C, 1200, 1440693382.5376825, {{battery_name, 519.91634122015228}, {peak_name, 1500}, {pv_name, 0}, {semibase_name, 1200}, {transmission_name, 2800}}, SOLVER_STATUS::OPTIMAL);

    auto projection_min_peak = SinglePbData(get_projection_pb_type(peak_name), MIN_C, 1500, 1440693382.5376825, {{battery_name, 511.01433490344891}, {peak_name, 1500}, {pv_name, 0}, {semibase_name, 1200}, {transmission_name, 2800}}, SOLVER_STATUS::OPTIMAL);

    auto projection_max_peak = SinglePbData(get_projection_pb_type(peak_name), MAX_C, 1500, 1440693382.5376825, {{battery_name, 519.91634122015932}, {peak_name, 1500}, {pv_name, 0}, {semibase_name, 1200}, {transmission_name, 2800}}, SOLVER_STATUS::OPTIMAL);

    auto projection_min_pv = SinglePbData(get_projection_pb_type(pv_name), MIN_C, 0, 1440693382.5376825, {{battery_name, 519.9163412201425}, {peak_name, 1500}, {pv_name, 0}, {semibase_name, 1200}, {transmission_name, 2800}}, SOLVER_STATUS::OPTIMAL);

    auto projection_max_pv = SinglePbData(get_projection_pb_type(pv_name), MAX_C, 1.2427901705828661, 1440693382.5376825, {{battery_name, 517.99999999999875}, {peak_name, 1500}, {pv_name, 1.2427901706122959}, {semibase_name, 1200}, {transmission_name, 2800}}, SOLVER_STATUS::OPTIMAL);

    auto projection_min_battery = SinglePbData(get_projection_pb_type(battery_name), MIN_C, 511.01433490356368, 1440693382.5376825, {{battery_name, 511.01433490356368}, {peak_name, 1500}, {pv_name, 0}, {semibase_name, 1200}, {transmission_name, 2800}}, SOLVER_STATUS::OPTIMAL);

    auto projection_max_battery = SinglePbData(get_projection_pb_type(battery_name), MAX_C, 519.91634122009395, 1440693382.5376825, {{battery_name, 519.91634122009395}, {peak_name, 1500}, {pv_name, 0}, {semibase_name, 1200}, {transmission_name, 2800}}, SOLVER_STATUS::OPTIMAL);

    auto projection_min_transmission = SinglePbData(get_projection_pb_type(transmission_name), MIN_C, 2800, 1440693382.5376825, {{battery_name, 519.91634122015114}, {peak_name, 1500}, {pv_name, 0}, {semibase_name, 1200}, {transmission_name, 2800}}, SOLVER_STATUS::OPTIMAL);

    auto projection_max_transmission = SinglePbData(get_projection_pb_type(transmission_name), MAX_C, 2800, 1440683382.5376825, {{battery_name, 517.99999999999739}, {peak_name, 1500}, {pv_name, 0}, {semibase_name, 1200}, {transmission_name, 2800}}, SOLVER_STATUS::OPTIMAL);

    std::vector<SinglePbData> pbs_data = {capex_min_data, capex_max_data, projection_min_semibase, projection_max_semibase, projection_min_peak, projection_max_peak, projection_min_pv, projection_max_pv, projection_min_battery, projection_max_battery, projection_min_transmission, projection_max_transmission};
    auto expec_output_data = SensitivityOutputData(epsilon, best_ub, pbs_data);

    verify_output_data(output_data, expec_output_data);
}