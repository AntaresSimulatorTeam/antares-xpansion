#include <algorithm>
#include <antares-xpansion/benders/benders_core/SimulationOptions.h>

#include "LoggerStub.h"
#include "RandomDirGenerator.h"
#include "antares-xpansion/benders/benders_core/CouplingMapGenerator.h"
#include "antares-xpansion/benders/benders_sequential/BendersSequential.h"
#include "antares-xpansion/benders/output/JsonWriter.h"
#include "antares-xpansion/helpers/ArchiveWriter.h"
#include "antares-xpansion/multisolver_interface/environment.h"
#include "gtest/gtest.h"

class BendersSequentialDouble: public BendersSequential
{
public:
    bool parametrized_stop = false;
    int parametrized_it = 0;
    int parametrized_nsubproblem = 0;
    double parametrized_lb = -1e20;
    double parametrized_ub = +1e20;
    double parametrized_best_ub = +1e20;

    mutable bool _deactivateIntConstraintCall = false;
    mutable bool _reactivateIntConstraintCall = false;
    bool _setDataPreRelaxationCall = false;
    bool _setDataPostRelaxationCall = false;

    explicit BendersSequentialDouble(const BendersBaseOptions& options,
                                     Logger& logger,
                                     std::shared_ptr<Output::OutputWriter> writer,
                                     std::shared_ptr<MathLoggerDriver> mathLoggerDriver):
        BendersSequential(options, logger, writer, mathLoggerDriver)
    {
    }

    void init_data() override
    {
        BendersBase::init_data();
        _data.stop = parametrized_stop;
        _data.nsubproblem = parametrized_nsubproblem;
        _data.lb = parametrized_lb;
        _data.best_ub = parametrized_best_ub;
        _data.it = parametrized_it;
    }

    [[nodiscard]] WorkerMasterPtr get_master() const override
    {
        return BendersSequential::get_master();
    }

    void get_master_value() override
    {
    }

    void BuildCut() override
    {
    }

    void compute_ub() override
    {
        _data.ub = parametrized_ub;
    }

    CurrentIterationData get_data() const
    {
        return _data;
    }

    void write_basis() const override
    {
    }

    void EndWritingInOutputFile() const override
    {
    }

    void UpdateTrace() override
    {
    }

    void post_run_actions() const override
    {
    }

    void SaveCurrentBendersData() override
    {
    }

    void free() override
    {
    }

    SubproblemsMapPtr problems() const
    {
        return GetSubProblemMap();
    }

    void DeactivateIntegrityConstraints() const override
    {
        _deactivateIntConstraintCall = true;
        BendersBase::DeactivateIntegrityConstraints();
    }

    // No override as the base class function is const
    void ActivateIntegrityConstraints() const override
    {
        _reactivateIntConstraintCall = true;
        BendersBase::ActivateIntegrityConstraints();
    }

    void SetDataPreRelaxation() override
    {
        _setDataPreRelaxationCall = true;
        BendersBase::SetDataPreRelaxation();
    }

    void ResetDataPostRelaxation() override
    {
        _setDataPostRelaxationCall = true;
        BendersBase::ResetDataPostRelaxation();
    }

    void HandleInitialMasterRelaxation() override
    {
        BendersBase::HandleInitialMasterRelaxation();
    }

    void set_data(bool stop, int nsubproblem)
    {
        parametrized_stop = stop;
        parametrized_nsubproblem = nsubproblem;
        _data.nsubproblem = nsubproblem;
    }

    void set_bounds(double lb, double best_ub)
    {
        parametrized_lb = lb;
        parametrized_best_ub = best_ub;
    }

    void set_bestx(Point x_out, Point x_in)
    {
        _data.x_out = x_out;
        _data.x_in = x_in;
    }

    void set_invest_bounds(Point min_invest, Point max_invest)
    {
        _data.min_invest = min_invest;
        _data.max_invest = max_invest;
    }

    void set_ub(double ub)
    {
        parametrized_ub = ub;
    }

    void set_it(int it)
    {
        parametrized_it = it;
    }
};

class BendersSequentialTest: public ::testing::Test
{
public:
    Logger logger;
    std::shared_ptr<MathLoggerDriver> mathLoggerDriver;
    std::shared_ptr<Output::OutputWriter> writer;
    const std::filesystem::path data_test_dir = "data_test";
    const std::filesystem::path mps_dir = data_test_dir / "mps";
    std::filesystem::path tmpDir;

protected:
    void SetUp() override
    {
        logger = std::make_shared<Xpansion::Test::LoggerNOOPStub>();
        writer = std::make_shared<Output::JsonWriter>(std::make_shared<Clock>(),
                                                      std::tmpnam(nullptr));
        original_dir = std::filesystem::current_path();
    }

    void TearDown() override
    {
        std::filesystem::current_path(original_dir);
    }

    void copyMasterMps(const std::string& mps_name = "mip_toy_prob.mps")
    {
        tmpDir = CreateRandomSubDir(std::filesystem::temp_directory_path());

        std::filesystem::copy(mps_dir / mps_name,
                              tmpDir,
                              std::filesystem::copy_options::update_existing);
    }

    void copyData()
    {
        std::filesystem::path data_test_dir = "data_test";
        std::filesystem::path data_dir = data_test_dir / "mini_network";
        tmpDir = CreateRandomSubDir(std::filesystem::temp_directory_path());

        std::filesystem::copy(data_dir,
                              tmpDir,
                              std::filesystem::copy_options::recursive
                                | std::filesystem::copy_options::update_existing);
    }

    SolverBaseOptions init_base_options(const std::string& solver,
                                        const std::string& master_name) const
    {
        SolverBaseOptions solver_options;

        solver_options.LOG_LEVEL = 0;
        solver_options.SLAVE_WEIGHT_VALUE = 1;
        solver_options.OUTPUTROOT = "my_output";
        solver_options.SLAVE_WEIGHT = "CONSTANT";
        solver_options.MASTER_NAME = master_name;
        solver_options.STRUCTURE_FILE = "my_structure.txt";
        solver_options.INPUTROOT = tmpDir.string();
        solver_options.SOLVER_NAME = solver;
        solver_options.weights = {};

        return solver_options;
    }

    BendersBaseOptions init_benders_options(MasterFormulation master_formulation,
                                            int max_iter,
                                            double relaxed_gap,
                                            double sep_param,
                                            const std::string& solver,
                                            const std::string& master_name) const
    {
        SolverBaseOptions solver_options(init_base_options(solver, master_name));
        BendersBaseOptions options(solver_options);

        options.MAX_ITERATIONS = max_iter;
        options.ABSOLUTE_GAP = 1e-4;
        options.RELATIVE_GAP = 1e-6;
        options.RELAXED_GAP = relaxed_gap;
        options.TIME_LIMIT = 10;
        options.SEPARATION_PARAM = sep_param;

        options.MASTER_FORMULATION = master_formulation;

        options.RESUME = false;
        options.NB_CUTS_PER_ITER = false;
        options.TRACE = false;
        options.BOUND_ALPHA = true;

        options.CSV_NAME = "my_trace";
        options.LAST_MASTER_MPS = "my_last_iteration";
        options.LAST_MASTER_BASIS = "my_last_basis";

        return options;
    }

    BendersSequentialDouble init_benders_sequential(
      MasterFormulation master_formulation,
      int max_iter,
      double relaxed_gap,
      double sep_param,
      const std::string& solver = "COIN",
      const std::string& master_name = "mip_toy_prob",
      ProblemsFormat format = ProblemsFormat::MPS_FILE)
    {
        BendersBaseOptions options = init_benders_options(master_formulation,
                                                          max_iter,
                                                          relaxed_gap,
                                                          sep_param,
                                                          solver,
                                                          master_name);
        options.PROBLEMS_FORMAT = format;
        return BendersSequentialDouble(options, logger, writer, mathLoggerDriver);
    }

    std::vector<char> get_nb_units_col_types(const BendersSequentialDouble& benders) const
    {
        char col_type;
        std::vector<char> nb_units_col_types;
        for (auto col_id: benders.get_master()->get_id_int_vars())
        {
            benders.get_master()->solver()->get_col_type(&col_type, col_id, col_id);
            nb_units_col_types.push_back(col_type);
        }
        return nb_units_col_types;
    }

    std::filesystem::path original_dir;
};

TEST_F(BendersSequentialTest, MasterNotRelaxedWhenSepSetToOne)
{
    copyMasterMps();
    MasterFormulation master_formulation = MasterFormulation::INTEGER;
    int max_iter = 1;
    double relaxed_gap = 1e-2;
    double sep_param = 1;
    BendersSequentialDouble benders = init_benders_sequential(master_formulation,
    BendersSequentialDouble benders = init_benders_sequential(master_formulation,
                                                              max_iter,
                                                              relaxed_gap,
                                                              sep_param);

    benders.set_data(true, 0);

    benders.launch();
    std::vector<char> nb_units_col_types = get_nb_units_col_types(benders);

    EXPECT_EQ(benders._deactivateIntConstraintCall, false);
    EXPECT_EQ(benders._setDataPreRelaxationCall, false);
    EXPECT_EQ(benders._reactivateIntConstraintCall, false);
    EXPECT_EQ(benders._setDataPostRelaxationCall, false);
}

TEST_F(BendersSequentialTest, MasterRelaxedWhenSepLowerThanOne)
{
    copyMasterMps();
    MasterFormulation master_formulation = MasterFormulation::INTEGER;
    int max_iter = 1;
    double relaxed_gap = 1e-2;
    double sep_param = 0.7;
    BendersSequentialDouble benders = init_benders_sequential(master_formulation,
                                                              max_iter,
                                                              relaxed_gap,
                                                              sep_param);

    benders.set_data(true, 0);
    benders.launch();

    std::vector<char> nb_units_col_types = get_nb_units_col_types(benders);

    EXPECT_EQ(benders._deactivateIntConstraintCall, true);
    EXPECT_EQ(benders._setDataPreRelaxationCall, true);
    EXPECT_EQ(benders._reactivateIntConstraintCall, false);
    EXPECT_EQ(benders._setDataPostRelaxationCall, false);
}

TEST_F(BendersSequentialTest, ReactivateIntConstraintAfterRelaxedGapReached)
{
    copyMasterMps();
    MasterFormulation master_formulation = MasterFormulation::INTEGER;
    int max_iter = 1;
    double relaxed_gap = 1e-2;
    double sep_param = 0.7;
    BendersSequentialDouble benders = init_benders_sequential(master_formulation,
                                                              max_iter,
                                                              relaxed_gap,
                                                              sep_param);

    benders.set_data(false, 0);
    benders.set_bounds(1000, 1001);
    benders.launch();

    std::vector<char> nb_units_col_types = get_nb_units_col_types(benders);

    EXPECT_EQ(benders._deactivateIntConstraintCall, true);
    EXPECT_EQ(benders._setDataPreRelaxationCall, true);
    EXPECT_EQ(benders._reactivateIntConstraintCall, true);
    EXPECT_EQ(benders._setDataPostRelaxationCall, true);
}

TEST_F(BendersSequentialTest,
       MaxIterReachedBeforeRelaxedGapShouldEndRunWithAnIntegerMasterIteration)
{
    copyMasterMps();
    MasterFormulation master_formulation = MasterFormulation::INTEGER;
    int max_iter = 1;
    double relaxed_gap = 1e-5;
    double sep_param = 0.7;
    BendersSequentialDouble benders = init_benders_sequential(master_formulation,
                                                              max_iter,
                                                              relaxed_gap,
                                                              sep_param);

    int expec_benders_run_it = 2;

    benders.set_data(false, 0);
    benders.set_bounds(1000, 1001);
    benders.launch();

    std::vector<char> nb_units_col_types = get_nb_units_col_types(benders);

    EXPECT_EQ(benders._deactivateIntConstraintCall, true);
    EXPECT_EQ(benders._setDataPreRelaxationCall, true);
    EXPECT_EQ(benders._reactivateIntConstraintCall, true);
    EXPECT_EQ(benders._setDataPostRelaxationCall, true);

    EXPECT_EQ(benders.get_data().it, expec_benders_run_it);
}

TEST_F(BendersSequentialTest, CheckDataPostRelaxation)
{
    copyMasterMps();
    MasterFormulation master_formulation = MasterFormulation::INTEGER;
    int max_iter = 1;
    double relaxed_gap = 1e-2;
    double sep_param = 0.7;
    BendersSequentialDouble benders = init_benders_sequential(master_formulation,
                                                              max_iter,
                                                              relaxed_gap,
                                                              sep_param);

    benders.set_data(false, 0);
    benders.set_bounds(1000, 1001);
    benders.launch();

    EXPECT_EQ(benders._deactivateIntConstraintCall, true);
    EXPECT_EQ(benders._setDataPreRelaxationCall, true);
    EXPECT_EQ(benders._reactivateIntConstraintCall, true);
    EXPECT_EQ(benders._setDataPostRelaxationCall, true);

    EXPECT_EQ(benders.get_data().best_ub, 1e+20);
    EXPECT_EQ(benders.get_data().best_it, 0);
}

TEST_F(BendersSequentialTest, CheckInOutDataWhithoutImprovement)
{
    copyMasterMps();
    MasterFormulation master_formulation = MasterFormulation::RELAXED;
    double sep_param = 0.8;
    double relaxed_gap = 1e-2;
    int current_it = 4;
    int max_iter = current_it + 1;

    double init_lb = 1000;
    double init_ub = 1001;
    double current_ub = 2000;

    Point x_out = {{"x1", 1}, {"x2", 2}};
    Point x_in = {{"x1", 3}, {"x2", 6}};
    Point min_invest = {{"x1", 0}, {"x2", 0}};
    Point max_invest = {{"x1", 10}, {"x2", 10}};

    BendersSequentialDouble benders = init_benders_sequential(master_formulation,
                                                              max_iter,
                                                              relaxed_gap,
                                                              sep_param);

    benders.set_data(false, 0);
    benders.set_bounds(init_lb, init_ub);
    benders.set_it(current_it);
    benders.set_bestx(x_out, x_in);
    benders.set_ub(current_ub);
    benders.set_invest_bounds(min_invest, max_invest);

    Point expec_x_cut;
    for (const auto& [coord, val]: x_out)
    {
        expec_x_cut[coord] = sep_param * x_out[coord] + (1 - sep_param) * x_in[coord];
    }

    benders.launch();

    EXPECT_EQ(benders._deactivateIntConstraintCall, true);
    EXPECT_EQ(benders._setDataPreRelaxationCall, false);
    EXPECT_EQ(benders._reactivateIntConstraintCall, false);
    EXPECT_EQ(benders._setDataPostRelaxationCall, false);

    EXPECT_EQ(benders.get_data().x_out, x_out);
    EXPECT_EQ(benders.get_data().x_cut, expec_x_cut);
    EXPECT_EQ(benders.get_data().x_in, x_in);
    EXPECT_EQ(benders.get_data().best_ub, init_ub);
    EXPECT_EQ(benders.get_data().best_it, 0);
}

TEST_F(BendersSequentialTest, CheckInOutDataWhenImprovement)
{
    copyMasterMps();
    MasterFormulation master_formulation = MasterFormulation::RELAXED;
    double relaxed_gap = 1e-2;
    double sep_param = 0.8;
    int current_it = 4;
    int max_iter = current_it + 1;

    double init_lb = 1000;
    double init_ub = 1001;
    double current_ub = 1000.5;

    Point x_out = {{"x1", 1}, {"x2", 2}};
    Point x_in = {{"x1", 3}, {"x2", 6}};

    Point min_invest = {{"x1", 0}, {"x2", 0}};
    Point max_invest = {{"x1", 10}, {"x2", 10}};

    BendersSequentialDouble benders = init_benders_sequential(master_formulation,
                                                              max_iter,
                                                              relaxed_gap,
                                                              sep_param);

    benders.set_data(false, 0);
    benders.set_bounds(init_lb, init_ub);
    benders.set_it(current_it);
    benders.set_bestx(x_out, x_in);
    benders.set_ub(current_ub);
    benders.set_invest_bounds(min_invest, max_invest);

    Point expec_x_cut;
    for (const auto& [coord, val]: x_out)
    {
        expec_x_cut[coord] = sep_param * x_out[coord] + (1 - sep_param) * x_in[coord];
    }

    benders.launch();

    EXPECT_EQ(benders._deactivateIntConstraintCall, true);
    EXPECT_EQ(benders._setDataPreRelaxationCall, false);
    EXPECT_EQ(benders._reactivateIntConstraintCall, false);
    EXPECT_EQ(benders._setDataPostRelaxationCall, false);

    EXPECT_EQ(benders.get_data().x_out, x_out);
    EXPECT_EQ(benders.get_data().x_cut, expec_x_cut);
    EXPECT_EQ(benders.get_data().x_in, expec_x_cut);
    EXPECT_EQ(benders.get_data().best_ub, current_ub);
    EXPECT_EQ(benders.get_data().best_it, current_it + 1);
}

TEST_F(BendersSequentialTest, IntegersAndBinariesAreRelaxedAfterDeactivation)
{
    copyMasterMps("mip_toy_prob_binary.mps");
    BendersSequentialDouble benders = init_benders_sequential(MasterFormulation::INTEGER,
                                                              100,
                                                              1e-4,
                                                              0.5,
                                                              "COIN",
                                                              "mip_toy_prob_binary");

    // Build only the problems to access the master without running the whole algorithm
    benders.InitializeProblems();

    // Before deactivation: the tracked ids are integer or binary columns
    std::vector<char> types_before = get_nb_units_col_types(benders);
    ASSERT_GT(types_before.size(), 0u);
    for (char t: types_before)
    {
        EXPECT_TRUE(t == 'I' || t == 'B');
    }

    // Deactivate integrity constraints and ensure these variables become continuous
    benders.HandleInitialMasterRelaxation();
    std::vector<char> types_after = get_nb_units_col_types(benders);
    ASSERT_EQ(types_after.size(), types_before.size());
    for (char t: types_after)
    {
        EXPECT_EQ(t, 'C');
    }
}

TEST_F(BendersSequentialTest, BinariesAreCorrectlyRelaxedWhenMasterFormulationIsRelaxed)
{
    copyMasterMps("mip_toy_prob_binary.mps");
    BendersSequentialDouble benders = init_benders_sequential(MasterFormulation::RELAXED,
                                                              100,
                                                              1e-4,
                                                              0.5,
                                                              "COIN",
                                                              "mip_toy_prob_binary");

    // Build only the problems to access the master without running the whole algorithm
    benders.InitializeProblems();

    // Before deactivation: the tracked ids are integer or binary columns
    std::vector<char> types_before = get_nb_units_col_types(benders);
    ASSERT_GT(types_before.size(), 0u);
    for (char t: types_before)
    {
        EXPECT_TRUE(t == 'I' || t == 'B');
    }

    // Deactivate integrity constraints and ensure these variables become continuous
    benders.HandleInitialMasterRelaxation();
    std::vector<char> types_after = get_nb_units_col_types(benders);
    ASSERT_EQ(types_after.size(), types_before.size());
    for (char t: types_after)
    {
        EXPECT_EQ(t, 'C');
    }
}

auto solvers()
{
    std::vector<std::string> solvers_name;
    solvers_name.push_back("COIN");
    if (LoadXpress::XpressLoader xpressLoader; xpressLoader.XpressIsCorrectlyInstalled())
    {
        solvers_name.push_back("XPRESS");
    }
    return solvers_name;
}

class BendersSequentialTestBySolver: public BendersSequentialTest,
                                     public ::testing::WithParamInterface<std::string>
{
};

TEST_P(BendersSequentialTestBySolver, CreateMasterProblemProperly)
{
    copyMasterMps();
    BendersSequentialDouble benders = init_benders_sequential(MasterFormulation::RELAXED,
                                                              100,
                                                              1e-4,
                                                              1e-6,
                                                              GetParam());
    benders.InitializeProblems();

    // Assert that the master problem has been created properly
    EXPECT_TRUE(benders.get_master());
}

// Problems
TEST_P(BendersSequentialTestBySolver, CreateProblemsProperly)
{
    copyData();
    std::filesystem::current_path(tmpDir);

    SimulationOptions options;
    options.read(tmpDir / "options_default.json");
    options.SOLVER_NAME = GetParam();
    BendersSequentialDouble benders(options.get_benders_options(),
                                    logger,
                                    writer,
                                    mathLoggerDriver);
    auto coupling_map = CouplingMapGenerator::BuildInput(options.STRUCTURE_FILE,
                                                         logger.get(),
                                                         "Benders");
    benders.set_input_map(coupling_map);
    benders.InitializeProblems();
    auto&& problems = benders.problems();

    // Assert that the master problem has been created properly
    EXPECT_TRUE(problems.size() == 2);
}

class BendersSequentialTestSolverAndFormat
    : public BendersSequentialTest,
      public ::testing::WithParamInterface<std::tuple<std::string, ProblemsFormat>>
{
};

// Master svf
TEST_P(BendersSequentialTestBySolver, CreateMasterProblemProperlyWhenRestore)
{
    copyMasterMps();

    SolverFactory factory;
    auto&& solver = factory.create_solver(GetParam() == "COIN" ? "CBC" : GetParam());
    solver->read_prob_mps(tmpDir / "mip_toy_prob.mps");
    std::filesystem::remove(tmpDir / "mip_toy_prob.mps");
    solver->save_prob(tmpDir / "mip_toy_prob");

    BendersSequentialDouble benders = init_benders_sequential(MasterFormulation::RELAXED,
                                                              100,
                                                              1e-4,
                                                              1e-6,
                                                              GetParam(),
                                                              "mip_toy_prob",
                                                              ProblemsFormat::OPTIMIZED);
    benders.InitializeProblems();

    // Assert that the master problem has been created properly
    EXPECT_TRUE(benders.get_master());
}

void updateStructureFile(const std::string& structure_file_path, const std::string& solver)
{
    if (solver == "XPRESS")
    {
        auto struct_file = std::ifstream(structure_file_path);
        auto replaced_text = std::ostringstream();
        std::regex mps_ext(".mps");
        std::string line;
        while (std::getline(struct_file, line))
        {
            replaced_text << std::regex_replace(line, mps_ext, ".svf") << "\n";
        }
        struct_file.close();
        auto new_struct_file = std::ofstream(structure_file_path);
        new_struct_file << replaced_text.str();
        new_struct_file.close();
    }
}

TEST_P(BendersSequentialTestBySolver, CreateProblemsProperlyWhenRestore)
{
    copyData();
    std::filesystem::current_path(tmpDir);

    SimulationOptions options;
    options.read(tmpDir / "options_default.json");

    SolverFactory factory;
    auto&& solver = factory.create_solver(GetParam() == "COIN" ? "CBC" : GetParam());
    for (std::string problem: {"SP1", "SP2"})
    {
        solver->read_prob_mps(tmpDir / (problem + ".mps"));
        std::filesystem::remove(tmpDir / (problem + ".mps"));
        solver->save_prob(tmpDir / problem);
    }
    updateStructureFile(options.STRUCTURE_FILE, GetParam());

    options.SOLVER_NAME = GetParam();
    BendersSequentialDouble benders(options.get_benders_options(),
                                    logger,
                                    writer,
                                    mathLoggerDriver);
    auto coupling_map = CouplingMapGenerator::BuildInput(options.STRUCTURE_FILE,
                                                         logger.get(),
                                                         "Benders");
    benders.set_input_map(coupling_map);
    benders.InitializeProblems();
    auto&& problems = benders.problems();

    // Assert that the master problem has been created properly
    EXPECT_TRUE(problems.size() == 2);
    if (GetParam() == "COIN")
    {
        EXPECT_TRUE(problems["SP1.mps"]);
        EXPECT_TRUE(problems["SP2.mps"]);
    }
    else
    {
        EXPECT_TRUE(problems["SP1.svf"]);
        EXPECT_TRUE(problems["SP2.svf"]);
    }
}

INSTANTIATE_TEST_SUITE_P(Solvers, BendersSequentialTestBySolver, ::testing::ValuesIn(solvers()));

// Problems svf
