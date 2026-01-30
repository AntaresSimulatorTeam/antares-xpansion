#include <gtest/gtest.h>
#include <fstream>
#include <filesystem>
#include "antares-xpansion/benders/benders_core/BendersProblemFromFile.h"
#include "antares-xpansion/multisolver_interface/SolverFactory.h"
#include "antares-xpansion/core/ProblemFormat.h"
#include "RandomDirGenerator.h"
class BendersProblemFromFileTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        temp_dir = CreateRandomSubDir(std::filesystem::temp_directory_path());
        test_mps_file = temp_dir / "test_problem.mps";
        CreateTestMPSFile();
    }
    void TearDown() override
    {
        std::filesystem::remove_all(temp_dir);
    }
    void CreateTestMPSFile()
    {
        std::ofstream mps_file(test_mps_file);
        mps_file << "NAME          TEST\n";
        mps_file << "ROWS\n";
        mps_file << " N  OBJ\n";
        mps_file << " L  C1\n";
        mps_file << "COLUMNS\n";
        mps_file << "    x1        OBJ       1.0\n";
        mps_file << "    x1        C1        1.0\n";
        mps_file << "RHS\n";
        mps_file << "    RHS       C1        10.0\n";
        mps_file << "BOUNDS\n";
        mps_file << " LO BND       x1        0.0\n";
        mps_file << " UP BND       x1        10.0\n";
        mps_file << "ENDATA\n";
        mps_file.close();
    }
    std::filesystem::path temp_dir;
    std::filesystem::path test_mps_file;
};
TEST_F(BendersProblemFromFileTest, ConstructorStoresFilePath)
{
    BendersProblemFromFile problem_provider(test_mps_file);
    EXPECT_EQ(problem_provider.problem_file_path, test_mps_file);
}
TEST_F(BendersProblemFromFileTest, ProvideFilePathReturnsCorrectPath)
{
    BendersProblemFromFile problem_provider(test_mps_file);
    auto returned_path = problem_provider.provide_file_path();
    EXPECT_EQ(returned_path, test_mps_file);
}
TEST_F(BendersProblemFromFileTest, ProvideProblemLoadsFromMPSFile)
{
    BendersProblemFromFile problem_provider(test_mps_file);
    SolverFactory factory;
    auto solver = factory.create_solver("CLP");
    SolverIO solver_io;
    solver_io.configure("CLP", ProblemsFormat::MPS_FILE);
    EXPECT_NO_THROW(problem_provider.provide_problem(solver_io, solver));
    EXPECT_GT(solver->get_ncols(), 0);
    EXPECT_GT(solver->get_nrows(), 0);
}
TEST_F(BendersProblemFromFileTest, ProvideProblemWithNonExistentFile)
{
    std::filesystem::path non_existent_file = temp_dir / "non_existent.mps";
    BendersProblemFromFile problem_provider(non_existent_file);
    SolverFactory factory;
    auto solver = factory.create_solver("CLP");
    SolverIO solver_io;
    solver_io.configure("CLP", ProblemsFormat::MPS_FILE);
    EXPECT_THROW(problem_provider.provide_problem(solver_io, solver), std::exception);
}
TEST_F(BendersProblemFromFileTest, ConstructorWithEmptyPath)
{
    std::filesystem::path empty_path;
    BendersProblemFromFile problem_provider(empty_path);
    EXPECT_TRUE(problem_provider.problem_file_path.empty());
}
TEST_F(BendersProblemFromFileTest, MultipleProvideProblemCalls)
{
    BendersProblemFromFile problem_provider(test_mps_file);
    SolverFactory factory;
    auto solver1 = factory.create_solver("CLP");
    auto solver2 = factory.create_solver("CLP");
    SolverIO solver_io;
    solver_io.configure("CLP", ProblemsFormat::MPS_FILE);
    EXPECT_NO_THROW(problem_provider.provide_problem(solver_io, solver1));
    EXPECT_NO_THROW(problem_provider.provide_problem(solver_io, solver2));
    EXPECT_EQ(solver1->get_ncols(), solver2->get_ncols());
    EXPECT_EQ(solver1->get_nrows(), solver2->get_nrows());
}
