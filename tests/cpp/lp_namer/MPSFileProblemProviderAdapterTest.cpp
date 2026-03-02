#include <RandomDirGenerator.h>
#include <fstream>

#include "antares-xpansion/lpnamer/problem_modifier/MPSFileProblemProviderAdapter.h"
#include "gtest/gtest.h"

class FixtureMPSFileProblemProviderAdapter: public ::testing::Test
{
    void SetUp() override
    {
        previous_path = std::filesystem::current_path();
        tmp_dir = CreateRandomSubDir(std::filesystem::temp_directory_path());
        std::filesystem::current_path(tmp_dir);
    }

    void TearDown() override
    {
        std::filesystem::current_path(previous_path);
    }

    std::filesystem::path previous_path;

public:
    std::filesystem::path tmp_dir;
};

TEST_F(FixtureMPSFileProblemProviderAdapter, ProvideProblemProperly)
{
    std::ofstream file(tmp_dir / "problem_1_1.mps");
    file << R"(NAME          MASTER  FREE
ROWS
 N  OBJROW
 L  C1
COLUMNS
    X1        OBJROW       3.0   C1        2.0
RHS
    RHS      C1        8.0
BOUNDS
 UP BOUND      X1        4.0
ENDATA)";
    file.close();
    SolverLogManager solver_log_manager_;
    MPSFileProblemProviderAdapter adapter(tmp_dir, "problem_1_1.mps");
    auto problem = adapter.provide_problem("cbc", solver_log_manager_);
    ASSERT_TRUE(problem != nullptr);
}

TEST_F(FixtureMPSFileProblemProviderAdapter, ProvideProblemFail)
{
    SolverLogManager solver_log_manager_;
    MPSFileProblemProviderAdapter adapter(tmp_dir, "problem_1_1.mps");
    ASSERT_THROW((void)adapter.provide_problem("Invalid", solver_log_manager_),
                 InvalidSolverNameException);
}
