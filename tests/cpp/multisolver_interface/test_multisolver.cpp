#include "antares-xpansion/multisolver_interface/SolverFactory.h"
#include "gtest/gtest.h"

struct ILoggerMock final: public ILoggerXpansion
{
    void display_message(const std::string& str) override
    {
    }

    void display_message(const std::string& msg,
                         LogUtils::LOGLEVEL level,
                         const std::string& context) override
    {
    }

    void PrintIterationSeparatorBegin() override
    {
    }

    void PrintIterationSeparatorEnd() override
    {
    }

    ~ILoggerMock() override = default;
};

TEST(SolverLoader, GetAvailableSolvers)
{
    auto logger = std::make_shared<ILoggerMock>();
    auto solvers = SolverLoader::GetAvailableSolvers(logger);
    EXPECT_EQ(solvers.size(), 3);
}

TEST(SolverLoader, GetSupportedSolvers)
{
    auto solvers = SolverLoader::GetSupportedSolvers();
    EXPECT_EQ(solvers.size(), 3);
}

class CreateFixture: public ::testing::TestWithParam<std::string>
{
public:
    void SetUp() override
    {
    }

    void TearDown() override
    {
    }

    SolverFactory solver_factory;
};

TEST_P(CreateFixture, create_solver)
{
    auto solver = solver_factory.create_solver(GetParam());
    EXPECT_EQ(solver->get_solver_name(), GetParam());
}

INSTANTIATE_TEST_SUITE_P(Valid, CreateFixture, testing::Values("CLP", "CBC", "XPRESS"));

TEST(SolverFactory, create_invalid)
{
    SolverFactory solver_factory;
    EXPECT_THROW(
      (void)solver_factory.create_solver("solver-does-not-exist"),
      InvalidSolverNameException);
}
