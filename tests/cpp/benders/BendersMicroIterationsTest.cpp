#include <algorithm>
#include <antares-xpansion/benders/benders_core/SimulationOptions.h>

#include "LoggerStub.h"
#include "RandomDirGenerator.h"
#include "antares-xpansion/benders/benders_core/BendersBase.h"
#include "antares-xpansion/benders/plugins/BendersPlugin.h"
#include "antares-xpansion/benders/output/JsonWriter.h"
#include "gtest/gtest.h"

class BendersPluginStub : public BendersPlugin
{
public:
    void OnBendersStart(const SubproblemsMapPtr& subproblem_map,
                        const Logger& logger,
                        const BendersBaseOptions& options,
                        const SolverLogManager& solver_log_manager,
                        std::shared_ptr<SolverAbstract> sub_problem_solver) override
    {
    }

    void OnBendersEnd() override
    {
    }

    void OnBendersIterationStart() override
    {
    }

    void OnBendersIterationEnd() override
    {
    }

    void OnBendersSubResolutionStart(const std::shared_ptr<SubproblemWorker>& sub_worker,
                                     std::string sub_name) override
    {
    }

    void OnBendersSubResolutionEnd() override
    {
    }

    void OnBendersMasterResolutionStart() override
    {
    }

    void OnBendersMasterResolutionEnd(std::map<std::string, double>& master_out,
                                      int& num_iter) override
    {
    }

    void OnBendersMicroIterationStart() override
    {
    }

    void OnBendersMicroIterationEnd(std::string sub_name,
                                    bool& added_rows,
                                    std::string solve_time,
                                    int num_master_iter,
                                    int num_micro_iter) override
    {
    }

    bool ShouldRestoreSubproblemBasis() const override
    {
        return true;
    }
};

class BendersMicroIterationsDouble : public BendersBase
{
public:
    explicit BendersMicroIterationsDouble(const BendersBaseOptions& options,
                                          Logger& logger,
                                          std::shared_ptr<Output::OutputWriter> writer,
                                          std::shared_ptr<MathLoggerDriver> mathLoggerDriver):
        BendersBase(options, logger, writer, mathLoggerDriver)
    {
        auto plugin = std::make_shared<BendersPluginStub>();
        SetPlugin(plugin);
    }

    // Expose protected members for testing
    using BendersBase::_data;

    // Override pure virtual methods
    void free() override
    {
    }

    void launch() override
    {
    }

    std::string BendersName() const override
    {
        return "";
    }

    void InitializeProblems() override
    {
    }

    void Run() override
    {
        // OnBendersStart
        benders_plugin_->OnBendersStart(subproblem_map, _logger, _options, solver_log_manager_, nullptr);

        // Simulate one iteration
        benders_plugin_->OnBendersIterationStart();

        // Sub resolution
        std::string sub_name = "sub_dummy";
        benders_plugin_->OnBendersSubResolutionStart(nullptr, sub_name);

        benders_plugin_->OnBendersMicroIterationStart();

        bool added_rows = false;
        std::string solve_time = "0.0";
        int num_master_iter = 1;
        int num_micro_iter = 1;
        benders_plugin_->OnBendersMicroIterationEnd(sub_name, added_rows, solve_time,
                                                     num_master_iter, num_micro_iter);

        benders_plugin_->OnBendersSubResolutionEnd();

        // Master resolution
        benders_plugin_->OnBendersMasterResolutionStart();

        std::map<std::string, double> master_out = {{"x1", 1.0}};
        int iter = 1;
        benders_plugin_->OnBendersMasterResolutionEnd(master_out, iter);

        benders_plugin_->OnBendersIterationEnd();

        // OnBendersEnd
        benders_plugin_->OnBendersEnd();
    }

    void get_master_value() override
    {
    }

    void init_data() override
    {
        BendersBase::init_data();
    }

    void UpdateTrace() override
    {
    }

    void post_run_actions() const override
    {
    }

    void EndWritingInOutputFile() const override
    {
    }

    void SaveCurrentBendersData() override
    {
    }

    void write_basis() const override
    {
    }
};

class BendersMicroIterationsTest : public ::testing::Test
{
public:
    Logger logger;
    std::shared_ptr<MathLoggerDriver> mathLoggerDriver;
    std::shared_ptr<Output::OutputWriter> writer;

protected:
    void SetUp() override
    {
        logger = std::make_shared<Xpansion::Test::LoggerNOOPStub>();
        writer = std::make_shared<Output::JsonWriter>(std::make_shared<Clock>(),
                                                      std::tmpnam(nullptr));
    }

    BendersBaseOptions init_benders_options() const
    {
        SolverBaseOptions solver_options;
        solver_options.LOG_LEVEL = 0;
        solver_options.SLAVE_WEIGHT_VALUE = 1;
        solver_options.OUTPUTROOT = "my_output";
        solver_options.SLAVE_WEIGHT = "CONSTANT";
        solver_options.MASTER_NAME = "master";
        solver_options.STRUCTURE_FILE = "my_structure.txt";
        solver_options.SOLVER_NAME = "COIN";
        solver_options.weights = {};

        BendersBaseOptions options(solver_options);
        options.SEPARATION_PARAM = 0.5;
        options.MASTER_FORMULATION = MasterFormulation::RELAXED;
        options.RESUME = ResumeMode::COLD_START;
        options.NB_CUTS_PER_ITER = false;
        options.TRACE = false;
        options.BOUND_ALPHA = true;
        options.MASTER_SOLUTION_TOLERANCE = 0.1;

        return options;
    }
};

TEST_F(BendersMicroIterationsTest, RunCallsAllPluginMethods)
{
    BendersMicroIterationsDouble benders(init_benders_options(),
                                         logger,
                                         writer,
                                         mathLoggerDriver);
    EXPECT_NO_THROW(benders.Run());
}
