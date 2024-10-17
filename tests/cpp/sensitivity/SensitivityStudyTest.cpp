#include "antares-xpansion/sensitivity/SensitivityFileLogger.h"
#include "antares-xpansion/sensitivity/SensitivityStudy.h"
#include "gtest/gtest.h"
#include "antares-xpansion/multisolver_interface/SolverFactory.h"
#include "antares-xpansion/multisolver_interface/environment.h"

class SensitivityStudyTest : public ::testing::Test {
 public:
  const std::string coin_name = "CBC";
  const std::string xpress_name = "XPRESS";

  std::string data_test_dir;

  const std::string semibase_name = "semibase";
  const std::string peak_name = "peak";
  const std::string pv_name = "pv";
  const std::string battery_name = "battery";
  const std::string transmission_name = "transmission_line";

  std::vector<std::string> candidate_names;

  SolverAbstract::Ptr math_problem;

  std::shared_ptr<SensitivityILogger> logger;
  std::shared_ptr<SensitivityWriter> writer;

  SensitivityInputData input_data;

 protected:
  void SetUp() override {
    data_test_dir = "data_test/sensitivity";

    std::string logger_filename = std::tmpnam(nullptr);
    logger = std::make_shared<SensitivityFileLogger>(logger_filename);

    std::string json_filename = std::tmpnam(nullptr);
    writer = std::make_shared<SensitivityWriter>(json_filename);
  }

  void init_solver(std::string solver_name, std::string last_master_mps_path) {
    SolverFactory factory;
    auto solver_log_manager = SolverLogManager(std::tmpnam(nullptr));
    math_problem = factory.create_solver(solver_name, solver_log_manager);
    math_problem->read_prob_mps(last_master_mps_path);
  }

  std::string prepare_toy_sensitivity_pb(
      bool capex = true, std::vector<std::string> projection_candidates = {}) {
    double epsilon = 100;
    double best_ub = 1390;
    double benders_capex = 12;
    std::map<std::string, double> benders_solution = {{peak_name, 14},
                                                      {semibase_name, 10}};

    std::map<std::string, int> name_to_id = {{peak_name, 0},
                                             {semibase_name, 1}};

    std::map<std::string, std::pair<double, double>> candidates_bounds = {
        {peak_name, {0, 3000}}, {semibase_name, {0, 400}}};

    std::string toy_basis_path = data_test_dir + "/toy_basis.bss";

    input_data = {epsilon,        best_ub,
                  benders_capex,  benders_solution,
                  name_to_id,     nullptr,
                  toy_basis_path, candidates_bounds,
                  capex,          projection_candidates};
    return data_test_dir + "/toy_last_iteration.mps";
  }

  std::string prepare_real_sensitivity_pb(
      bool capex = true, std::vector<std::string> projection_candidates = {}) {
    double epsilon = 10000;
    double best_ub = 1440683382.5376825;
    double benders_capex = 1234;
    std::map<std::string, double> benders_solution = {{semibase_name, 12},
                                                      {peak_name, 34},
                                                      {pv_name, 56},
                                                      {battery_name, 78},
                                                      {transmission_name, 90}};

    std::map<std::string, int> name_to_id = {{semibase_name, 3},
                                             {peak_name, 1},
                                             {pv_name, 2},
                                             {battery_name, 0},
                                             {transmission_name, 4}};

    std::map<std::string, std::pair<double, double>> candidates_bounds = {
        {peak_name, {0, 2000}},
        {semibase_name, {0, 2000}},
        {battery_name, {0, 1000}},
        {pv_name, {0, 1000}},
        {transmission_name, {0, 3200}}};

    std::string real_basis_path = data_test_dir + "/real_basis.bss";

    input_data = {epsilon,         best_ub,
                  benders_capex,   benders_solution,
                  name_to_id,      nullptr,
                  real_basis_path, candidates_bounds,
                  capex,           projection_candidates};
    return data_test_dir + "/real_last_iteration.mps";
  }

  void launch_tests(
      std::string mps_path,
      std::map<std::string, std::vector<SinglePbData>> expec_output_data_map) {
    std::vector<std::string> solvers_name = {coin_name};
    LoadXpress::XpressLoader xpress_loader;
    if (xpress_loader.XpressIsCorrectlyInstalled()) {
      solvers_name.push_back(xpress_name);
    }

    for (auto solver_name : solvers_name) {
      init_solver(solver_name, mps_path);
      input_data.last_master = math_problem;

      auto sensitivity_study = SensitivityStudy(input_data, logger, writer);
      sensitivity_study.launch();

      auto output_data = sensitivity_study.get_output_data();
      verify_output_data(output_data, expec_output_data_map[solver_name]);
    }
  }

  std::stringstream get_single_pb_data_stream(const SinglePbData &pb_data) {
    std::stringstream ss;
    ss << std::endl;
    ss << "The problem data that has found no match is the following :"
       << std::endl;
    ss << "Problem type: " << pb_data.str_pb_type << std::endl;
    ss << "Candidate name: " << pb_data.candidate_name << std::endl;
    ss << "Optimization direction: " << pb_data.opt_dir << std::endl;
    ss << "Objective: " << std::fixed << std::setprecision(16)
       << pb_data.objective << std::endl;
    ss << "System cost: " << pb_data.system_cost << std::endl;
    ss << "Solver status: " << pb_data.solver_status << std::endl;
    ss << "Candidates: " << std::endl;
    for (auto candidate : pb_data.candidates) {
      ss << indent_1 << candidate.first << " " << candidate.second << std::endl;
    }
    ss << std::endl;
    return ss;
  }

  bool areEquals(const Point &left, const Point &right) {
    if (left.size() != right.size()) return false;

    for (auto candidate_it = left.begin(), expec_candidate_it = right.begin();
         candidate_it != left.end(), expec_candidate_it != right.end();
         candidate_it++, expec_candidate_it++) {
      if (candidate_it->first != expec_candidate_it->first) return false;
      if (fabs(candidate_it->second - expec_candidate_it->second) /
              fabs(expec_candidate_it->second) >
          1e-6)
        return false;
    }
    return true;
  }

  bool areEquals(const SinglePbData &left, const SinglePbData &right) {
    return left.pb_type == right.pb_type &&
           left.str_pb_type == right.str_pb_type &&
           left.candidate_name == right.candidate_name &&
           left.opt_dir == right.opt_dir &&
           fabs(left.objective - right.objective) /
                   (fabs(right.objective) + 1e-10) <
               1e-8 &&
           fabs(left.system_cost - right.system_cost) /
                   (fabs(right.system_cost) + 1e-10) <
               1e-8 &&
           left.solver_status == right.solver_status &&
           areEquals(left.candidates, right.candidates);
  }

  void verify_output_data(const std::vector<SinglePbData> &pbs_data,
                          std::vector<SinglePbData> expec_pbs_data) {
    ASSERT_EQ(pbs_data.size(), expec_pbs_data.size());

    for (auto leftMatch : pbs_data) {
      auto rightMatch =
          std::find_if(expec_pbs_data.begin(), expec_pbs_data.end(),
                       [&leftMatch, this](const SinglePbData &data) {
                         return areEquals(leftMatch, data);
                       });
      ASSERT_NE(rightMatch, expec_pbs_data.end())
          << get_single_pb_data_stream(leftMatch).str();
      expec_pbs_data.erase(rightMatch);
    }
    ASSERT_EQ(expec_pbs_data.size(), 0);
  }

  void verify_single_pb_data(const SinglePbData &single_pb_data,
                             const SinglePbData &expec_single_pb_data) {
    EXPECT_EQ(single_pb_data.pb_type, expec_single_pb_data.pb_type);
    EXPECT_EQ(single_pb_data.str_pb_type, expec_single_pb_data.str_pb_type);
    EXPECT_EQ(single_pb_data.candidate_name,
              expec_single_pb_data.candidate_name);
    EXPECT_EQ(single_pb_data.opt_dir, expec_single_pb_data.opt_dir);
    EXPECT_NEAR(single_pb_data.objective, expec_single_pb_data.objective, 1e-6);
    EXPECT_NEAR(single_pb_data.system_cost, expec_single_pb_data.system_cost,
                1e-6);
    EXPECT_EQ(single_pb_data.solver_status, expec_single_pb_data.solver_status);

    verify_candidates(single_pb_data.candidates,
                      expec_single_pb_data.candidates);
  }

  void verify_candidates(const Point &candidates,
                         const Point &expec_candidates) {
    ASSERT_EQ(candidates.size(), expec_candidates.size());

    for (auto candidate_it = candidates.begin(),
              expec_candidate_it = expec_candidates.begin();
         candidate_it != candidates.end(),
              expec_candidate_it != expec_candidates.end();
         candidate_it++, expec_candidate_it++) {
      EXPECT_EQ(candidate_it->first, expec_candidate_it->first);
      EXPECT_NEAR(candidate_it->second, expec_candidate_it->second, 1e-6);
    }
  }
};

class SensitivityLogMock : public SensitivityILogger {
 public:
  SensitivityLogMock() = default;

  std::string displayed_message;
  bool display_message_called = false;

  void display_message(const std::string &str) override {
    displayed_message = str;
    display_message_called = true;
  }
  void log_at_start(const SensitivityInputData &input_data) override {}
  void log_begin_pb_resolution(const SinglePbData &pb_data) override {}
  void log_pb_solution(const SinglePbData &pb_data) override {}
  void log_summary(const SensitivityInputData &input_data,
                   const std::vector<SinglePbData> &pbs_data) override {}
  void log_at_ending() override {}
};

TEST_F(SensitivityStudyTest, EmptyStudy) {
  input_data.capex = false;
  input_data.projection = {};
  auto _logger = std::make_shared<SensitivityLogMock>();
  auto sensitivity_study = SensitivityStudy(input_data, _logger, writer);
  sensitivity_study.launch();

  std::string message = "Study is empty. No capex or projection provided.";
  EXPECT_TRUE(_logger->display_message_called);
  EXPECT_EQ(_logger->displayed_message, message);
}

TEST_F(SensitivityStudyTest, CandidateIgnored) {
  prepare_toy_sensitivity_pb();

  std::string cand_name = "i_am_not_in_the_list";
  input_data.projection = {cand_name};
  input_data.capex = false;

  auto _logger = std::make_shared<SensitivityLogMock>();
  auto sensitivity_study = SensitivityStudy(input_data, _logger, writer);
  sensitivity_study.launch();

  std::string message = "Warning: " + cand_name +
                        " ignored as it has not been found in the list "
                        "of investment candidates";
  EXPECT_TRUE(_logger->display_message_called);
  EXPECT_EQ(_logger->displayed_message, message);
}

TEST_F(SensitivityStudyTest, BasisNonExistent) {
  std::string mps_path = prepare_toy_sensitivity_pb();
  init_solver(coin_name, mps_path);
  input_data.last_master = math_problem;

  std::string mock_basis_path = "wrong_path.something";
  input_data.basis_file_path = mock_basis_path;

  auto _logger = std::make_shared<SensitivityLogMock>();
  auto sensitivity_study = SensitivityStudy(input_data, _logger, writer);
  sensitivity_study.launch();

  std::string message =
      "Warning: Basis file " + mock_basis_path + " could not be read.";
  EXPECT_TRUE(_logger->display_message_called);
  EXPECT_EQ(_logger->displayed_message, message);
}

TEST_F(SensitivityStudyTest, OutputDataInit) {
  std::string mps_path = prepare_toy_sensitivity_pb();
  init_solver(coin_name, mps_path);

  auto sensitivity_study = SensitivityStudy(input_data, logger, writer);
  std::vector<SinglePbData> expec_output_data = {};
  auto output_data = sensitivity_study.get_output_data();

  verify_output_data(output_data, expec_output_data);
}

TEST_F(SensitivityStudyTest, GetCapexSolutions) {
  std::string mps_path = prepare_toy_sensitivity_pb(true, {});

  // In this capex test, the cuts do not constrain too much the overall system
  // cost, which induces differences between solvers.
  auto capex_min_data = SinglePbData(
      SensitivityPbType::CAPEX, CAPEX_C, "", MIN_C, 1040, 1390,
      {{peak_name, 14}, {semibase_name, 10}}, SOLVER_STATUS::OPTIMAL);

  auto capex_max_data = SinglePbData(
      SensitivityPbType::CAPEX, CAPEX_C, "", MAX_C, 1224.745762711, 1490,
      {{peak_name, 17.22033898}, {semibase_name, 11.69491525}},
      SOLVER_STATUS::OPTIMAL);

  std::vector<SinglePbData> pbs_data = {capex_min_data, capex_max_data};

  std::map<std::string, std::vector<SinglePbData>> expec_output_data_map = {
      {coin_name, pbs_data}, {xpress_name, pbs_data}};

  launch_tests(mps_path, expec_output_data_map);
}

TEST_F(SensitivityStudyTest, GetCandidatesProjection) {
  std::string mps_path =
      prepare_toy_sensitivity_pb(false, {peak_name, semibase_name});

  auto projection_min_peak =
      SinglePbData(SensitivityPbType::PROJECTION, PROJECTION_C, peak_name,
                   MIN_C, 13.83050847, 1490,
                   {{peak_name, 13.83050847}, {semibase_name, 11.694915254}},
                   SOLVER_STATUS::OPTIMAL);

  auto projection_max_peak = SinglePbData(
      SensitivityPbType::PROJECTION, PROJECTION_C, peak_name, MAX_C, 24, 1490,
      {{peak_name, 24}, {semibase_name, 10}}, SOLVER_STATUS::OPTIMAL);

  auto projection_min_semibase = SinglePbData(
      SensitivityPbType::PROJECTION, PROJECTION_C, semibase_name, MIN_C, 10,
      1390, {{peak_name, 14}, {semibase_name, 10}}, SOLVER_STATUS::OPTIMAL);

  auto projection_max_semibase_cbc =
      SinglePbData(SensitivityPbType::PROJECTION, PROJECTION_C, semibase_name,
                   MAX_C, 11.694915254, 1490,
                   {{peak_name, 13.83050847}, {semibase_name, 11.694915254}},
                   SOLVER_STATUS::OPTIMAL);

  auto projection_max_semibase_xpress = projection_max_semibase_cbc;
  projection_max_semibase_xpress.candidates[peak_name] = 17.22033898;

  std::vector<SinglePbData> pbs_data_cbc = {
      projection_min_peak, projection_max_peak, projection_min_semibase,
      projection_max_semibase_cbc};

  std::vector<SinglePbData> pbs_data_xpress = {
      projection_min_peak, projection_max_peak, projection_min_semibase,
      projection_max_semibase_xpress};

  std::map<std::string, std::vector<SinglePbData>> expec_output_data_map = {
      {coin_name, pbs_data_cbc}, {xpress_name, pbs_data_xpress}};

  launch_tests(mps_path, expec_output_data_map);
}

TEST_F(SensitivityStudyTest, FullSensitivityTest) {
  std::string mps_path = prepare_real_sensitivity_pb(
      true,
      {semibase_name, peak_name, pv_name, battery_name, transmission_name});

  auto capex_min_data =
      SinglePbData(SensitivityPbType::CAPEX, CAPEX_C, "", MIN_C,
                   299860860.094227, 1440693382.53768253,
                   {{battery_name, 511.01433490373716},
                    {peak_name, 1500},
                    {pv_name, 0},
                    {semibase_name, 1200},
                    {transmission_name, 2800}},
                   SOLVER_STATUS::OPTIMAL);

  auto capex_max_data =
      SinglePbData(SensitivityPbType::CAPEX, CAPEX_C, "", MAX_C,
                   300394980.4731960, 1440693382.53768134,
                   {{battery_name, 519.91634122019002},
                    {peak_name, 1500},
                    {pv_name, 0},
                    {semibase_name, 1200},
                    {transmission_name, 2800}},
                   SOLVER_STATUS::OPTIMAL);

  auto projection_min_semibase =
      SinglePbData(SensitivityPbType::PROJECTION, PROJECTION_C, semibase_name,
                   MIN_C, 1200, 1440693382.5376825,
                   {{battery_name, 519.91634122019059},
                    {peak_name, 1500},
                    {pv_name, 0},
                    {semibase_name, 1200},
                    {transmission_name, 2800}},
                   SOLVER_STATUS::OPTIMAL);

  auto projection_max_semibase =
      SinglePbData(SensitivityPbType::PROJECTION, PROJECTION_C, semibase_name,
                   MAX_C, 1200, 1440693382.5376825,
                   {{battery_name, 519.91634122015228},
                    {peak_name, 1500},
                    {pv_name, 0},
                    {semibase_name, 1200},
                    {transmission_name, 2800}},
                   SOLVER_STATUS::OPTIMAL);

  auto projection_min_peak =
      SinglePbData(SensitivityPbType::PROJECTION, PROJECTION_C, peak_name,
                   MIN_C, 1500, 1440693382.5376825,
                   {{battery_name, 511.01433490344891},
                    {peak_name, 1500},
                    {pv_name, 0},
                    {semibase_name, 1200},
                    {transmission_name, 2800}},
                   SOLVER_STATUS::OPTIMAL);

  auto projection_max_peak =
      SinglePbData(SensitivityPbType::PROJECTION, PROJECTION_C, peak_name,
                   MAX_C, 1500, 1440693382.5376825,
                   {{battery_name, 519.91634122015932},
                    {peak_name, 1500},
                    {pv_name, 0},
                    {semibase_name, 1200},
                    {transmission_name, 2800}},
                   SOLVER_STATUS::OPTIMAL);

  auto projection_min_pv =
      SinglePbData(SensitivityPbType::PROJECTION, PROJECTION_C, pv_name, MIN_C,
                   0, 1440693382.5376825,
                   {{battery_name, 519.9163412201425},
                    {peak_name, 1500},
                    {pv_name, 0},
                    {semibase_name, 1200},
                    {transmission_name, 2800}},
                   SOLVER_STATUS::OPTIMAL);

  auto projection_max_pv =
      SinglePbData(SensitivityPbType::PROJECTION, PROJECTION_C, pv_name, MAX_C,
                   1.2427901705828661, 1440693382.5376825,
                   {{battery_name, 518},
                    {peak_name, 1500},
                    {pv_name, 1.2427901706122959},
                    {semibase_name, 1200},
                    {transmission_name, 2800}},
                   SOLVER_STATUS::OPTIMAL);

  auto projection_min_battery =
      SinglePbData(SensitivityPbType::PROJECTION, PROJECTION_C, battery_name,
                   MIN_C, 511.0132577770553, 1440693382.5376825,
                   {{battery_name, 511.0132577770553},
                    {peak_name, 1500},
                    {pv_name, 0},
                    {semibase_name, 1200},
                    {transmission_name, 2800}},
                   SOLVER_STATUS::OPTIMAL);

  auto projection_min_battery_cbc =
      SinglePbData(SensitivityPbType::PROJECTION, PROJECTION_C, battery_name,
                   MIN_C, 511.01433490356368, 1440693382.5376825,
                   {{battery_name, 511.01433490356368},
                    {peak_name, 1500},
                    {pv_name, 0},
                    {semibase_name, 1200},
                    {transmission_name, 2800}},
                   SOLVER_STATUS::OPTIMAL);

  auto projection_max_battery =
      SinglePbData(SensitivityPbType::PROJECTION, PROJECTION_C, battery_name,
                   MAX_C, 519.91634122009395, 1440693382.5376825,
                   {{battery_name, 519.91634122009395},
                    {peak_name, 1500},
                    {pv_name, 0},
                    {semibase_name, 1200},
                    {transmission_name, 2800}},
                   SOLVER_STATUS::OPTIMAL);

  auto projection_min_transmission =
      SinglePbData(SensitivityPbType::PROJECTION, PROJECTION_C,
                   transmission_name, MIN_C, 2800, 1440693382.5376825,
                   {{battery_name, 519.91634122015114},
                    {peak_name, 1500},
                    {pv_name, 0},
                    {semibase_name, 1200},
                    {transmission_name, 2800}},
                   SOLVER_STATUS::OPTIMAL);

  auto projection_max_transmission =
      SinglePbData(SensitivityPbType::PROJECTION, PROJECTION_C,
                   transmission_name, MAX_C, 2800, 1440693382.5376825,
                   {{battery_name, 519.91634122015114},
                    {peak_name, 1500},
                    {pv_name, 0},
                    {semibase_name, 1200},
                    {transmission_name, 2800}},
                   SOLVER_STATUS::OPTIMAL);

  auto projection_max_transmission_cbc =
      SinglePbData(SensitivityPbType::PROJECTION, PROJECTION_C,
                   transmission_name, MAX_C, 2800, 1440683382.5376825,
                   {{battery_name, 517.99999999999739},
                    {peak_name, 1500},
                    {pv_name, 0},
                    {semibase_name, 1200},
                    {transmission_name, 2800}},
                   SOLVER_STATUS::OPTIMAL);

  std::vector<SinglePbData> pbs_data_cbc = {capex_min_data,
                                            capex_max_data,
                                            projection_min_semibase,
                                            projection_max_semibase,
                                            projection_min_peak,
                                            projection_max_peak,
                                            projection_min_pv,
                                            projection_max_pv,
                                            projection_min_battery_cbc,
                                            projection_max_battery,
                                            projection_min_transmission,
                                            projection_max_transmission_cbc};
  std::vector<SinglePbData> pbs_data_xpress = {capex_min_data,
                                               capex_max_data,
                                               projection_min_semibase,
                                               projection_max_semibase,
                                               projection_min_peak,
                                               projection_max_peak,
                                               projection_min_pv,
                                               projection_max_pv,
                                               projection_min_battery,
                                               projection_max_battery,
                                               projection_min_transmission,
                                               projection_max_transmission};

  std::map<std::string, std::vector<SinglePbData>> expec_output_data_map = {
      {coin_name, pbs_data_cbc}, {xpress_name, pbs_data_xpress}};

  launch_tests(mps_path, expec_output_data_map);
}