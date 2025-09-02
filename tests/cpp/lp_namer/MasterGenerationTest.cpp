#include <antares-xpansion/lpnamer/problem_modifier/FileWriter.h>
#include <antares-xpansion/lpnamer/problem_modifier/StructureGeneration.h>
#include <gtest/gtest.h>

#include "antares-xpansion/lpnamer/problem_modifier/MasterGeneration.h"
#include "antares-xpansion/multisolver_interface/Solver.h"

// Noop ProblemGenerationLogger
class NoopProblemGenerationLogger: public ProblemGenerationLog::ProblemGenerationLogger
{
public:
    using ProblemGenerationLogger::ProblemGenerationLogger;

    void display_message(const std::string& message) override
    {
    }

    void display_message(const std::string& message,
                         const LogUtils::LOGLEVEL log_level,
                         const std::string& context) override
    {
    }

    void PrintIterationSeparatorBegin() override
    {
    }

    void PrintIterationSeparatorEnd() override
    {
    }
};

// Fixture
class MasterGenerationTest: public ::testing::Test
{
protected:
    void SetUp() override
    {
        // Create a temporary directory
        temp_test_dir = std::filesystem::temp_directory_path() / std::tmpnam(nullptr);
        std::filesystem::create_directories(temp_test_dir / "lp");
    }

    void AddCandidate(std::string name)
    {
        CandidateData candidate_data;
        candidate_data.name = name;

        ActiveLinksBuilder active_link_builder({candidate_data},
                                               {},
                                               std::make_shared<NoopProblemGenerationLogger>(
                                                 LogUtils::LOGLEVEL::INFO));

        auto links = active_link_builder.getLinks();
        active_links_.insert(active_links_.end(), links.begin(), links.end());
    }

    std::filesystem::path temp_test_dir;
    AdditionalConstraints additionalConstraints_{nullptr};
    Couplings couplings_;
    SolverLogManager solver_log_manager_;
    std::vector<ActiveLink> active_links_;
    FileWriter writer{ProblemsFormat::OPTIMIZED};
};

using SolverName = std::string;
using SolverAndExpectation = std::pair<SolverName, std::string>;

class TestForSolverAndMode: public MasterGenerationTest,
                            public ::testing::WithParamInterface<SolverName>
{
};

class TestForSolverAndExpectation: public MasterGenerationTest,
                                   public ::testing::WithParamInterface<SolverAndExpectation>
{
};

auto solverNamesGenerator = ::testing::ValuesIn(SolverLoader::GetSupportedSolvers());

#define SKIP_UNAVAILABLE_SOLVER(solver_name)                                          \
    auto available_solvers = SolverLoader::GetAvailableSolvers(                       \
      std::make_shared<NoopProblemGenerationLogger>(LogUtils::LOGLEVEL::INFO));       \
    if (std::ranges::find(available_solvers, solver_name) == available_solvers.end()) \
    {                                                                                 \
        GTEST_SKIP() << "Solver " << (solver_name) << " is not available";            \
    }

TEST_P(TestForSolverAndExpectation, master_file_is_generated)
{
    auto&& [solver_name, expectation] = GetParam();
    SKIP_UNAVAILABLE_SOLVER(solver_name)
    AddCandidate("dummy_candidate");
    auto _ = MasterGeneration(temp_test_dir, solver_name, nullptr, solver_log_manager_, writer)
               .generate(active_links_, "master_formulation", additionalConstraints_);
    auto master_file = temp_test_dir / "lp" / "master";
    master_file.replace_extension(expectation);
    ASSERT_TRUE(std::filesystem::exists(master_file));
}

// Structure file is written
TEST_P(TestForSolverAndMode, structure_file_is_written)
{
    auto&& solver_name = GetParam();
    SKIP_UNAVAILABLE_SOLVER(solver_name)
    AddCandidate("dummy_candidate");
    auto&& candidates = MasterGeneration(temp_test_dir,
                                         solver_name,
                                         nullptr,
                                         solver_log_manager_,
                                         writer)
                          .generate(active_links_, "master_formulation", additionalConstraints_);
    StructureGeneration(temp_test_dir, solver_name, ProblemsFormat::OPTIMIZED)(candidates,
                                                                                couplings_);
    ASSERT_TRUE(std::filesystem::exists(temp_test_dir / "lp" / "structure.txt"));
}

// Structure file contains master name
TEST_P(TestForSolverAndMode, structure_file_contains_master_name)
{
    auto&& solver_name = GetParam();
    AddCandidate("dummy_candidate");
    SKIP_UNAVAILABLE_SOLVER(solver_name)
    auto&& candidates = MasterGeneration(temp_test_dir,
                                         solver_name,
                                         nullptr,
                                         solver_log_manager_,
                                         writer)
                          .generate(active_links_, "master_formulation", additionalConstraints_);
    StructureGeneration(temp_test_dir, solver_name, ProblemsFormat::OPTIMIZED)(candidates,
                                                                                couplings_);
    std::ifstream structure_file(temp_test_dir / "lp" / "structure.txt");
    std::string line;
    bool found = false;
    while (std::getline(structure_file, line))
    {
        if (line.find("master") != std::string::npos)
        {
            found = true;
            break;
        }
    }
    ASSERT_TRUE(found);
}

INSTANTIATE_TEST_SUITE_P(_, TestForSolverAndMode, solverNamesGenerator);

////Structure file contains master name without file extension
TEST_P(TestForSolverAndExpectation, structure_file_contains_master_name_without_extension)
{
    AddCandidate("dummy_candidate");
    auto&& [solver_name, expectation] = GetParam();
    SKIP_UNAVAILABLE_SOLVER(solver_name)
    auto&& candidates = MasterGeneration(temp_test_dir,
                                         solver_name,
                                         nullptr,
                                         solver_log_manager_,
                                         writer)
                          .generate(active_links_, "master_formulation", additionalConstraints_);
    StructureGeneration(temp_test_dir, solver_name, ProblemsFormat::OPTIMIZED)(candidates,
                                                                                couplings_);
    std::ifstream structure_file(temp_test_dir / "lp" / "structure.txt");
    std::string line;
    bool found = false;
    while (std::getline(structure_file, line))
    {
        auto pos = line.find("master");
        if (pos != std::string::npos)
        {
            auto with_ext_pos = line.find("master." + expectation);
            if (with_ext_pos == pos)
            {
                continue; // We want to find the name without the extension, so skip
            }
            found = true;
            break;
        }
    }
    ASSERT_TRUE(found);
}

INSTANTIATE_TEST_SUITE_P(_,
                         TestForSolverAndExpectation,
                         ::testing::Values(SolverAndExpectation{"XPRESS", "svf"},
                                           SolverAndExpectation{"CBC", "mps"},
                                           SolverAndExpectation{"CLP", "mps"}));

/**
 * Test that the structure file contains the problem file with the file extension
 */
TEST_P(TestForSolverAndExpectation, structure_file_contains_problem_name_with_extension)
{
    /**
     * This test isn't quiet good. It doesn't really test that a structures file
     * contains the problem name but it tests that the structure file contains
     * what is in the coupligs map.
     */
    auto&& [solver_name, expectation] = GetParam();
    SKIP_UNAVAILABLE_SOLVER(solver_name)

    AddCandidate("dummy_candidate");
    couplings_.insert({{"dummy_candidate", "dummy_problem." + expectation}, 0});
    auto&& candidates = MasterGeneration(temp_test_dir,
                                         solver_name,
                                         nullptr,
                                         solver_log_manager_,
                                         writer)
                          .generate(active_links_, "master_formulation", additionalConstraints_);
    StructureGeneration(temp_test_dir, solver_name, ProblemsFormat::OPTIMIZED)(candidates,
                                                                                couplings_);
    std::ifstream structure_file(temp_test_dir / "lp" / "structure.txt");
    std::string line;
    bool found = false;
    while (std::getline(structure_file, line))
    {
        auto pos = line.find("dummy_problem");
        if (pos != std::string::npos)
        {
            auto with_ext_pos = line.find("dummy_problem." + expectation);
            if (with_ext_pos == pos)
            {
                found = true;
                break;
            }
        }
    }
    ASSERT_TRUE(found);
}
