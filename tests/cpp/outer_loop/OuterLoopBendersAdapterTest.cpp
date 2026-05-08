#include "antares-xpansion/benders/factories/LoggerFactories.h"
#include "antares-xpansion/benders/factories/WriterFactories.h"
#include "antares-xpansion/benders/outer_loop/OuterLoopBendersAdapter.h"
#include "gtest/gtest.h"

using namespace Outerloop;

namespace
{

class RecordingWriter: public Output::OutputWriter
{
public:
    int iteration_writes = 0;
    int solution_writes = 0;
    int dumps = 0;
    Output::Iteration last_iteration{};
    size_t last_iteration_num = 0;
    Output::SolutionData last_solution{};

    void dump() override
    {
        ++dumps;
    }
    void write_solution(const Output::SolutionData& solution) override
    {
        ++solution_writes;
        last_solution = solution;
    }
    void write_iteration(const Output::Iteration& iteration_data,
                         const size_t iteration_num) override
    {
        ++iteration_writes;
        last_iteration = iteration_data;
        last_iteration_num = iteration_num;
    }

    void update_solution(const Output::SolutionData&) override
    {
    }
    void initialize() override
    {
    }
    void end_writing(const Output::IterationsData&) override
    {
    }
    void write_solver_name(const std::string&) override
    {
    }
    void write_master_name(const std::string&) override
    {
    }
    void write_log_level(const int) override
    {
    }
    void updateBeginTime() override
    {
    }
    void updateEndTime() override
    {
    }
    void write_nbweeks(const int) override
    {
    }
    void write_duration(const double) override
    {
    }
    std::string solution_status() const override
    {
        return {};
    }
    void WriteProblem(const Output::ProblemData&) override
    {
    }
    void WriteProblemFormat(const std::string) override
    {
    }
};

class FakeBenders: public BendersBase
{
public:
    int launch_count = 0;
    int free_count = 0;
    int init_problems_count = 0;

    using OnLaunch = std::function<void(FakeBenders&)>;
    OnLaunch on_launch;

    FakeBenders(BendersBaseOptions options,
                Logger logger,
                std::shared_ptr<Output::OutputWriter> writer,
                std::shared_ptr<MathLoggerDriver> math_logger):
        BendersBase(std::move(options),
                    std::move(logger),
                    std::move(writer),
                    std::move(math_logger))
    {
    }

    void launch() override
    {
        ++launch_count;
        if (on_launch)
        {
            on_launch(*this);
        }
    }

    void free() override
    {
        ++free_count;
    }

    void InitializeProblems() override
    {
        ++init_problems_count;
    }

    std::string BendersName() const override
    {
        return "fake";
    }

    void SetLastWorkerMasterDataValid(bool valid)
    {
        relevantIterationData_.last._valid = valid;
    }

    void SetCriteriaPerIteration(std::vector<std::vector<double>> v)
    {
        criteria_vector_for_each_iteration_ = std::move(v);
    }

    void SetBestIt(int best_it)
    {
        _data.best_it = best_it;
    }

    void SetCriteriaCurrentIterationData(const CriteriaCurrentIterationData& d)
    {
        _data.criteria_current_iteration_data = d;
    }

protected:
    void Run() override
    {
    }
};

std::shared_ptr<FakeBenders> MakeFake(std::shared_ptr<Output::OutputWriter> writer)
{
    BendersBaseOptions options;
    options.OUTPUTROOT = std::filesystem::temp_directory_path().string();
    options.CSV_NAME = "fake";
    auto math_log = MathLoggerFactory::get_void_logger();
    auto logger = build_void_logger();
    return std::make_shared<FakeBenders>(options, logger, writer, math_log);
}

} // namespace

TEST(OuterLoopBendersAdapterTest, IncrementBendersRunNumberAdvancesAdapterCounter)
{
    auto writer = std::make_shared<RecordingWriter>();
    auto benders = MakeFake(writer);
    OuterLoopBendersAdapter adapter(benders);

    EXPECT_EQ(adapter.GetBendersRunNumber(), 0);
    adapter.IncrementBendersRunNumber();
    adapter.IncrementBendersRunNumber();
    EXPECT_EQ(adapter.GetBendersRunNumber(), 2);
}

TEST(OuterLoopBendersAdapterTest, InitOuterLoopDataSetsLambdaParameters)
{
    auto writer = std::make_shared<RecordingWriter>();
    auto benders = MakeFake(writer);
    OuterLoopBendersAdapter adapter(benders);

    adapter.InitOuterLoopData(2.5, 1.0, 4.0);

    auto lambdas = adapter.GetLambdaParameters();
    EXPECT_DOUBLE_EQ(lambdas.lambda, 2.5);
    EXPECT_DOUBLE_EQ(lambdas.lambda_min, 1.0);
    EXPECT_DOUBLE_EQ(lambdas.lambda_max, 4.0);
}

TEST(OuterLoopBendersAdapterTest, SetBilevelBestubReflectedEverywhere)
{
    auto writer = std::make_shared<RecordingWriter>();
    auto benders = MakeFake(writer);
    OuterLoopBendersAdapter adapter(benders);

    adapter.SetBilevelBestub(123.5);

    EXPECT_DOUBLE_EQ(adapter.GetBilevelBestub(), 123.5);
    EXPECT_DOUBLE_EQ(adapter.GetOuterLoopData().outer_loop_bilevel_best_ub, 123.5);
    EXPECT_DOUBLE_EQ(adapter.GetCurrentIterationData()
                       .criteria_current_iteration_data.outer_loop_bilevel_best_ub,
                     123.5);
}

TEST(OuterLoopBendersAdapterTest, LaunchPreservesAdapterOwnedFields)
{
    auto writer = std::make_shared<RecordingWriter>();
    auto benders = MakeFake(writer);
    OuterLoopBendersAdapter adapter(benders);

    adapter.InitOuterLoopData(2.5, 1.0, 4.0);
    adapter.SetBilevelBestub(77.0);
    adapter.IncrementBendersRunNumber();
    adapter.IncrementBendersRunNumber();

    // Simulate Benders writing competing values during a run.
    benders->on_launch = [](FakeBenders& b) {
        CriteriaCurrentIterationData competing;
        competing.lambda = 99.0;
        competing.lambda_min = 99.0;
        competing.lambda_max = 99.0;
        competing.outer_loop_bilevel_best_ub = 99.0;
        competing.benders_num_run = 99;
        b.SetCriteriaCurrentIterationData(competing);
    };

    adapter.Launch();

    EXPECT_EQ(benders->launch_count, 1);
    auto lambdas = adapter.GetLambdaParameters();
    EXPECT_DOUBLE_EQ(lambdas.lambda, 2.5);
    EXPECT_DOUBLE_EQ(lambdas.lambda_min, 1.0);
    EXPECT_DOUBLE_EQ(lambdas.lambda_max, 4.0);
    EXPECT_DOUBLE_EQ(adapter.GetBilevelBestub(), 77.0);
    EXPECT_EQ(adapter.GetBendersRunNumber(), 2);
}

TEST(OuterLoopBendersAdapterTest, SaveCurrentOuterLoopIterationIsNoOpWhenInvalid)
{
    auto writer = std::make_shared<RecordingWriter>();
    auto benders = MakeFake(writer);
    benders->SetLastWorkerMasterDataValid(false);
    OuterLoopBendersAdapter adapter(benders);

    adapter.SaveCurrentOuterLoopIterationInOutputFile();

    EXPECT_EQ(writer->iteration_writes, 0);
    EXPECT_EQ(writer->dumps, 0);
}

TEST(OuterLoopBendersAdapterTest, SaveCurrentOuterLoopIterationWritesWhenValid)
{
    auto writer = std::make_shared<RecordingWriter>();
    auto benders = MakeFake(writer);
    benders->SetLastWorkerMasterDataValid(true);
    OuterLoopBendersAdapter adapter(benders);
    adapter.IncrementBendersRunNumber();
    adapter.IncrementBendersRunNumber();
    adapter.IncrementBendersRunNumber();

    adapter.SaveCurrentOuterLoopIterationInOutputFile();

    EXPECT_EQ(writer->iteration_writes, 1);
    EXPECT_EQ(writer->dumps, 1);
    EXPECT_EQ(writer->last_iteration_num, 3u);
}

TEST(OuterLoopBendersAdapterTest, GetOuterLoopCriterionAtBestBendersUsesLastRefresh)
{
    auto writer = std::make_shared<RecordingWriter>();
    auto benders = MakeFake(writer);
    benders->SetCriteriaPerIteration({{1.0, 2.0}, {3.0, 4.0}, {5.0, 6.0}});
    benders->SetBestIt(2);
    OuterLoopBendersAdapter adapter(benders);

    adapter.Launch();

    auto criterion = adapter.GetOuterLoopCriterionAtBestBenders();
    ASSERT_EQ(criterion.size(), 2u);
    EXPECT_DOUBLE_EQ(criterion[0], 3.0);
    EXPECT_DOUBLE_EQ(criterion[1], 4.0);
}
