#include <fstream>
#include <RandomDirGenerator.h>


#include "gtest/gtest.h"

#include "antares-xpansion/lpnamer/problem_modifier/MPSFileProblemProviderAdapter.h"

TEST(MPSFileProblemProviderAdapterTest, ProvideProblemProperly) {
    auto tmp_dir = CreateRandomSubDir(std::filesystem::temp_directory_path());
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
  std::filesystem::current_path(tmp_dir);
    SolverLogManager solver_log_manager_;
    MPSFileProblemProviderAdapter adapter(tmp_dir, "problem_1_1.mps");
  auto problem = adapter.provide_problem("cbc", solver_log_manager_);
  ASSERT_TRUE(problem != nullptr);
}

TEST(MPSFileProblemProviderAdapterTest, ProvideProblemFail)
{
    auto tmp_dir = CreateRandomSubDir(std::filesystem::temp_directory_path());
    SolverLogManager solver_log_manager_;
    MPSFileProblemProviderAdapter adapter(tmp_dir, "problem_1_1.mps");
    ASSERT_THROW(adapter.provide_problem("Invalid", solver_log_manager_), InvalidSolverNameException);
}